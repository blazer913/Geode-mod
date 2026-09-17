#include "WindowAnalyzer.hpp"
#include <algorithm>

namespace twa {

std::vector<InputWindow> WindowAnalyzer::analyze(
    InputTimeline const& timeline,
    IGameplayProbe& probe,
    int maxWindow,
    ProgressFn progress
) const {
    std::vector<InputWindow> result;
    result.reserve(timeline.size());

    AnalysisProgress p{0, timeline.size()};
    if (progress) progress(p);

    for (size_t i = 0; i < timeline.size(); ++i) {
        InputWindow w;
        w.eventIndex = i;
        w.centerSurvives = probe.test(timeline, i, 0);

        // Search the two directions independently. The displayed width is
        // the contiguous valid interval containing the original timing.
        int left = 0;
        int right = 0;

        for (int d = 1; d <= maxWindow; ++d) {
            if (probe.test(timeline, i, -d)) left = -d;
            else break;
        }

        for (int d = 1; d <= maxWindow; ++d) {
            if (probe.test(timeline, i, +d)) right = d;
            else break;
        }

        if (w.centerSurvives) {
            w.left = left;
            w.right = right;
            w.width = right - left + 1;
        } else {
            w.left = w.right = 0;
            w.width = 0;
        }

        result.push_back(w);

        p.completed = i + 1;
        if (progress) progress(p);
    }

    return result;
}

}
