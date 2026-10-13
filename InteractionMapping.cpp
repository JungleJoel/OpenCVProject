
#include "InteractionMapping.h"

#include <algorithm>
#include <cmath>

LampState InteractionMapping::update(
    bool markerFound,
    double markerAngle
) {
    if (!markerFound || !std::isfinite(markerAngle)) {
        return {false, 0.0};
    }

    double brightness = std::clamp(
        (markerAngle + 90.0) / 180.0,
        0.0,
        1.0
    );

    return {true, brightness};
}
