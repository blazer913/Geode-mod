#pragma once
#include <Geode/Geode.hpp>
#include "macro.hpp"

using namespace geode::prelude;

class MacroModMenu : public geode::Popup<> {
protected:
    bool setup() override {
        this->setTitle("Macro Settings");

        auto menu = CCMenu::create();
        
        auto toggleSpr = ButtonSprite::create("Toggle Macro");
        auto toggleBtn = CCMenuItemSpriteExtra::create(
            toggleSpr,
            this,
            menu_selector(MacroModMenu::onToggleMacro)
        );
        
        menu->addChild(toggleBtn);
        
        menu->setLayout(ColumnLayout::create()->setGap(10.f));
        m_mainLayer->addChild(menu);

        return true;
    }

    void onToggleMacro(CCObject* sender) {
        // Replace with your actual macro execution logic from macro.hpp
        log::info("Macro button pressed.");
        FLAlertLayer::create("Macro", "Macro toggled!", "OK")->show();
    }

public:
    static MacroModMenu* create() {
        auto ret = new MacroModMenu();
        if (ret && ret->initAnchored(320.f, 240.f)) { 
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }
};
