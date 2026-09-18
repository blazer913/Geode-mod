#pragma once
#include <Geode/Geode.hpp>
#include <Geode/ui/Notification.hpp>
#include "macro.hpp"

using namespace geode::prelude;

// ==========================================
// MENU 2: THE MACRO SELECTION SCREEN
// ==========================================
class MacroSelectMenu : public FLAlertLayer {
protected:
    PauseLayer* m_pauseLayer;

    bool init(PauseLayer* pauseLayer) {
        if (!FLAlertLayer::init(75)) return false;
        m_pauseLayer = pauseLayer;

        auto winSize = CCDirector::sharedDirector()->getWinSize();
        
        auto bg = CCScale9Sprite::create("GJ_square01.png", {0, 0, 80, 80});
        bg->setContentSize({320.f, 240.f});
        bg->setPosition(winSize.width / 2, winSize.height / 2);
        this->m_mainLayer->addChild(bg);
        
        auto title = CCLabelBMFont::create("Select a Macro", "goldFont.fnt");
        title->setPosition(winSize.width / 2, winSize.height / 2 + 95.f);
        title->setScale(0.8f);
        this->m_mainLayer->addChild(title);
        
        auto menu = CCMenu::create();
        menu->setPosition(winSize.width / 2, winSize.height / 2);
        
        // Loop through saved macros and create a button for each
        auto& saved = MacroEngine::get().m_savedMacros;
        for (auto const& [name, inputs] : saved) {
            auto btnSpr = ButtonSprite::create(name.c_str());
            auto btn = CCMenuItemSpriteExtra::create(
                btnSpr, this, menu_selector(MacroSelectMenu::onSelectMacro)
            );
            // Store the name in the button's ID so we know which one was clicked
            btn->setID(name);
            menu->addChild(btn);
        }
        
        menu->setLayout(ColumnLayout::create()->setGap(5.f));
        this->m_mainLayer->addChild(menu);

        auto closeBtnSpr = CCSprite::createWithSpriteFrameName("GJ_closeBtn_001.png");
        auto closeBtn = CCMenuItemSpriteExtra::create(
            closeBtnSpr, this, menu_selector(MacroSelectMenu::onClose)
        );
        
        auto closeMenu = CCMenu::create();
        closeMenu->setPosition(winSize.width / 2 - 145.f, winSize.height / 2 + 105.f);
        closeMenu->addChild(closeBtn);
        this->m_mainLayer->addChild(closeMenu);

        this->setKeypadEnabled(true);
        this->setTouchEnabled(true);

        return true;
    }

    void onClose(CCObject*) {
        this->setKeyboardEnabled(false);
        this->removeFromParentAndCleanup(true);
    }

    void onSelectMacro(CCObject* sender) {
        auto btn = static_cast<CCNode*>(sender);
        std::string macroName = btn->getID();
        
        auto& engine = MacroEngine::get();
        engine.m_inputs = engine.m_savedMacros[macroName];
        engine.m_state = MacroEngine::State::Playing;
        
        geode::Notification::create("Playing: " + macroName, NotificationIcon::Info)->show();
        
        this->onClose(nullptr);
        if (m_pauseLayer) m_pauseLayer->onRestart(nullptr);
    }

public:
    static MacroSelectMenu* create(PauseLayer* pauseLayer) {
        auto ret = new MacroSelectMenu();
        if (ret && ret->init(pauseLayer)) {
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }
};


// ==========================================
// MENU 1: THE MAIN PAUSE MENU HUB
// ==========================================
class MacroModMenu : public FLAlertLayer {
protected:
    PauseLayer* m_pauseLayer;

    bool init(PauseLayer* pauseLayer) {
        if (!FLAlertLayer::init(75)) return false;
        m_pauseLayer = pauseLayer;

        auto winSize = CCDirector::sharedDirector()->getWinSize();
        
        auto bg = CCScale9Sprite::create("GJ_square01.png", {0, 0, 80, 80});
        bg->setContentSize({320.f, 240.f});
        bg->setPosition(winSize.width / 2, winSize.height / 2);
        this->m_mainLayer->addChild(bg);
        
        auto title = CCLabelBMFont::create("Macro Menu", "goldFont.fnt");
        title->setPosition(winSize.width / 2, winSize.height / 2 + 95.f);
        title->setScale(0.8f);
        this->m_mainLayer->addChild(title);
        
        auto menu = CCMenu::create();
        menu->setPosition(winSize.width / 2, winSize.height / 2);
        
        auto recBtn = CCMenuItemSpriteExtra::create(
            ButtonSprite::create("Record New Attempt"),
            this, menu_selector(MacroModMenu::onRecord)
        );
        
        auto playBtn = CCMenuItemSpriteExtra::create(
            ButtonSprite::create("Play Macro"),
            this, menu_selector(MacroModMenu::onPlay)
        );

        auto stopBtn = CCMenuItemSpriteExtra::create(
            ButtonSprite::create("Stop / Save Macro"),
            this, menu_selector(MacroModMenu::onStop)
        );
        
        menu->addChild(recBtn);
        menu->addChild(playBtn);
        menu->addChild(stopBtn);
        menu->setLayout(ColumnLayout::create()->setGap(10.f));
        this->m_mainLayer->addChild(menu);

        auto closeBtnSpr = CCSprite::createWithSpriteFrameName("GJ_closeBtn_001.png");
        auto closeBtn = CCMenuItemSpriteExtra::create(
            closeBtnSpr, this, menu_selector(MacroModMenu::onClose)
        );
        
        auto closeMenu = CCMenu::create();
        closeMenu->setPosition(winSize.width / 2 - 145.f, winSize.height / 2 + 105.f);
        closeMenu->addChild(closeBtn);
        this->m_mainLayer->addChild(closeMenu);

        this->setKeypadEnabled(true);
        this->setTouchEnabled(true);

        return true;
    }

    void onClose(CCObject*) {
        this->setKeyboardEnabled(false);
        this->removeFromParentAndCleanup(true);
    }

    void keyBackClicked() override {
        this->onClose(nullptr);
    }

    void onRecord(CCObject*) {
        MacroEngine::get().m_state = MacroEngine::State::Recording;
        MacroEngine::get().m_inputs.clear(); // Fresh start
        geode::Notification::create("Recording Started", NotificationIcon::Info)->show();
        
        this->onClose(nullptr);
        if (m_pauseLayer) m_pauseLayer->onRestart(nullptr); 
    }

    void onPlay(CCObject*) {
        if (MacroEngine::get().m_savedMacros.empty()) {
            geode::Notification::create("No macros saved yet!", NotificationIcon::Warning)->show();
            return;
        }
        
        // Open the selection menu and close this one
        MacroSelectMenu::create(m_pauseLayer)->show();
        this->onClose(nullptr);
    }

    void onStop(CCObject*) {
        auto& engine = MacroEngine::get();
        if (engine.m_state == MacroEngine::State::Recording) {
            engine.saveCurrentMacro();
            geode::Notification::create("Macro Saved!", NotificationIcon::Success)->show();
        } else {
            geode::Notification::create("Macro Stopped", NotificationIcon::Info)->show();
        }
        
        engine.m_state = MacroEngine::State::Idle;
        this->onClose(nullptr);
    }

public:
    static MacroModMenu* create(PauseLayer* pauseLayer) {
        auto ret = new MacroModMenu();
        if (ret && ret->init(pauseLayer)) {
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }
};
