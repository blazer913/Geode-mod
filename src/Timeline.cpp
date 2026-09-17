#include "Timeline.hpp"
#include <algorithm>

namespace twa {

void InputTimeline::clear() {
    m_events.clear();
}

void InputTimeline::add(InputEvent event) {
    m_events.push_back(event);
}

void InputTimeline::sortAndNormalize() {
    std::stable_sort(m_events.begin(), m_events.end(),
        [](auto const& a, auto const& b) {
            if (a.frame != b.frame) return a.frame < b.frame;
            return a.sourceIndex < b.sourceIndex;
        });
}

int64_t InputTimeline::firstFrame() const {
    return m_events.empty() ? 0 : m_events.front().frame;
}

int64_t InputTimeline::lastFrame() const {
    return m_events.empty() ? 0 : m_events.back().frame;
}

InputTimeline InputTimeline::shifted(size_t index, int deltaFrames) const {
    InputTimeline out = *this;
    if (index >= out.m_events.size()) return out;

    auto& selected = out.m_events[index];
    selected.frame += deltaFrames;

    // Keep a held press/release pair together when it can be identified.
    if (selected.down) {
        for (size_t i = index + 1; i < out.m_events.size(); ++i) {
            auto& e = out.m_events[i];
            if (e.button == selected.button &&
                e.player2 == selected.player2 &&
                !e.down) {
                e.frame += deltaFrames;
                break;
            }
        }
    }

    out.sortAndNormalize();
    return out;
}

}
