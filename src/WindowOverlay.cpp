#include "WindowOverlay.hpp"
#include <algorithm>
#include <sstream>

using namespace cocos2d;
using namespace geode::prelude;

namespace twa {

WindowOverlay* WindowOverlay::create() {
    auto p = new WindowOverlay();
    if (p && p->init()) {
        p->autorelease();
        return p;
    }
    delete p;
    return nullptr;
}

bool WindowOverlay::init() {
    if (!CCLayer::init()) return false;

    this->setTouchEnabled(true);
    loadPosition();

    m_text = CCLabelBMFont::create("INPUT WINDOWS", "bigFont.fnt");
    m_text->setAnchorPoint({0.f, 1.f});
    m_text->setPosition({8.f, -8.f});
    this->addChild(m_text);

    this->rebuild();
    return true;
}

static char const* colorTagFor(int width) {
    if (width >= 7) return "<co>BLUE</c>";
    if (width >= 4) return "<cg>GREEN</c>";
    if (width >= 2) return "<cy>YELLOW</c>";
    return "<cr>RED</c>";
}

void WindowOverlay::setWindows(std::vector<InputWindow> windows) {
    std::ostringstream out;
    out << "INPUT WINDOWS\n\n";

    int counts[11] = {};
    for (auto const& w : windows) {
        if (w.width >= 1 && w.width <= 10)
            ++counts[w.width];
    }

    for (int width = 10; width >= 1; --width) {
        out << width << " : ";
        if (counts[width] < 10) out << ' ';
        out << counts[width] << "\n";
    }

    out << "\n";
    out << "BLUE 10-7\n";
    out << "GREEN 6-4\n";
    out << "YELLOW 3-2\n";
    out << "RED 1";

    m_text->setString(out.str().c_str());
}

void WindowOverlay::setVisibleForUser(bool visible) {
    m_userVisible = visible;
    this->setVisible(visible);
}

void WindowOverlay::rebuild() {
    setWindows({});
}

bool WindowOverlay::ccTouchBegan(CCTouch* touch, CCEvent*) {
    if (!m_userVisible) return false;
    auto p = this->convertTouchToNodeSpace(touch);
    auto size = m_text->getScaledContentSize();

    CCRect r(0.f, -size.height, size.width + 20.f, size.height + 20.f);
    if (!r.containsPoint(p)) return false;

    m_dragging = true;
    m_dragOffset = p;
    return true;
}

void WindowOverlay::ccTouchMoved(CCTouch* touch, CCEvent*) {
    if (!m_dragging) return;

    auto p = this->getParent()->convertTouchToNodeSpace(touch);
    auto pos = p - m_dragOffset;

    auto win = CCDirector::sharedDirector()->getWinSize();
    pos.x = std::max(0.f, std::min(pos.x, win.width - 20.f));
    pos.y = std::max(20.f, std::min(pos.y, win.height - 20.f));

    this->setPosition(pos);
}

void WindowOverlay::ccTouchEnded(CCTouch*, CCEvent*) {
    if (!m_dragging) return;
    m_dragging = false;
    savePosition();
}

void WindowOverlay::savePosition() {
    auto mod = Mod::get();
    mod->setSavedValue("overlay-x", this->getPositionX());
    mod->setSavedValue("overlay-y", this->getPositionY());
}

void WindowOverlay::loadPosition() {
    auto mod = Mod::get();
    auto win = CCDirector::sharedDirector()->getWinSize();

    float x = mod->getSavedValue("overlay-x", 10.f);
    float y = mod->getSavedValue("overlay-y", win.height - 20.f);

    this->setPosition({x, y});
}

}
