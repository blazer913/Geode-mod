#pragma once
#include <string>
#include <vector>

namespace cbot {
enum class Button : int { Jump=1, Left=2, Right=3 };

struct InputEvent {
    int frame = 0;
    Button button = Button::Jump;
    bool pressed = false;
    int player = 1;
};

struct Macro {
    std::string name;
    int startFrame = 0;
    int endFrame = 0;
    std::vector<InputEvent> events;

    void clear();
    void add(InputEvent e);
};
}
