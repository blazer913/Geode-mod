#pragma once
#include <Geode/Geode.hpp>
#include <string>

namespace cbot {

class CbotOverlay : public cocos2d::CCLayer {
public:
    static CbotOverlay* create();
    bool init() override;

    void setFrame(int frame);
    void setStatus(std::string const& status);

private:
    cocos2d::CCLabelBMFont* m_frameLabel = nullptr;
    cocos2d::CCLabelBMFont* m_statusLabel = nullptr;
};

}
