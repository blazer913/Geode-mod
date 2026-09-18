#include "CbotOverlay.hpp"

using namespace geode::prelude;

namespace cbot {
CbotOverlay* CbotOverlay::create() {
    auto ret = new CbotOverlay;
    if (ret && ret->initAnchored(190.f, 115.f)) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool CbotOverlay::setup() {
    this->setTitle("CBot");

    m_frame = CCLabelBMFont::create("FRAME 0", "bigFont.fnt");
    m_frame->setScale(.45f);
    m_frame->setPosition({25.f, 72.f});
    m_frame->setAnchorPoint({0.f, .5f});
    m_mainLayer->addChild(m_frame);

    m_status = CCLabelBMFont::create("IDLE", "goldFont.fnt");
    m_status->setScale(.42f);
    m_status->setPosition({25.f, 47.f});
    m_status->setAnchorPoint({0.f, .5f});
    m_mainLayer->addChild(m_status);

    return true;
}

void CbotOverlay::setFrame(int frame) {
    if (m_frame)
        m_frame->setString(("FRAME " + std::to_string(frame)).c_str());
}

void CbotOverlay::setStatus(const char* status) {
    if (m_status) m_status->setString(status);
}
}
