#pragma once
#include <Geode/Geode.hpp>
#include <Geode/ui/Notification.hpp>
#include "macro.hpp"

using namespace geode::prelude;

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
        
        auto title = CCLabelBMFont::create("Select File to Load", "goldFont.fnt");
        title->setPosition(winSize.width / 2, winSize.height / 2 + 95.f);
        title->setScale(0.7f);
        this->m_mainLayer->addChild(title);
        
        auto menu = CCMenu::create();
        menu->setPosition(winSize.width / 2, winSize.height / 2);
        
        auto savedFiles = MacroEngine::get().getSavedMacroFiles();
        for (const auto& filename : savedFiles) {
            auto btnSpr = ButtonSprite::create(filename.c_str());
            btnSpr->setScale(0.75f);
            auto btn = CCMenuItemSpriteExtra::create(
                btnSpr, this, menu_selector(MacroSelectMenu::onSelectMacro)
            );
            btn->setID(filename); 
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
        std::string filename = btn->getID();
        
        auto& engine = MacroEngine::get();
        if (engine.loadMacroFromFile(filename)) {
            engine.m_state = MacroEngine::State::Playing;
            geode::Notification::create("Loaded: " + filename, NotificationIcon::Info)->show();
            
            this->onClose(nullptr);
            if (m_pauseLayer) m_pauseLayer->onRestart(nullptr);
        } else {
            geode::Notification::create("Failed to load file", NotificationIcon::Error)->show();
        }
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
            ButtonSprite::create("Load & Play Macro"),
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
        MacroEngine::get().m_inputs.clear(); 
        geode::Notification::create("Recording Started", NotificationIcon::Info)->show();
        
        this->onClose(nullptr);
        if (m_pauseLayer) m_pauseLayer->onRestart(nullptr); 
    }

    void onPlay(CCObject*) {
        auto savedFiles = MacroEngine::get().getSavedMacroFiles();
        if (savedFiles.empty()) {
            geode::Notification::create("No files found in save dir!", NotificationIcon::Warning)->show();
            return;
        }
        
        MacroSelectMenu::create(m_pauseLayer)->show();
        this->onClose(nullptr);
    }

    void onStop(CCObject*) {
        auto& engine = MacroEngine::get();
        if (engine.m_state == MacroEngine::State::Recording) {
            int count = static_cast<int>(engine.m_inputs.size());
            engine.saveCurrentMacroToFile();
            geode::Notification::create(
                "Saved " + std::to_string(count) + " inputs!",
                count > 0 ? NotificationIcon::Success : NotificationIcon::Warning
            )->show();
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
