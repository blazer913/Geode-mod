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

    void keyDown(enumKeyCodes key) {
        if (key == enumKeyCodes::KEY_M) {
            this->onOpenMacroMenu(nullptr);
            return;
        }
        PauseLayer::keyDown(key);
    }
};

class $modify(MyMacroInputs, UILayer) {
    void keyDown(enumKeyCodes key) {
        if (key == enumKeyCodes::KEY_R) {
            log::info("Recording triggered via Keybind");
            // MacroEngine::get()->startRecording();
            return; 
        } 
        else if (key == enumKeyCodes::KEY_P) {
            log::info("Playback triggered via Keybind");
            // MacroEngine::get()->startPlayback();
            return;
        }
        
        UILayer::keyDown(key);
    }
};
