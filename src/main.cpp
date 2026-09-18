#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/GJBaseGameLayer.hpp>
#include <Geode/modify/UILayer.hpp>
#include "MacroModMenu.hpp"
#include "macro.hpp"

using namespace geode::prelude;

// Hook 1: Frame Tracking and Practice Mode Resets
class $modify(MyPlayLayer, PlayLayer) {
    void update(float dt) {
        PlayLayer::update(dt);
        
        auto& engine = MacroEngine::get();
        if (engine.m_state == MacroEngine::State::Idle) return;

        engine.m_currentFrame++;

        // Process playback if playing
        if (engine.m_state == MacroEngine::State::Playing) {
            while (engine.m_playbackIndex < engine.m_inputs.size() && 
                   engine.m_inputs[engine.m_playbackIndex].frame <= engine.m_currentFrame) 
            {
                auto& input = engine.m_inputs[engine.m_playbackIndex];
                
                // Force the input into the game engine
                this->handleButton(input.isDown, static_cast<int>(input.button), input.isPlayer1);
                
                engine.m_playbackIndex++;
            }
            
            if (engine.m_playbackIndex >= engine.m_inputs.size()) {
                engine.m_state = MacroEngine::State::Idle;
                log::info("Playback finished.");
                FLAlertLayer::create("Macro", "Playback Finished", "OK")->show();
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
                 engine.m_inputs.clear(); // Wipe on normal mode death
             }
        }
    }
};

// Hook 2: The Core Input Interceptor
class $modify(MyGameLayer, GJBaseGameLayer) {
    void handleButton(bool isDown, int button, bool isPlayer1) {
        auto& engine = MacroEngine::get();
        
        if (engine.m_state == MacroEngine::State::Recording) {
            engine.recordInput(static_cast<PlayerButton>(button), isDown, isPlayer1);
        }

        GJBaseGameLayer::handleButton(isDown, button, isPlayer1);
    }
};

// Hook 3: On-Screen Mobile Buttons & PC Keyboard Shortcuts
class $modify(MyUILayer, UILayer) {
    bool init(GJBaseGameLayer* gameLayer) {
        if (!UILayer::init(gameLayer)) return false;

        auto winSize = CCDirector::sharedDirector()->getWinSize();

        // 1. Create a menu for our on-screen buttons
        auto macroMenu = CCMenu::create();
        macroMenu->setID("macro-mobile-menu"_spr);
        
        // Position it at the top-left, slightly to the right of the pause button
        macroMenu->setPosition({110.f, winSize.height - 20.f});
        macroMenu->setLayout(RowLayout::create()->setGap(10.f));

        // 2. Create the Record Button
        auto recSpr = ButtonSprite::create("REC");
        recSpr->setScale(0.55f);
        auto recBtn = CCMenuItemSpriteExtra::create(
            recSpr, this, menu_selector(MyUILayer::onRecordBtn)
        );

        // 3. Create the Play Button
        auto playSpr = ButtonSprite::create("PLAY");
        playSpr->setScale(0.55f);
        auto playBtn = CCMenuItemSpriteExtra::create(
            playSpr, this, menu_selector(MyUILayer::onPlayBtn)
        );

        // Add them to the menu and update layout
        macroMenu->addChild(recBtn);
        macroMenu->addChild(playBtn);
        macroMenu->updateLayout();

        this->addChild(macroMenu);

        return true;
    }

    // Mobile touch callbacks
    void onRecordBtn(CCObject* sender) {
        MacroEngine::get().toggleRecording();
        
        // Optional visual feedback on mobile
        auto alertStr = MacroEngine::get().m_state == MacroEngine::State::Recording ? "Recording Started" : "Recording Stopped";
        auto notif = NotificationCenter::sharedNotificationCenter();
        log::info("{}", alertStr);
    }

    void onPlayBtn(CCObject* sender) {
        MacroEngine::get().togglePlayback();
        log::info("Playback toggled");
    }

    // Keep the PC keybinds working alongside the mobile buttons
    void keyDown(cocos2d::enumKeyCodes key, double timestamp) {
        if (key == cocos2d::enumKeyCodes::KEY_R) {
            this->onRecordBtn(nullptr);
            return; 
        } 
        else if (key == cocos2d::enumKeyCodes::KEY_P) {
            this->onPlayBtn(nullptr);
            return;
        }
        
        UILayer::keyDown(key, timestamp);
    }
};
