#include <Geode/Geode.hpp>
#include <Geode/modify/PauseLayer.hpp>
#include <Geode/modify/UILayer.hpp>
#include "MacroModMenu.hpp"

using namespace geode::prelude;

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
        MacroModMenu::create()->show();
    }

    // Updated signature with the two missing boolean arguments
    void keyDown(cocos2d::enumKeyCodes key, bool isDown, bool isRepeat) {
        if (key == cocos2d::enumKeyCodes::KEY_M) {
            this->onOpenMacroMenu(nullptr);
            return;
        }
        // Pass all arguments back to the original function
        PauseLayer::keyDown(key, isDown, isRepeat);
    }
};

class $modify(MyMacroInputs, UILayer) {
    // Updated signature here as well
    void keyDown(cocos2d::enumKeyCodes key, bool isDown, bool isRepeat) {
        if (key == cocos2d::enumKeyCodes::KEY_R) {
            log::info("Recording triggered via Keybind");
            // MacroEngine::get()->startRecording();
            return; 
        } 
        else if (key == cocos2d::enumKeyCodes::KEY_P) {
            log::info("Playback triggered via Keybind");
            // MacroEngine::get()->startPlayback();
            return;
        }
        
        UILayer::keyDown(key, isDown, isRepeat);
    }
};
