#pragma once
#include <Geode/Geode.hpp>
#include <Geode/ui/Popup.hpp>
#include "Controller.hpp"
#include <functional>

namespace cbot {

class CbotSettingsPopup : public geode::Popup<> {
protected:
    bool init();

public:
    static CbotSettingsPopup* create();
    void setOnCommand(std::function<void(Command)> cb) { m_onCommand = std::move(cb); }

private:
    void showTab(int index);
    void refreshLabels();

    void onTabMacros(cocos2d::CCObject*);
    void onTabFrameCounter(cocos2d::CCObject*);
    void onRecord(cocos2d::CCObject*);
    void onPlay(cocos2d::CCObject*);
    void onToggleCounter(cocos2d::CCObject*);
    void onCalculate(cocos2d::CCObject*);

    cocos2d::CCNode* m_macrosPanel = nullptr;
    cocos2d::CCNode* m_frameCounterPanel = nullptr;

    cocos2d::CCLabelBMFont* m_macroStatusLabel = nullptr;
    cocos2d::CCLabelBMFont* m_macroLengthLabel = nullptr;
    CCMenuItemSpriteExtra* m_recordBtn = nullptr;
    CCMenuItemSpriteExtra* m_playBtn = nullptr;

    cocos2d::CCLabelBMFont* m_counterStateLabel = nullptr;
    cocos2d::CCLabelBMFont* m_calculateResultLabel = nullptr;

    std::function<void(Command)> m_onCommand;
};

}
