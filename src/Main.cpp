#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/PlayerObject.hpp>
#include <Geode/modify/PauseLayer.hpp>
#include "Controller.hpp"
#include "Macro.hpp"
#include "State.hpp"
#include "CbotOverlay.hpp"
#include "CbotSettingsPopup.hpp"

using namespace geode::prelude;
using namespace cocos2d;

namespace cbot {

static PlayerButton toPlayerButton(Button b) { return static_cast<PlayerButton>(static_cast<int>(b)); }
static Button toCbotButton(PlayerButton b) { return static_cast<Button>(static_cast<int>(b)); }

static State g_state;
State& getState() { return g_state; }

void State::refreshHud() {
    if (!hud) return;
    hud->setFrame(static_cast<int>(time * 1000)); // Display ms instead of frames
    if (recording) hud->setStatus("RECORDING");
    else if (playing) hud->setStatus("PLAYING " + std::to_string(playIndex) + "/" + std::to_string(macro.events.size()));
    else hud->setStatus("IDLE");
}

static void installController() {
    g_state.controller.setCallback([](Command c) {
        switch (c) {
            case Command::RecordToggle:
                g_state.recording = !g_state.recording;
                g_state.playing = false;
                g_state.needsReset = true; // Safely wait for unpause
                break;

            case Command::PlayPause:
                if (g_state.macro.events.empty()) break;
                g_state.playing = !g_state.playing;
                g_state.recording = false;
                g_state.needsReset = true;
                break;

            case Command::OverlayToggle:
                g_state.overlayOn = !g_state.overlayOn;
                if (g_state.hud) g_state.hud->setVisible(g_state.overlayOn);
                break;
            default: break;
        }
        g_state.refreshHud();
    });
}

class $modify(CBotPlayerObject, PlayerObject) {
    void pushButton(PlayerButton button) {
        PlayerObject::pushButton(button);
        auto pl = PlayLayer::get();
        if (!pl || !g_state.recording) return;
        int playerNum = (pl->m_player1 == this) ? 1 : 2;
        g_state.macro.add({ g_state.time, toCbotButton(button), true, playerNum });
    }

    void releaseButton(PlayerButton button) {
        PlayerObject::releaseButton(button);
        auto pl = PlayLayer::get();
        if (!pl || !g_state.recording) return;
        int playerNum = (pl->m_player1 == this) ? 1 : 2;
        g_state.macro.add({ g_state.time, toCbotButton(button), false, playerNum });
    }
};

class $modify(CBotPlayLayer, PlayLayer) {
    void resetLevel() {
        PlayLayer::resetLevel();

        // Practice Mode Rollback Logic
        if (this->m_isPracticeMode && !g_state.checkpoints.empty()) {
            auto& cp = g_state.checkpoints.back();
            g_state.time = cp.time;
            g_state.playIndex = cp.playIndex;
            if (g_state.recording) {
                g_state.macro.events.resize(cp.macroSize); // Truncate dead inputs
            }
        } else {
            g_state.time = 0.0;
            g_state.playIndex = 0;
            g_state.checkpoints.clear();
            if (g_state.recording) g_state.macro.clear();
        }
        g_state.refreshHud();
    }

    void markCheckpoint() {
        PlayLayer::markCheckpoint();
        g_state.checkpoints.push_back({ g_state.time, g_state.macro.events.size(), g_state.playIndex });
    }

    void removeLastCheckpoint() {
        PlayLayer::removeLastCheckpoint();
        if (!g_state.checkpoints.empty()) g_state.checkpoints.pop_back();
    }

    void update(float dt) {
        if (g_state.needsReset) {
            g_state.needsReset = false;
            this->resetLevel();
            return;
        }

        g_state.time += dt; // Accumulate dt (automatically handles speedhack)

        if (g_state.playing) {
            auto& events = g_state.macro.events;
            while (g_state.playIndex < events.size() && events[g_state.playIndex].time <= g_state.time) {
                auto const& e = events[g_state.playIndex];
                auto player = (e.player == 2) ? this->m_player2 : this->m_player1;
                if (player) {
                    auto btn = toPlayerButton(e.button);
                    if (e.pressed) player->pushButton(btn);
                    else player->releaseButton(btn);
                }
                ++g_state.playIndex;
            }
            if (g_state.playIndex >= events.size()) g_state.playing = false;
        }

        PlayLayer::update(dt);
        g_state.refreshHud();
    }
    
    // Include your original init() and onQuit() methods below here...
};

// Include your original PauseLayer hook and $execute block below here...
