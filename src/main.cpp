#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/PlayerObject.hpp>
#include <Geode/modify/GJBaseGameLayer.hpp>
#include <Geode/modify/PauseLayer.hpp>
#include <Geode/ui/Notification.hpp>
#include "MacroModMenu.hpp"
#include "macro.hpp"

using namespace geode::prelude;

class $modify(MyPlayerObject, PlayerObject) {
    void pushButton(PlayerButton button) {
        PlayerObject::pushButton(button);
        this->recordFromPlayerObject(button, true);
    }

    void releaseButton(PlayerButton button) {
        PlayerObject::releaseButton(button);
        this->recordFromPlayerObject(button, false);
    }

    void recordFromPlayerObject(PlayerButton button, bool isDown) {
        auto& engine = MacroEngine::get();
        auto pl = PlayLayer::get();
        bool isPlayer1 = !pl || pl->m_player1 == this;
        engine.recordInput(static_cast<int>(button), isDown, isPlayer1);
    }
};

class $modify(MyGameLayer, GJBaseGameLayer) {
    void handleButton(bool isDown, int button, bool isPlayer1) {
        auto& engine = MacroEngine::get();
        engine.recordInput(button, isDown, isPlayer1);

        GJBaseGameLayer::handleButton(isDown, button, isPlayer1);
    }
};

class $modify(MyPlayLayer, PlayLayer) {
    void update(float dt) {
        PlayLayer::update(dt);

        auto& engine = MacroEngine::get();
        if (engine.m_state == MacroEngine::State::Idle) return;

        engine.m_currentFrame++;

        if (engine.m_state == MacroEngine::State::Playing) {
            while (engine.m_playbackIndex < engine.m_inputs.size() &&
                   engine.m_inputs[engine.m_playbackIndex].frame <= engine.m_currentFrame)
            {
                auto& input = engine.m_inputs[engine.m_playbackIndex];

                engine.m_isPlaybackInput = true;
                this->handleButton(input.isDown, input.button, input.isPlayer1);
                engine.m_isPlaybackInput = false;

                engine.m_playbackIndex++;
            }

            if (engine.m_playbackIndex >= engine.m_inputs.size()) {
                engine.m_state = MacroEngine::State::Idle;
                geode::Notification::create("Playback Finished", NotificationIcon::Success)->show();
            }
        }
    }

    void resetLevel() {
        PlayLayer::resetLevel();

        auto& engine = MacroEngine::get();

        if (this->m_isPracticeMode) {
             engine.syncOnReset();
        } else {
             engine.m_currentFrame = 0;
             engine.m_playbackIndex = 0;

             if (engine.m_state == MacroEngine::State::Recording) {
                 engine.m_inputs.clear();
             }
        }
    }
};

class $modify(MyPauseLayer, PauseLayer) {
    void customSetup() {
        PauseLayer::customSetup();

        auto btnSprite = CCSprite::createWithSpriteFrameName("GJ_optionsBtn_001.png");
        btnSprite->setColor({50, 255, 50});

        auto modBtn = CCMenuItemSpriteExtra::create(
            btnSprite,
            this,
            menu_selector(MyPauseLayer::onOpenMacroMenu)
        );
        modBtn->setID("macro-menu-button"_spr);

        if (auto rightMenu = this->getChildByID("right-button-menu")) {
            rightMenu->addChild(modBtn);
            rightMenu->updateLayout();
        }
    }

    void onOpenMacroMenu(CCObject* sender) {
        MacroModMenu::create(this)->show();
    }
};
