#pragma once
#include <Geode/Geode.hpp>
#include "Controller.hpp"
#include <functional>
#include <string>

namespace cbot {

class CbotOverlay : public cocos2d::CCLayer {
public:
    static CbotOverlay* create();
    bool init() override;

    void setFrame(int frame);
    void setStatus(std::string const& status);
    void setCounterVisible(bool visible);
    void setOnCommand(std::function<void(Command)> cb) { m_onCommand = std::move(cb); }

private:
    void onRecord(cocos2d::CCObject*);
    void onPlay(cocos2d::CCObject*);
    void onToggleCounter(cocos2d::CCObject*);

    cocos2d::CCNode* m_counterNode = nullptr;
    cocos2d::CCLabelBMFont* m_frameLabel = nullptr;
    cocos2d::CCLabelBMFont* m_statusLabel = nullptr;
    std::function<void(Command)> m_onCommand;
};

}
