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
    LampState update(bool markerFound, double markerAngle);
};

#endif //CAMERA_GRAB_INTERACTIONMAPPING_H