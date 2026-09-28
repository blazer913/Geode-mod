#pragma once
// Geode-free macro parsing + click window math (std only).
#include <algorithm>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <string>
#include <vector>

namespace cw {

struct Input { uint64_t frame = 0; bool down = false; bool p2 = false; };
struct Click { uint64_t frame = 0; bool p2 = false; int early = 0; int late = 0; };
struct Macro { double fps = 240.0; std::vector<Input> inputs; std::string error; };

struct Reader {
    const uint8_t* d; size_t n; size_t p = 0; bool bad = false;
    uint8_t u8() { if (p >= n) { bad = true; return 0; } return d[p++]; }
    void skip(size_t k) { if (k > n - p) { bad = true; p = n; } else p += k; }
    uint64_t varint() {
        uint64_t v = 0; int s = 0;
        for (;;) {
            uint8_t b = u8(); if (bad) return 0;
            if (s < 64) v |= uint64_t(b & 0x7F) << s;
            if (!(b & 0x80)) break;
            s += 7; if (s > 70) { bad = true; return 0; }
        }
        return v;
    }
    std::string cstr() {
        std::string s;
        for (;;) { uint8_t b = u8(); if (bad || b == 0) break; s.push_back(char(b)); }
        return s;
    }
};

// ---------- GDR2 (.gdr2): binary, LEB128 + packed deltas ----------
inline Macro parseGdr2(const std::vector<uint8_t>& bytes) {
    Macro m; Reader r{bytes.data(), bytes.size()};
    r.skip(3);                       // "GDR"
    r.varint();                      // format version
    std::string tag = r.cstr();      // input tag ("" = no per-input extension)
    r.cstr(); r.cstr();              // author, description
    r.varint(); r.varint();          // duration(frames), game version
    uint64_t fps = r.varint(); if (fps) m.fps = double(fps);
    r.varint(); r.varint();          // seed, coins
    r.u8();                          // ldm
    bool platformer = r.u8() != 0;
    r.cstr(); r.varint();            // bot name/version
    r.varint(); r.cstr();            // level id/name
    r.skip(r.varint());              // replay extension block
    uint64_t deaths = r.varint();
    for (uint64_t i = 0; i < deaths && !r.bad; i++) r.varint();
    uint64_t count = r.varint(), p1 = r.varint();
    if (r.bad || count > bytes.size()) { m.error = "Corrupt GDR2 header"; return m; }
    uint64_t frame = 0;
    for (uint64_t i = 0; i < count && !r.bad; i++) {
        if (i == p1) frame = 0;      // P2 block restarts delta chain
        uint8_t b = r.u8();
        bool down = b & 0x40; uint8_t btn = 1; uint64_t delta; int shift;
        if (platformer) { btn = (b >> 4) & 3; delta = b & 0x0F; shift = 4; }
        else            { delta = b & 0x3F; shift = 6; }
        while ((b & 0x80) && !r.bad) { b = r.u8(); if (shift < 64) delta |= uint64_t(b & 0x7F) << shift; shift += 7; }
        frame += delta;
        if (!tag.empty()) r.skip(r.varint());   // per-input extension
        if (btn == 1) m.inputs.push_back({frame, down, i >= p1});
    }
    if (r.bad) m.error = "Unexpected end of GDR2 data";
    return m;
}

// ---------- GDR1 (.gdr): MessagePack map ----------
struct Val { enum K { Nil, Bool, Int, Flt, Str, Arr, Map, Other, Bad } k = Nil; int64_t i = 0; double f = 0; std::string s; uint64_t n = 0;
    double num() const { return k == Flt ? f : double(i); } };

inline Val mpHead(Reader& r) {
    Val v; uint8_t b = r.u8();
    auto len = [&](int nb) { uint64_t x = 0; for (int i = 0; i < nb; i++) x = (x << 8) | r.u8(); return x; };
    auto str = [&](uint64_t l) { v.k = Val::Str; if (l > r.n - r.p) { r.bad = true; return; } for (uint64_t i = 0; i < l; i++) v.s.push_back(char(r.u8())); };
    if (b <= 0x7F) { v.k = Val::Int; v.i = b; }
    else if (b >= 0xE0) { v.k = Val::Int; v.i = int8_t(b); }
    else if ((b & 0xE0) == 0xA0) str(b & 0x1F);
    else if ((b & 0xF0) == 0x90) { v.k = Val::Arr; v.n = b & 0x0F; }
    else if ((b & 0xF0) == 0x80) { v.k = Val::Map; v.n = b & 0x0F; }
    else switch (b) {
        case 0xC0: v.k = Val::Nil; break;
        case 0xC2: v.k = Val::Bool; v.i = 0; break;
        case 0xC3: v.k = Val::Bool; v.i = 1; break;
        case 0xC4: r.skip(len(1)); v.k = Val::Other; break;
        case 0xC5: r.skip(len(2)); v.k = Val::Other; break;
        case 0xC6: r.skip(len(4)); v.k = Val::Other; break;
        case 0xCA: { uint32_t x = uint32_t(len(4)); float f; std::memcpy(&f, &x, 4); v.k = Val::Flt; v.f = f; break; }
        case 0xCB: { uint64_t x = len(8); double f; std::memcpy(&f, &x, 8); v.k = Val::Flt; v.f = f; break; }
        case 0xCC: v.k = Val::Int; v.i = int64_t(len(1)); break;
        case 0xCD: v.k = Val::Int; v.i = int64_t(len(2)); break;
        case 0xCE: v.k = Val::Int; v.i = int64_t(len(4)); break;
        case 0xCF: v.k = Val::Int; v.i = int64_t(len(8)); break;
        case 0xD0: v.k = Val::Int; v.i = int8_t(len(1)); break;
        case 0xD1: v.k = Val::Int; v.i = int16_t(len(2)); break;
        case 0xD2: v.k = Val::Int; v.i = int32_t(len(4)); break;
        case 0xD3: v.k = Val::Int; v.i = int64_t(len(8)); break;
        case 0xD9: str(len(1)); break;
        case 0xDA: str(len(2)); break;
        case 0xDB: str(len(4)); break;
        case 0xDC: v.k = Val::Arr; v.n = len(2); break;
        case 0xDD: v.k = Val::Arr; v.n = len(4); break;
        case 0xDE: v.k = Val::Map; v.n = len(2); break;
        case 0xDF: v.k = Val::Map; v.n = len(4); break;
        default: v.k = Val::Bad; r.bad = true;
    }
    return v;
}
inline void mpSkip(Reader& r, int depth = 0) {
    if (depth > 32) { r.bad = true; return; }
    Val v = mpHead(r);
    uint64_t k = v.k == Val::Arr ? v.n : v.k == Val::Map ? v.n * 2 : 0;
    for (uint64_t i = 0; i < k && !r.bad; i++) mpSkip(r, depth + 1);
}
inline Macro parseGdr1(const std::vector<uint8_t>& bytes) {
    Macro m; Reader r{bytes.data(), bytes.size()};
    Val top = mpHead(r);
    if (top.k != Val::Map) { m.error = "Not a GDR1 (MessagePack) file"; return m; }
    for (uint64_t i = 0; i < top.n && !r.bad; i++) {
        Val key = mpHead(r);
        if (key.s == "framerate") { Val v = mpHead(r); if (v.num() > 0) m.fps = v.num(); }
        else if (key.s == "inputs") {
            Val arr = mpHead(r);
            for (uint64_t j = 0; j < arr.n && !r.bad; j++) {
                Val mp = mpHead(r); Input in; bool plat = false; int btn = 1;
                for (uint64_t q = 0; q < mp.n && !r.bad; q++) {
                    Val k = mpHead(r);
                    if (k.s == "frame")     in.frame = uint64_t(mpHead(r).num());
                    else if (k.s == "btn")  { btn = int(mpHead(r).num()); plat = true; }
                    else if (k.s == "2p")   in.p2 = mpHead(r).i != 0;
                    else if (k.s == "down") in.down = mpHead(r).i != 0;
                    else mpSkip(r);
                }
                (void)plat;
                if (btn == 1) m.inputs.push_back(in);
            }
        } else mpSkip(r);
    }
    if (r.bad) m.error = "Could not read GDR1 data";
    return m;
}

inline Macro load(const std::filesystem::path& path) {
    Macro m;
    auto ext = path.extension().string();
    std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char c) { return char(std::tolower(c)); });
    std::ifstream f(path, std::ios::binary);
    if (!f) { m.error = "Can't open macro file"; return m; }
    std::vector<uint8_t> bytes((std::istreambuf_iterator<char>(f)), {});
    if (bytes.size() >= 3 && std::memcmp(bytes.data(), "GDR", 3) == 0) m = parseGdr2(bytes);
    else if (ext == ".ttr") m.error = "TTR isn't supported yet (format unknown)";
    else m = parseGdr1(bytes);
    return m;
}

// Window = free frames around the click before it would touch the neighbouring
// input of the same player, capped at `cap`.
inline std::vector<Click> computeClicks(const Macro& m, int cap) {
    std::vector<Click> out;
    for (int pl = 0; pl < 2; pl++) {
        std::vector<Input> v;
        for (auto& i : m.inputs) if (i.p2 == bool(pl)) v.push_back(i);
        std::stable_sort(v.begin(), v.end(), [](auto& a, auto& b) { return a.frame < b.frame; });
        for (size_t i = 0; i < v.size(); i++) {
            if (!v[i].down) continue;
            int64_t f = int64_t(v[i].frame);
            int64_t before = i ? f - int64_t(v[i - 1].frame) - 1 : f;
            int64_t after = i + 1 < v.size() ? int64_t(v[i + 1].frame) - f - 1 : cap;
            out.push_back({v[i].frame, bool(pl),
                int(std::clamp<int64_t>(before, 0, cap)), int(std::clamp<int64_t>(after, 0, cap))});
        }
    }
    std::sort(out.begin(), out.end(), [](auto& a, auto& b) { return a.frame < b.frame; });
    return out;
}

} // namespace cw
