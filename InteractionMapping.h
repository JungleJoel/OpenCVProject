//
// Created by Kevin Taverner on 2026-10-13.
//

#ifndef CAMERA_GRAB_INTERACTIONMAPPING_H
#define CAMERA_GRAB_INTERACTIONMAPPING_H

#pragma once

struct LampState {
    bool on = false;
    double brightness = 0.0;
};

class InteractionMapping {
public:
    // markerAngle is the PoseEstimator angle in degrees, -180..180.
    LampState update(bool markerFound, double markerAngle);

    // Mirrors an angle in -180..180 into -90..90: 135 -> 45, -135 -> -45.
    static double foldAngle(double markerAngle);
};

#endif //CAMERA_GRAB_INTERACTIONMAPPING_H