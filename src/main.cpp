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

        if (engine.m_state == MacroEngine::State::Playing) {
            while (engine.m_playbackIndex < engine.m_inputs.size() && 
                   engine.m_inputs[engine.m_playbackIndex].frame <= engine.m_currentFrame) 
            {
                auto& input = engine.m_inputs[engine.m_playbackIndex];
                
                // Set flag to true so our mod knows this is a bot input, not a player input
                engine.m_isPlaybackInput = true;
                
                // Inject the button press/release into the game
                this->handleButton(input.isDown, static_cast<int>(input.button), input.isPlayer1);
                
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
                 engine.m_inputs.clear(); // Clear bad inputs on death
             }
        }
    }
};

class $modify(MyGameLayer, GJBaseGameLayer) {
    void handleButton(bool isDown, int button, bool isPlayer1) {
        auto& engine = MacroEngine::get();
        
        // Only record the click if the bot isn't currently injecting playback inputs
        if (engine.m_state == MacroEngine::State::Recording && !engine.m_isPlaybackInput) {
            engine.recordInput(static_cast<PlayerButton>(button), isDown, isPlayer1);
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
        // Pass 'this' so the menu knows which PauseLayer to trigger the restart on
        MacroModMenu::create(this)->show(); 
    }
};
