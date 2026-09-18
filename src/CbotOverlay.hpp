#pragma once
#include <Geode/Geode.hpp>
#include <Geode/ui/GeodeUI.hpp>

namespace cbot {
class CbotOverlay : public geode::Popup<> {
protected:
    bool setup() override;
public:
    static CbotOverlay* create();
    void setFrame(int frame);
    void setStatus(const char* status);
private:
    cocos2d::CCLabelBMFont* m_frame = nullptr;
    cocos2d::CCLabelBMFont* m_status = nullptr;
};
}
