
#include "InteractionMapping.h"

#include <algorithm>
#include <cmath>

double InteractionMapping::foldAngle(double markerAngle) {
    double a = std::fmod(markerAngle, 360.0);
    if (a > 180.0) {
        a -= 360.0;
    } else if (a < -180.0) {
        a += 360.0;
    }

    if (a > 90.0) {
        return 180.0 - a;
    }
    if (a < -90.0) {
        return -180.0 - a;
    }
    return a;
}

LampState InteractionMapping::update(
    bool markerFound,
    double markerAngle
) {
    if (!markerFound || !std::isfinite(markerAngle)) {
        return {false, 0.0};
    }

    double folded = foldAngle(markerAngle);

    double brightness = std::clamp(
        (folded + 90.0) / 180.0,
        0.0,
        1.0
    );

    return {true, brightness};
}
