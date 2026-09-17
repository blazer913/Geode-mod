#pragma once
#include "Timeline.hpp"
#include <cstddef>
#include <functional>
#include <vector>

namespace twa {

struct InputWindow {
    size_t eventIndex = 0;
    int left = 0;
    int right = 0;
    int width = 0;
    bool centerSurvives = false;
};

class IGameplayProbe {
public:
    virtual ~IGameplayProbe() = default;

    virtual bool test(InputTimeline const& timeline,
                       size_t changedEvent,
                       int deltaFrames) = 0;
};

struct AnalysisProgress {
    size_t completed = 0;
    size_t total = 0;
};

class WindowAnalyzer {
public:
    using ProgressFn = std::function<void(AnalysisProgress)>;

    std::vector<InputWindow> analyze(
        InputTimeline const& timeline,
        IGameplayProbe& probe,
        int maxWindow = 10,
        ProgressFn progress = {}
    ) const;
};

}
