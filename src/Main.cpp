#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include "TTRL.hpp"
#include "GameplayProbe.hpp"
#include "WindowAnalyzer.hpp"
#include "WindowOverlay.hpp"

using namespace geode::prelude;
using namespace cocos2d;

namespace {

class $modify(WindowAnalyzerPlayLayer, PlayLayer) {
    struct Fields {
        twa::WindowOverlay* overlay = nullptr;
        bool attached = false;
    };

    bool init(GJGameLevel* level, bool useReplay, bool dontCreateObjects) {
        if (!PlayLayer::init(level, useReplay, dontCreateObjects))
            return false;

        auto overlay = twa::WindowOverlay::create();
        if (overlay) {
            this->addChild(overlay, 9999);
            m_fields->overlay = overlay;
            m_fields->attached = true;
        }

        return true;
    }

    void onQuit() {
        if (m_fields->overlay) {
            m_fields->overlay->removeFromParentAndCleanup(true);
            m_fields->overlay = nullptr;
        }
        PlayLayer::onQuit();
    }
};

}

$execute {
    log::info("Toasty Window Analyzer v0.2.0 loaded.");
}
