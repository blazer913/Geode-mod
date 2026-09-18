#include "CbotSettingsPopup.hpp"
#include "State.hpp"

using namespace cocos2d;
using namespace geode::prelude;

namespace cbot {

extern State& getState();

CbotSettingsPopup* CbotSettingsPopup::create() {
    auto p = new CbotSettingsPopup();
    if (p->init()) {
        p->autorelease();
        return p;
    }
    delete p;
    return nullptr;
}

bool CbotSettingsPopup::init() {
    if (!Popup::init(280.f, 200.f))
        return false;

    this->setTitle("CBot");

    auto tabMenu = CCMenu::create();
    tabMenu->setPosition({140.f, 172.f});

    auto macrosSpr = ButtonSprite::create("Macros");
    macrosSpr->setScale(.6f);
    auto macrosBtn = CCMenuItemSpriteExtra::create(macrosSpr, this, menu_selector(CbotSettingsPopup::onTabMacros));
    macrosBtn->setPosition({-50.f, 0.f});
    tabMenu->addChild(macrosBtn);

    auto frameSpr = ButtonSprite::create("Frame Counter");
    frameSpr->setScale(.6f);
    auto frameBtn = CCMenuItemSpriteExtra::create(frameSpr, this, menu_selector(CbotSettingsPopup::onTabFrameCounter));
    frameBtn->setPosition({50.f, 0.f});
    tabMenu->addChild(frameBtn);

    m_mainLayer->addChild(tabMenu);

    // --- Macros panel ---
    m_macrosPanel = CCNode::create();
    m_macrosPanel->setPosition({20.f, 60.f});

    m_macroStatusLabel = CCLabelBMFont::create("IDLE", "goldFont.fnt");
    m_macroStatusLabel->setScale(.5f);
    m_macroStatusLabel->setAnchorPoint({0.f, .5f});
    m_macroStatusLabel->setPosition({0.f, 70.f});
    m_macrosPanel->addChild(m_macroStatusLabel);

    m_macroLengthLabel = CCLabelBMFont::create("0 events", "chatFont.fnt");
    m_macroLengthLabel->setScale(.5f);
    m_macroLengthLabel->setAnchorPoint({0.f, .5f});
    m_macroLengthLabel->setPosition({0.f, 50.f});
    m_macrosPanel->addChild(m_macroLengthLabel);

    auto macroMenu = CCMenu::create();
    macroMenu->setPosition({0.f, 15.f});
    macroMenu->setAnchorPoint({0.f, .5f});

    auto recordSpr = ButtonSprite::create("Record");
    recordSpr->setScale(.6f);
    m_recordBtn = CCMenuItemSpriteExtra::create(recordSpr, this, menu_selector(CbotSettingsPopup::onRecord));
    m_recordBtn->setPosition({35.f, 0.f});
    m_recordBtn->setAnchorPoint({0.f, .5f});
    macroMenu->addChild(m_recordBtn);

    auto playSpr = ButtonSprite::create("Play");
    playSpr->setScale(.6f);
    m_playBtn = CCMenuItemSpriteExtra::create(playSpr, this, menu_selector(CbotSettingsPopup::onPlay));
    m_playBtn->setPosition({115.f, 0.f});
    m_playBtn->setAnchorPoint({0.f, .5f});
    macroMenu->addChild(m_playBtn);

    m_macrosPanel->addChild(macroMenu);
    m_mainLayer->addChild(m_macrosPanel);

    // --- Frame Counter panel ---
    m_frameCounterPanel = CCNode::create();
    m_frameCounterPanel->setPosition({20.f, 60.f});
    m_frameCounterPanel->setVisible(false);

    m_counterStateLabel = CCLabelBMFont::create("Counter: OFF", "goldFont.fnt");
    m_counterStateLabel->setScale(.5f);
    m_counterStateLabel->setAnchorPoint({0.f, .5f});
    m_counterStateLabel->setPosition({0.f, 70.f});
    m_frameCounterPanel->addChild(m_counterStateLabel);

    auto counterMenu = CCMenu::create();
    counterMenu->setAnchorPoint({0.f, .5f});
    counterMenu->setPosition({0.f, 45.f});

    auto toggleSpr = ButtonSprite::create("Toggle");
    toggleSpr->setScale(.6f);
    auto toggleBtn = CCMenuItemSpriteExtra::create(toggleSpr, this, menu_selector(CbotSettingsPopup::onToggleCounter));
    toggleBtn->setPosition({35.f, 0.f});
    toggleBtn->setAnchorPoint({0.f, .5f});
    counterMenu->addChild(toggleBtn);

    auto calcSpr = ButtonSprite::create("Calculate");
    calcSpr->setScale(.6f);
    auto calcBtn = CCMenuItemSpriteExtra::create(calcSpr, this, menu_selector(CbotSettingsPopup::onCalculate));
    calcBtn->setPosition({120.f, 0.f});
    calcBtn->setAnchorPoint({0.f, .5f});
    counterMenu->addChild(calcBtn);

    m_frameCounterPanel->addChild(counterMenu);

    m_calculateResultLabel = CCLabelBMFont::create("", "chatFont.fnt");
    m_calculateResultLabel->setScale(.45f);
    m_calculateResultLabel->setAnchorPoint({0.f, .5f});
    m_calculateResultLabel->setPosition({0.f, 15.f});
    m_frameCounterPanel->addChild(m_calculateResultLabel);

    m_mainLayer->addChild(m_frameCounterPanel);

    refreshLabels();
    return true;
}

void CbotSettingsPopup::showTab(int index) {
    m_macrosPanel->setVisible(index == 0);
    m_frameCounterPanel->setVisible(index == 1);
}

void CbotSettingsPopup::onTabMacros(CCObject*) { showTab(0); }
void CbotSettingsPopup::onTabFrameCounter(CCObject*) { showTab(1); }

void CbotSettingsPopup::onRecord(CCObject*) {
    if (m_onCommand) m_onCommand(Command::RecordToggle);
    refreshLabels();
}

void CbotSettingsPopup::onPlay(CCObject*) {
    if (m_onCommand) m_onCommand(Command::PlayPause);
    refreshLabels();
}

void CbotSettingsPopup::onToggleCounter(CCObject*) {
    if (m_onCommand) m_onCommand(Command::OverlayToggle);
    refreshLabels();
}

void CbotSettingsPopup::onCalculate(CCObject*) {
    // Default behaviour: total recorded length of the current macro.
    // Tell me if you actually meant something else here (e.g. per-input
    // timing windows) and I'll swap this out.
    auto& state = getState();
    int total = state.macro.events.empty() ? 0 : (state.macro.endFrame - state.macro.startFrame);
    m_calculateResultLabel->setString(("Length: " + std::to_string(total) + " frames").c_str());
}

void CbotSettingsPopup::refreshLabels() {
    auto& state = getState();

    if (state.recording) m_macroStatusLabel->setString("RECORDING");
    else if (state.playing) m_macroStatusLabel->setString("PLAYING");
    else m_macroStatusLabel->setString("IDLE");

    m_macroLengthLabel->setString((std::to_string(state.macro.events.size()) + " events").c_str());
    m_counterStateLabel->setString(state.overlayOn ? "Counter: ON" : "Counter: OFF");
}

}
