#include "Macro.hpp"
#include <algorithm>
namespace cbot {
void Macro::clear() { events.clear(); startFrame=endFrame=0; }
void Macro::add(InputEvent e) {
    if (events.empty()) startFrame=e.frame;
    endFrame=std::max(endFrame,e.frame);
    events.push_back(e);
}
}
