#pragma once
#include <cstdint>
#include <vector>

namespace twa {

enum class ActionType : uint8_t {
    Press = 1,
    Release = 2,
    Unknown = 255
};

struct InputEvent {
    int64_t frame = 0;
    int button = 0;
    bool down = false;
    bool player2 = false;
    uint32_t sourceIndex = 0;
};

class InputTimeline {
public:
    void clear();
    void add(InputEvent event);
    void sortAndNormalize();

    std::vector<InputEvent> const& events() const { return m_events; }
    std::vector<InputEvent>& events() { return m_events; }

    size_t size() const { return m_events.size(); }
    int64_t firstFrame() const;
    int64_t lastFrame() const;

    InputTimeline shifted(size_t index, int deltaFrames) const;

private:
    std::vector<InputEvent> m_events;
};

}
