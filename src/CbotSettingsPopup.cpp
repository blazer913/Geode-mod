#include "CbotSettingsPopup.hpp"
#include "State.hpp"

using namespace cocos2d;
using namespace cocos2d::extension;
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
    if (!Popup::init(320.f, 220.f)) return false;
    this->setTitle("CBot Manager");

    auto winSize = CCDirector::sharedDirector()->getWinSize();

    // -- Background Panel --
    auto panelBg = CCScale9Sprite::create("square02_small.png");
    panelBg->setContentSize({280.f, 110.f});
    panelBg->setPosition({winSize.width / 2, winSize.height / 2 - 10.f});
    panelBg->setOpacity(100);
    m_mainLayer->addChild(panelBg);

    // -- Macro Controls (Centered inside panel) --
    m_macrosPanel = CCMenu::create();
    m_macrosPanel->setPosition({winSize.width / 2, winSize.height / 2 - 10.f});
    m_macrosPanel->setLayout(
        ColumnLayout::create()->setGap(8.f)->setAxisAlignment(AxisAlignment::Center)
    );

    m_macroStatusLabel = CCLabelBMFont::create("IDLE", "goldFont.fnt");
    m_macroStatusLabel->setScale(0.6f);
    m_macrosPanel->addChild(m_macroStatusLabel);

    m_macroLengthLabel = CCLabelBMFont::create("0 events", "chatFont.fnt");
    m_macroLengthLabel->setScale(0.6f);
    m_macrosPanel->addChild(m_macroLengthLabel);

    auto btnMenu = CCMenu::create();
    btnMenu->setLayout(RowLayout::create()->setGap(15.f));
    
    auto recordSpr = ButtonSprite::create("Record", "goldFont.fnt", "GJ_button_04.png", .8f);
    m_recordBtn = CCMenuItemSpriteExtra::create(recordSpr, this, menu_selector(CbotSettingsPopup::onRecord));
    btnMenu->addChild(m_recordBtn);

    auto playSpr = ButtonSprite::create("Play", "goldFont.fnt", "GJ_button_01.png", .8f);
    m_playBtn = CCMenuItemSpriteExtra::create(playSpr, this, menu_selector(CbotSettingsPopup::onPlay));
    btnMenu->addChild(m_playBtn);

    btnMenu->updateLayout();
    m_macrosPanel->addChild(btnMenu);
    m_macrosPanel->updateLayout();
    m_mainLayer->addChild(m_macrosPanel);

    // -- Bottom Toggles --
    auto toggleMenu = CCMenu::create();
    toggleMenu->setPosition({winSize.width / 2, winSize.height / 2 - 85.f});
    
    auto toggleSpr = ButtonSprite::create("Toggle HUD Overlay");
    toggleSpr->setScale(.6f);
    auto toggleBtn = CCMenuItemSpriteExtra::create(toggleSpr, this, menu_selector(CbotSettingsPopup::onToggleCounter));
    toggleMenu->addChild(toggleBtn);
    
    m_mainLayer->addChild(toggleMenu);

    refreshLabels();
    return true;
}

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

void CbotSettingsPopup::refreshLabels() {
    auto& state = getState();
    if (state.recording) m_macroStatusLabel->setString("STATUS: RECORDING");
    else if (state.playing) m_macroStatusLabel->setString("STATUS: PLAYING");
    else m_macroStatusLabel->setString("STATUS: IDLE");

    m_macroLengthLabel->setString((std::to_string(state.macro.events.size()) + " events").c_str());
}

}
