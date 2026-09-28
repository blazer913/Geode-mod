#include <Geode/Geode.hpp>
#include <Geode/modify/GJBaseGameLayer.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <fstream>
#include "Macro.hpp"

using namespace geode::prelude;

// ---- frame -> position table (index 0 = P1, 1 = P2), filled while playing ----
namespace {
    std::vector<CCPoint> g_pos[2];
    std::vector<uint8_t> g_has[2];
    bool g_record = true;

    void setPos(int pl, size_t f, CCPoint p) {
        if (f > 20'000'000) return;
        if (g_pos[pl].size() <= f) { g_pos[pl].resize(f + 1); g_has[pl].resize(f + 1, 0); }
        g_pos[pl][f] = p; g_has[pl][f] = 1;
    }
    bool getPos(int pl, size_t f, CCPoint& out) {
        if (f >= g_pos[pl].size() || !g_has[pl][f]) return false;
        out = g_pos[pl][f]; return true;
    }
    std::filesystem::path cachePath(int id) {
        return Mod::get()->getSaveDir() / fmt::format("pos_{}.bin", id);
    }
    void saveCache(int id) {
        std::ofstream o(cachePath(id), std::ios::binary);
        for (int pl = 0; pl < 2; pl++) {
            uint64_t n = g_pos[pl].size();
            o.write((char*)&n, 8);
            for (uint64_t i = 0; i < n; i++) {
                float xy[2] = {g_pos[pl][i].x, g_pos[pl][i].y};
                o.write((char*)&g_has[pl][i], 1); o.write((char*)xy, 8);
            }
        }
    }
    void loadCache(int id) {
        for (int pl = 0; pl < 2; pl++) { g_pos[pl].clear(); g_has[pl].clear(); }
        std::ifstream in(cachePath(id), std::ios::binary);
        for (int pl = 0; pl < 2 && in; pl++) {
            uint64_t n = 0; in.read((char*)&n, 8);
            if (!in || n > 20'000'000) return;
            g_pos[pl].resize(n); g_has[pl].resize(n);
            for (uint64_t i = 0; i < n && in; i++) {
                float xy[2]; in.read((char*)&g_has[pl][i], 1); in.read((char*)xy, 8);
                g_pos[pl][i] = ccp(xy[0], xy[1]);
            }
        }
    }
}

$on_mod(Loaded) {
    g_record = Mod::get()->getSettingValue<bool>("record");
    listenForSettingChanges<bool>("record", [](bool v) { g_record = v; });
}

// Records where the player is at the start of every physics step.
class $modify(CWRecorder, GJBaseGameLayer) {
    void processCommands(float dt) {
        if (g_record && static_cast<GJBaseGameLayer*>(PlayLayer::get()) == this && m_player1 && !m_player1->m_isDead) {
            size_t f = m_gameState.m_currentProgress;
            setPos(0, f, m_player1->getPosition());
            if (m_gameState.m_isDualMode && m_player2) setPos(1, f, m_player2->getPosition());
        }
        GJBaseGameLayer::processCommands(dt);
    }
};

class $modify(CWPlayLayer, PlayLayer) {
    struct Fields {
        std::vector<cw::Click> clicks;
        std::vector<uint8_t> built;
        CCNode* holder = nullptr;
        int tick = 0;
        int levelID = 0;
    };

    void setupHasCompleted() {
        PlayLayer::setupHasCompleted();
        auto& f = *m_fields;
        f.levelID = m_level->m_levelID.value();
        loadCache(f.levelID);

        auto path = Mod::get()->getSettingValue<std::filesystem::path>("macro-path");
        if (path.empty()) return;
        auto macro = cw::load(path);
        if (!macro.error.empty()) {
            Notification::create(macro.error, NotificationIcon::Error)->show();
            return;
        }
        f.clicks = cw::computeClicks(macro, (int)Mod::get()->getSettingValue<int64_t>("cap"));
        f.built.assign(f.clicks.size(), 0);
        f.holder = CCNode::create();
        m_objectLayer->addChild(f.holder, 1000);
        Notification::create(fmt::format("{} clicks analysed", f.clicks.size()), NotificationIcon::Success)->show();
        buildIndicators();
    }

    void postUpdate(float dt) {
        PlayLayer::postUpdate(dt);
        auto& f = *m_fields;
        if (f.holder && ++f.tick % 10 == 0) buildIndicators();
    }

    void onQuit() {
        if (g_record) saveCache(m_fields->levelID);
        PlayLayer::onQuit();
    }

    void buildIndicators() {
        auto& f = *m_fields;
        bool labels = Mod::get()->getSettingValue<bool>("show-labels");
        for (size_t i = 0; i < f.clicks.size(); i++) {
            if (f.built[i]) continue;
            auto& c = f.clicks[i];
            int pl = c.p2 ? 1 : 0;
            CCPoint at;
            if (!getPos(pl, c.frame, at)) continue;

            int width = c.early + c.late + 1;
            ccColor4F col = width <= 2 ? ccColor4F{1.f, .25f, .25f, .7f}
                          : width <= 6 ? ccColor4F{1.f, .85f, .2f, .7f}
                                       : ccColor4F{.3f, 1.f, .4f, .7f};
            // circle centred on where the player actually clicked
            auto draw = CCDrawNode::create();
            draw->drawDot(at, 9.f, ccColor4F{1, 1, 1, .9f});   // white rim
            draw->drawDot(at, 7.f, col);                       // window colour
            f.holder->addChild(draw);

            if (labels) {
                double ms = width * 1000.0 / 240.0;
                auto lbl = CCLabelBMFont::create(
                    fmt::format("{}  -{}/+{}  ({:.0f}ms)", c.frame, c.early, c.late, ms).c_str(), "chatFont.fnt");
                lbl->setScale(.4f);
                lbl->setOpacity(210);
                lbl->setPosition(at.x, at.y + 20.f);
                f.holder->addChild(lbl);
            }
            f.built[i] = 1;
        }
    }
};
