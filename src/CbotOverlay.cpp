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

    m_frameLabel = CCLabelBMFont::create("TIME 0ms", "bigFont.fnt");
    m_frameLabel->setScale(.45f);
    m_frameLabel->setAnchorPoint({0.f, 1.f});
    m_frameLabel->setPosition({12.f, -50.f});
    this->addChild(m_frameLabel);

    m_statusLabel = CCLabelBMFont::create("IDLE", "goldFont.fnt");
    m_statusLabel->setScale(.4f);
    m_statusLabel->setAnchorPoint({0.f, 1.f});
    m_statusLabel->setPosition({12.f, -76.f});
    this->addChild(m_statusLabel);

    auto winSize = CCDirector::sharedDirector()->getWinSize();
    this->setPosition({0.f, winSize.height});
    this->setVisible(false);

    return true;
}

void CbotOverlay::setFrame(int timeMs) {
    if (m_frameLabel)
        m_frameLabel->setString(("TIME " + std::to_string(timeMs) + "ms").c_str());
}

void CbotOverlay::setStatus(std::string const& status) {
    if (m_statusLabel)
        m_statusLabel->setString(status.c_str());
}

}
