#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/PlayerObject.hpp>
#include "Controller.hpp"
#include "Macro.hpp"
#include "CbotOverlay.hpp"

using namespace geode::prelude;
using namespace cocos2d;

namespace cbot {

static PlayerButton toPlayerButton(Button b) { return static_cast<PlayerButton>(static_cast<int>(b)); }
static Button toCbotButton(PlayerButton b) { return static_cast<Button>(static_cast<int>(b)); }

class State {
public:
    Macro macro;
    Controller controller;
    bool recording = false;
    bool playing = false;
    bool overlayOn = true;
    int frame = 0;
    size_t playIndex = 0;
    CbotOverlay* hud = nullptr;

    State() {
        controller.setCallback([this](Command c) {
            auto pl = PlayLayer::get();
            switch (c) {
                case Command::RecordToggle:
                    if (playing) break;
                    recording = !recording;
                    if (recording) {
                        macro.clear();
                        frame = 0;
                        if (pl) pl->resetLevel();
                    }
                    break;
                case Command::PlayPause:
                    if (recording || macro.events.empty()) break;
                    playing = !playing;
                    if (playing) {
                        frame = 0;
                        playIndex = 0;
                        if (pl) pl->resetLevel();
                    }
                    break;
                case Command::FrameBack:
                case Command::FrameForward:
                    break;
                case Command::OverlayToggle:
                    overlayOn = !overlayOn;
                    if (hud) hud->setCounterVisible(overlayOn);
                    break;
            }
            refreshHud();
        });
    }

    void refreshHud() {
        if (!hud) return;
        hud->setFrame(frame);
        if (recording) hud->setStatus("RECORDING");
        else if (playing) hud->setStatus("PLAYING " + std::to_string(playIndex) + "/" + std::to_string(macro.events.size()));
        else hud->setStatus("IDLE");
    }
};

static State g_state;

class $modify(CBotPlayerObject, PlayerObject) {
    void pushButton(PlayerButton button) {
        PlayerObject::pushButton(button);
        auto pl = PlayLayer::get();
        if (!pl || !g_state.recording) return;
        int playerNum = (pl->m_player1 == this) ? 1 : 2;
        g_state.macro.add({ g_state.frame, toCbotButton(button), true, playerNum });
    }

    void releaseButton(PlayerButton button) {
        PlayerObject::releaseButton(button);
        auto pl = PlayLayer::get();
        if (!pl || !g_state.recording) return;
        int playerNum = (pl->m_player1 == this) ? 1 : 2;
        g_state.macro.add({ g_state.frame, toCbotButton(button), false, playerNum });
    }
};

class $modify(CBotPlayLayer, PlayLayer) {
    bool init(GJGameLevel* level, bool useReplay, bool dontCreateObjects) {
        if (!PlayLayer::init(level, useReplay, dontCreateObjects))
            return false;

        g_state.frame = 0;
        g_state.playIndex = 0;
        g_state.recording = false;
        g_state.playing = false;

        auto hud = CbotOverlay::create();
        if (hud) {
            this->addChild(hud, 9999);
            hud->setOnCommand([](Command c) { g_state.controller.command(c); });
            hud->setCounterVisible(g_state.overlayOn);
            g_state.hud = hud;
        }

        g_state.refreshHud();
        return true;
    }

    void update(float dt) {
        PlayLayer::update(dt);
        ++g_state.frame;

        if (g_state.playing) {
            auto& events = g_state.macro.events;
            while (g_state.playIndex < events.size() &&
                   events[g_state.playIndex].frame <= g_state.frame) {
                auto const& e = events[g_state.playIndex];
                auto player = (e.player == 2) ? this->m_player2 : this->m_player1;
                if (player) {
                    auto btn = toPlayerButton(e.button);
                    if (e.pressed) player->pushButton(btn);
                    else player->releaseButton(btn);
                }
                ++g_state.playIndex;
            }
            if (g_state.playIndex >= events.size())
                g_state.playing = false;
        }

        g_state.refreshHud();
    }

    void onQuit() {
        if (g_state.hud) {
            g_state.hud->removeFromParentAndCleanup(true);
            g_state.hud = nullptr;
        }
        PlayLayer::onQuit();
    }
};

}

$execute {
    log::info("CBot v0.2.0 loaded.");
}
