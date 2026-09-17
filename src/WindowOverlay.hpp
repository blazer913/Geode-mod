#pragma once
#include "WindowAnalyzer.hpp"
#include <Geode/Geode.hpp>

namespace twa {

class WindowOverlay final : public cocos2d::CCLayer {
public:
    static WindowOverlay* create();

    bool init() override;
    void setWindows(std::vector<InputWindow> windows);
    void setVisibleForUser(bool visible);
    bool visibleForUser() const { return m_userVisible; }

private:
    bool ccTouchBegan(cocos2d::CCTouch* touch, cocos2d::CCEvent* event) override;
    void ccTouchMoved(cocos2d::CCTouch* touch, cocos2d::CCEvent* event) override;
    void ccTouchEnded(cocos2d::CCTouch* touch, cocos2d::CCEvent* event) override;

    void rebuild();
    void savePosition();
    void loadPosition();

    cocos2d::CCPoint m_dragOffset{};
    bool m_dragging = false;
    bool m_userVisible = true;
    cocos2d::CCLabelBMFont* m_text = nullptr;
};

}
