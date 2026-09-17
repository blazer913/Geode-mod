#include "TTRL.hpp"
#include <fstream>
#include <cstring>
#include <sstream>

namespace twa {

static uint16_t u16le(std::vector<uint8_t> const& b, size_t p) {
    return uint16_t(b[p]) | (uint16_t(b[p + 1]) << 8);
}

static uint32_t u32le(std::vector<uint8_t> const& b, size_t p) {
    return uint32_t(b[p]) |
           (uint32_t(b[p + 1]) << 8) |
           (uint32_t(b[p + 2]) << 16) |
           (uint32_t(b[p + 3]) << 24);
}

TTRLParseResult TTRLReader::read(std::filesystem::path const& path) {
    std::ifstream f(path, std::ios::binary);
    if (!f) {
        TTRLParseResult r;
        r.error = "Could not open file.";
        return r;
    }

    f.seekg(0, std::ios::end);
    auto n = f.tellg();
    f.seekg(0, std::ios::beg);

    if (n <= 0 || n > 128 * 1024 * 1024) {
        TTRLParseResult r;
        r.error = "File size is invalid.";
        return r;
    }

    std::vector<uint8_t> bytes(static_cast<size_t>(n));
    f.read(reinterpret_cast<char*>(bytes.data()), n);

    return readBytes(bytes);
}

TTRLParseResult TTRLReader::readBytes(std::vector<uint8_t> const& b) {
    TTRLParseResult r;

    if (b.size() < 8) {
        r.error = "File is too small for a TTRL header.";
        return r;
    }

    r.header.magic = u32le(b, 0);
    if (r.header.magic != 0x4C525454) { // "TTRL" little-endian
        r.error = "Not a TTRL file.";
        return r;
    }

    r.header.version = b[4];
    r.header.flags = b[5];
    r.header.headerSize = u16le(b, 6);

    if (r.header.headerSize > b.size()) {
        r.error = "Header size exceeds file size.";
        return r;
    }

    if (b.size() >= 12)
        r.header.declaredSize = u32le(b, 8);

    // Do not guess the event layout. Older public converters describe
    // TTRL events conceptually as frame/button/down/player2, but the binary
    // representation differs by TTRL revision. Unknown records are therefore
    // left untouched instead of producing false input windows.
    r.ok = true;
    r.bytesConsumed = r.header.headerSize;
    r.error = "Header validated; event decoder not selected for this TTRL revision.";
    return r;
}

}
