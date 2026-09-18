#include "Macro.hpp"
#include <algorithm>

namespace cbot {
void Macro::clear() {
    events.clear();
    startTime = 0.0;
    endTime = 0.0;
}

void Macro::add(InputEvent e) {
    if (events.empty()) {
        startTime = e.time;
    }
    endTime = std::max(endTime, e.time);
    events.push_back(e);
}
}
