#pragma once
#include <string>
#include <vector>

namespace cbot {
enum class Button : int { Jump=1, Left=2, Right=3 };

struct InputEvent {
    double time = 0.0;
    Button button = Button::Jump;
    bool pressed = false;
    int player = 1;
};

struct Macro {
    std::string name;
    double startTime = 0.0;
    double endTime = 0.0;
    std::vector<InputEvent> events;

    void clear() { events.clear(); startTime = endTime = 0.0; }
    void add(InputEvent e) {
        if (events.empty()) startTime = e.time;
        endTime = std::max(endTime, e.time);
        events.push_back(e);
    }
};
}
