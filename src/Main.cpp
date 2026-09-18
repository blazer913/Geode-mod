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

static PlayerButton toPlayerButton(Button b) {
    return static_cast<PlayerButton>(static_cast<int>(b));
}

static Button toCbotButton(PlayerButton b) {
    return static_cast<Button>(static_cast<int>(b));
}

static State g_state;

State& getState() {
    return g_state;
}

void State::refreshHud() {
    if (!hud) return;
    hud->setFrame(static_cast<int>(time * 1000)); 
    if (recording)
        hud->setStatus("RECORDING");
    else if (playing)
        hud->setStatus("PLAYING " + std::to_string(playIndex) + "/" + std::to_string(macro.events.size()));
    else
        hud->setStatus("IDLE");
}

static void installController() {
    g_state.controller.setCallback([](Command c) {
        switch (c) {
            case Command::RecordToggle:
                g_state.recording = !g_state.recording;
                g_state.playing = false;
                g_state.needsReset = true; 
                break;

            case Command::PlayPause:
                if (g_state.macro.events.empty()) break;
                g_state.playing = !g_state.playing;
                g_state.recording = false;
                g_state.needsReset = true;
                break;

            case Command::OverlayToggle:
                g_state.overlayOn = !g_state.overlayOn;
                if (g_state.hud)
                    g_state.hud->setVisible(g_state.overlayOn);
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
        g_state.macro.add({
            g_state.time,
            toCbotButton(button),
            true,
            playerNum
        });
    }

    void releaseButton(PlayerButton button) {
        PlayerObject::releaseButton(button);

        auto pl = PlayLayer::get();
        if (!pl || !g_state.recording) return;

        int playerNum = (pl->m_player1 == this) ? 1 : 2;
        g_state.macro.add({
            g_state.time,
            toCbotButton(button),
            false,
            playerNum
        });
    }
};

class $modify(CBotPlayLayer, PlayLayer) {
    bool init(GJGameLevel* level, bool useReplay, bool dontCreateObjects) {
        if (!PlayLayer::init(level, useReplay, dontCreateObjects))
            return false;

        g_state.time = 0.0;
        g_state.playIndex = 0;
        g_state.recording = false;
        g_state.playing = false;
        g_state.checkpoints.clear();

        auto hud = CbotOverlay::create();
        if (hud) {
            this->addChild(hud, 9999);
            hud->setVisible(g_state.overlayOn);
            g_state.hud = hud;
        }

        g_state.refreshHud();
        return true;
    }

    void resetLevel() {
        PlayLayer::resetLevel();

        if (this->m_isPracticeMode && !g_state.checkpoints.empty()) {
            auto& cp = g_state.checkpoints.back();
            g_state.time = cp.time;
            g_state.playIndex = cp.playIndex;
            if (g_state.recording) {
                g_state.macro.events.resize(cp.macroSize); 
            }
        } else {
            g_state.time = 0.0;
            g_state.playIndex = 0;
            g_state.checkpoints.clear();
            if (g_state.recording) g_state.macro.clear();
        }
        g_state.refreshHud();
    }

    CheckpointObject* markCheckpoint() {
        auto cp = PlayLayer::markCheckpoint();
        g_state.checkpoints.push_back({ g_state.time, g_state.macro.events.size(), g_state.playIndex });
        return cp;
    }

    void removeCheckpoint(bool first) {
        PlayLayer::removeCheckpoint(first);
        if (!g_state.checkpoints.empty()) g_state.checkpoints.pop_back();
    }

    void update(float dt) {
        if (g_state.needsReset) {
            g_state.needsReset = false;
            this->resetLevel();
            return;
        }

        g_state.time += dt; 

        if (g_state.playing) {
            auto& events = g_state.macro.events;

            while (
                g_state.playIndex < events.size() &&
                events[g_state.playIndex].time <= g_state.time
            ) {
                auto const& e = events[g_state.playIndex];

                auto player = (e.player == 2)
                    ? this->m_player2
                    : this->m_player1;

                if (player) {
                    auto btn = toPlayerButton(e.button);

                    if (e.pressed)
                        player->pushButton(btn);
                    else
                        player->releaseButton(btn);
                }

                ++g_state.playIndex;
            }

            if (g_state.playIndex >= events.size())
                g_state.playing = false;
        }

        PlayLayer::update(dt);
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

class $modify(CBotPauseLayer, PauseLayer) {
    void customSetup() {
        PauseLayer::customSetup();

        auto spr = ButtonSprite::create("CBot");
        spr->setScale(.7f);

        auto btn = CCMenuItemSpriteExtra::create(
            spr,
            this,
            menu_selector(CBotPauseLayer::onCbot)
        );

        CCMenu* targetMenu = typeinfo_cast<CCMenu*>(
            this->getChildByID("pause-menu")
        );

        if (!targetMenu) {
            for (auto child : CCArrayExt<CCNode*>(this->getChildren())) {
                if (auto menu = typeinfo_cast<CCMenu*>(child)) {
                    targetMenu = menu;
                    break;
                }
            }
        }

        if (targetMenu) {
            targetMenu->addChild(btn);
            targetMenu->updateLayout();
        }
    }

    void onCbot(CCObject*) {
        auto popup = CbotSettingsPopup::create();

        if (popup) {
            popup->setOnCommand([](Command c) {
                g_state.controller.command(c);
            });
            popup->show();
        }
    }
};

$execute {
    installController();
    log::info("CBot v0.4 loaded.");
}

}
