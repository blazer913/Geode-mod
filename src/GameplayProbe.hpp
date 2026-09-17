#pragma once
#include "WindowAnalyzer.hpp"

namespace twa {

// Game-specific adapter. This intentionally returns false until the
// checkpoint/replay integration is connected to the target GD build.
class GeometryDashProbe final : public IGameplayProbe {
public:
    bool test(InputTimeline const& timeline,
              size_t changedEvent,
              int deltaFrames) override;

    void setEnabled(bool enabled) { m_enabled = enabled; }
    bool enabled() const { return m_enabled; }

private:
    bool m_enabled = false;
};

}
