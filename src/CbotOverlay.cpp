#include "CbotOverlay.hpp"

using namespace cocos2d;
using namespace geode::prelude;

namespace cbot {

CbotOverlay* CbotOverlay::create() {
    auto p = new CbotOverlay();
    if (p && p->init()) {
        p->autorelease();
        return p;
    }
    delete p;
    return nullptr;
}

bool CbotOverlay::init() {
    if (!CCLayer::init()) return false;

    m_counterNode = CCNode::create();
    m_counterNode->setAnchorPoint({0.f, 1.f});
    m_counterNode->setPosition({10.f, -10.f});

    m_frameLabel = CCLabelBMFont::create("FRAME 0", "bigFont.fnt");
    m_frameLabel->setScale(.4f);
    m_frameLabel->setAnchorPoint({0.f, 1.f});
    m_frameLabel->setPosition({0.f, 0.f});
    m_counterNode->addChild(m_frameLabel);

    m_statusLabel = CCLabelBMFont::create("IDLE", "goldFont.fnt");
    m_statusLabel->setScale(.4f);
    m_statusLabel->setAnchorPoint({0.f, 1.f});
    m_statusLabel->setPosition({0.f, -24.f});
    m_counterNode->addChild(m_statusLabel);

    this->addChild(m_counterNode);

    auto menu = CCMenu::create();
    menu->setAnchorPoint({0.f, 1.f});
    menu->setPosition({10.f, -58.f});

    auto recordSpr = ButtonSprite::create("Rec");
    recordSpr->setScale(.55f);
    auto recordBtn = CCMenuItemSpriteExtra::create(recordSpr, this, menu_selector(CbotOverlay::onRecord));
    recordBtn->setPosition({22.f, 0.f});
    menu->addChild(recordBtn);

    auto playSpr = ButtonSprite::create("Play");
    playSpr->setScale(.55f);
    auto playBtn = CCMenuItemSpriteExtra::create(playSpr, this, menu_selector(CbotOverlay::onPlay));
    playBtn->setPosition({75.f, 0.f});
    menu->addChild(playBtn);

    auto hideSpr = ButtonSprite::create("HUD");
    hideSpr->setScale(.55f);
    auto hideBtn = CCMenuItemSpriteExtra::create(hideSpr, this, menu_selector(CbotOverlay::onToggleCounter));
    hideBtn->setPosition({128.f, 0.f});
    menu->addChild(hideBtn);

    this->addChild(menu);

    this->setPosition({0.f, CCDirector::sharedDirector()->getWinSize().height - 4.f});
    return true;
}

void CbotOverlay::setFrame(int frame) {
    if (m_frameLabel)
        m_frameLabel->setString(("FRAME " + std::to_string(frame)).c_str());
}

void CbotOverlay::setStatus(std::string const& status) {
    if (m_statusLabel)
        m_statusLabel->setString(status.c_str());
}

void CbotOverlay::setCounterVisible(bool visible) {
    if (m_counterNode) m_counterNode->setVisible(visible);
}

void CbotOverlay::onRecord(CCObject*) { if (m_onCommand) m_onCommand(Command::RecordToggle); }
void CbotOverlay::onPlay(CCObject*) { if (m_onCommand) m_onCommand(Command::PlayPause); }
void CbotOverlay::onToggleCounter(CCObject*) { if (m_onCommand) m_onCommand(Command::OverlayToggle); }

}
