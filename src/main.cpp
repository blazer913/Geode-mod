#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/GJBaseGameLayer.hpp>
#include <Geode/modify/PauseLayer.hpp>
#include <Geode/ui/Notification.hpp>
#include "MacroModMenu.hpp"
#include "macro.hpp"

using namespace geode::prelude;

class $modify(MyPlayLayer, PlayLayer) {
    void update(float dt) {
        PlayLayer::update(dt);
        
        auto& engine = MacroEngine::get();
        if (engine.m_state == MacroEngine::State::Idle) return;

        engine.m_currentFrame++;

        // Inject inputs directly into the PlayerObject for 100% reliability
        if (engine.m_state == MacroEngine::State::Playing) {
            while (engine.m_playbackIndex < engine.m_inputs.size() && 
                   engine.m_inputs[engine.m_playbackIndex].frame <= engine.m_currentFrame) 
            {
                auto& input = engine.m_inputs[engine.m_playbackIndex];
                
                PlayerObject* targetPlayer = input.isPlayer1 ? this->m_player1 : this->m_player2;
                
                if (targetPlayer) {
                    if (input.isDown) {
                        targetPlayer->pushButton(static_cast<PlayerButton>(input.button));
                    } else {
                        targetPlayer->releaseButton(static_cast<PlayerButton>(input.button));
                    }
                }
                
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
             // Normal mode death: keep state, reset frame to 0
             engine.m_currentFrame = 0;
             engine.m_playbackIndex = 0;
             
             // If recording, clear inputs so the new attempt starts clean
             if (engine.m_state == MacroEngine::State::Recording) {
                 engine.m_inputs.clear(); 
             }
        }
    }
};

class $modify(MyGameLayer, GJBaseGameLayer) {
    void handleButton(bool isDown, int button, bool isPlayer1) {
        auto& engine = MacroEngine::get();
        
        // Record real player inputs
        if (engine.m_state == MacroEngine::State::Recording) {
            engine.recordInput(button, isDown, isPlayer1);
        }

        GJBaseGameLayer::handleButton(isDown, button, isPlayer1);
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
