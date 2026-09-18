#pragma once
#include <Geode/Geode.hpp>
#include <Geode/ui/Notification.hpp>
#include "macro.hpp"

using namespace geode::prelude;

class MacroModMenu : public FLAlertLayer {
protected:
    PauseLayer* m_pauseLayer;

    bool init(PauseLayer* pauseLayer) {
        if (!FLAlertLayer::init(75)) return false;
        
        // Save the pause layer so we can trigger a restart from it
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
        
        // 1. Record Button
        auto recBtn = CCMenuItemSpriteExtra::create(
            ButtonSprite::create("Record New Attempt"),
            this, menu_selector(MacroModMenu::onRecord)
        );
        
        // 2. Play Button
        auto playBtn = CCMenuItemSpriteExtra::create(
            ButtonSprite::create("Play Macro"),
            this, menu_selector(MacroModMenu::onPlay)
        );

        // 3. Stop Button
        auto stopBtn = CCMenuItemSpriteExtra::create(
            ButtonSprite::create("Stop / Idle"),
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
        // Triggers the "Restart" button in the pause menu natively
        if (m_pauseLayer) m_pauseLayer->onRestart(nullptr); 
    }

    void onPlay(CCObject*) {
        if (MacroEngine::get().m_inputs.empty()) {
            geode::Notification::create("No macro recorded!", NotificationIcon::Warning)->show();
            return;
        }
        MacroEngine::get().m_state = MacroEngine::State::Playing;
        geode::Notification::create("Playback Started", NotificationIcon::Info)->show();
        
        this->onClose(nullptr);
        if (m_pauseLayer) m_pauseLayer->onRestart(nullptr);
    }

    void onStop(CCObject*) {
        MacroEngine::get().m_state = MacroEngine::State::Idle;
        geode::Notification::create("Macro Stopped", NotificationIcon::Info)->show();
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
