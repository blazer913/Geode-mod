#include "GameplayProbe.hpp"

namespace twa {

bool GeometryDashProbe::test(
    InputTimeline const&,
    size_t,
    int
) {
    // Safety against silently generating bogus results. This becomes the
    // PlayLayer/CheckpointObject integration once the exact SDK symbols are
    // verified against the user's installed Geode SDK.
    return false;
}

}
