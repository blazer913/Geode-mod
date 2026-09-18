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
    hud->setFrame(frame);
    if (recording)
        hud->setStatus("RECORDING");
    else if (playing)
        hud->setStatus("PLAYING " + std::to_string(playIndex) + "/" + std::to_string(macro.events.size()));
    else
        hud->setStatus("IDLE");
}

static void installController() {
    g_state.controller.setCallback([](Command c) {
        auto pl = PlayLayer::get();

        switch (c) {
            case Command::RecordToggle:
                if (g_state.playing) break;
                g_state.recording = !g_state.recording;
                if (g_state.recording) {
                    g_state.macro.clear();
                    g_state.frame = 0;
                    if (pl) pl->resetLevel();
                }
                break;

            case Command::PlayPause:
                if (g_state.recording || g_state.macro.events.empty()) break;
                g_state.playing = !g_state.playing;
                if (g_state.playing) {
                    g_state.frame = 0;
                    g_state.playIndex = 0;
                    if (pl) pl->resetLevel();
                }
                break;

            case Command::FrameBack:
                if (g_state.frame > 0) --g_state.frame;
                break;

            case Command::FrameForward:
                ++g_state.frame;
                break;

            case Command::OverlayToggle:
                g_state.overlayOn = !g_state.overlayOn;
                if (g_state.hud)
                    g_state.hud->setVisible(g_state.overlayOn);
                break;
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
            g_state.frame,
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
            g_state.frame,
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

        g_state.frame = 0;
        g_state.playIndex = 0;
        g_state.recording = false;
        g_state.playing = false;

        auto hud = CbotOverlay::create();
        if (hud) {
            this->addChild(hud, 9999);
            hud->setVisible(g_state.overlayOn);
            g_state.hud = hud;
        }

        g_state.refreshHud();
        return true;
    }

    void update(float dt) {
        if (g_state.playing) {
            auto& events = g_state.macro.events;

            while (
                g_state.playIndex < events.size() &&
                events[g_state.playIndex].frame <= g_state.frame
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
        ++g_state.frame;
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
    log::info("CBot v0.3.1 loaded.");
}

}
