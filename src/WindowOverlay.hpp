#pragma once
#include "WindowAnalyzer.hpp"
#include <Geode/Geode.hpp>
#include <functional>
#include <string>

namespace twa {

class WindowOverlay final : public cocos2d::CCLayer {
public:
    static WindowOverlay* create();

    bool init() override;
    void setWindows(std::vector<InputWindow> windows);
    void setStatus(std::string const& text);
    void setVisibleForUser(bool visible);
    bool visibleForUser() const { return m_userVisible; }

    void setOnAnalyzeClicked(std::function<void()> cb) { m_onAnalyzeClicked = std::move(cb); }

private:
    bool ccTouchBegan(cocos2d::CCTouch* touch, cocos2d::CCEvent* event) override;
    void ccTouchMoved(cocos2d::CCTouch* touch, cocos2d::CCEvent* event) override;
    void ccTouchEnded(cocos2d::CCTouch* touch, cocos2d::CCEvent* event) override;

    void onAnalyzeButton(cocos2d::CCObject*);
    void rebuild();
    void relayout();
    void savePosition();
    void loadPosition();

    cocos2d::CCPoint m_dragOffset{};
    bool m_dragging = false;
    bool m_userVisible = true;
    cocos2d::CCLabelBMFont* m_text = nullptr;
    cocos2d::CCLabelBMFont* m_status = nullptr;
    cocos2d::CCMenu* m_menu = nullptr;
    std::function<void()> m_onAnalyzeClicked;
};

}
