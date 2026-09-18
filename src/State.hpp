#pragma once
#include "Macro.hpp"
#include "Controller.hpp"
#include <vector>

namespace cbot {
class CbotOverlay;

struct CheckpointData {
    double time = 0.0;
    size_t macroSize = 0;
    size_t playIndex = 0;
};

class State {
public:
    Macro macro;
    Controller controller;
    bool recording = false;
    bool playing = false;
    bool overlayOn = false;
    
    double time = 0.0;
    size_t playIndex = 0;
    bool needsReset = false; // Defers level reset until the game is unpaused

    CbotOverlay* hud = nullptr;
    std::vector<CheckpointData> checkpoints;

    void refreshHud();
};

State& getState();
}
