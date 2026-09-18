#pragma once
#include <Geode/Geode.hpp>
#include "macro.hpp"

using namespace geode::prelude;

// Inherit directly from the game's native alert layer
class MacroModMenu : public FLAlertLayer {
protected:
    bool init() {
        // 75 is the standard background dim opacity
        if (!FLAlertLayer::init(75)) return false;

        auto winSize = CCDirector::sharedDirector()->getWinSize();
        
        // 1. Create the Background
        auto bg = CCScale9Sprite::create("GJ_square01.png", {0, 0, 80, 80});
        bg->setContentSize({320.f, 240.f});
        bg->setPosition(winSize.width / 2, winSize.height / 2);
        this->m_mainLayer->addChild(bg);
        
        // 2. Create the Title
        auto title = CCLabelBMFont::create("Macro Settings", "goldFont.fnt");
        title->setPosition(winSize.width / 2, winSize.height / 2 + 95.f);
        title->setScale(0.8f);
        this->m_mainLayer->addChild(title);
        
        // 3. Create the Main Buttons
        auto menu = CCMenu::create();
        menu->setPosition(winSize.width / 2, winSize.height / 2);
        
        auto toggleSpr = ButtonSprite::create("Toggle Macro");
        auto toggleBtn = CCMenuItemSpriteExtra::create(
            toggleSpr,
            this,
            menu_selector(MacroModMenu::onToggleMacro)
        );
        
        menu->addChild(toggleBtn);
        menu->setLayout(ColumnLayout::create()->setGap(10.f));
        this->m_mainLayer->addChild(menu);

        // 4. Create the Close Button
        auto closeBtnSpr = CCSprite::createWithSpriteFrameName("GJ_closeBtn_001.png");
        auto closeBtn = CCMenuItemSpriteExtra::create(
            closeBtnSpr,
            this,
            menu_selector(MacroModMenu::onClose)
        );
        
        auto closeMenu = CCMenu::create();
        closeMenu->setPosition(winSize.width / 2 - 145.f, winSize.height / 2 + 105.f);
        closeMenu->addChild(closeBtn);
        this->m_mainLayer->addChild(closeMenu);

        // 5. Enable inputs
        this->setKeypadEnabled(true);
        this->setTouchEnabled(true);

        return true;
    }

    void onClose(CCObject*) {
        this->setKeyboardEnabled(false);
        this->removeFromParentAndCleanup(true);
    }

    // Allows the Escape key / Android Back button to close the menu
    void keyBackClicked() override {
        this->onClose(nullptr);
    }

    void onToggleMacro(CCObject* sender) {
        log::info("Macro button pressed.");
        FLAlertLayer::create("Macro", "Macro toggled!", "OK")->show();
    }

public:
    static MacroModMenu* create() {
        auto ret = new MacroModMenu();
        if (ret && ret->init()) {
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }
};
0
