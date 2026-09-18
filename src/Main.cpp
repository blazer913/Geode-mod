#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include "Controller.hpp"
#include "Macro.hpp"

using namespace geode::prelude;

namespace cbot {
class State {
public:
    Macro macro;
    Controller controller;
    bool recording = false;
    bool playing = false;
    bool overlay = true;
    int frame = 0;

    State() {
        controller.setCallback([this](Command c) {
            switch (c) {
                case Command::RecordToggle:
                    recording = !recording;
                    if (recording) macro.clear();
                    break;
                case Command::PlayPause:
                    playing = !playing;
                    break;
                case Command::FrameBack:
                    if (frame > 0) --frame;
                    break;
                case Command::FrameForward:
                    ++frame;
                    break;
                case Command::OverlayToggle:
                    overlay = !overlay;
                    break;
            }
        });
    }
};

static State g_state;

class $modify(CBotPlayLayer, PlayLayer) {
    bool init(GJGameLevel* level, bool useReplay, bool dontCreateObjects) {
        if (!PlayLayer::init(level, useReplay, dontCreateObjects))
            return false;
        g_state.frame = 0;
        return true;
    }

    void update(float dt) {
        PlayLayer::update(dt);
        ++g_state.frame;
    }
};
}
