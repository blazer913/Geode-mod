#pragma once
#include "Macro.hpp"
#include "Controller.hpp"

namespace cbot {

class CbotOverlay;

class State {
public:
    Macro macro;
    Controller controller;
    bool recording = false;
    bool playing = false;
    bool overlayOn = false;
    int frame = 0;
    size_t playIndex = 0;
    CbotOverlay* hud = nullptr;

    void refreshHud();
};

State& getState();

}
