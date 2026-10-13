//
// Created by Kevin Taverner on 2026-10-13.
//

#ifndef CAMERA_GRAB_LAMPRENDERER_H
#define CAMERA_GRAB_LAMPRENDERER_H

#pragma once

#include <opencv2/opencv.hpp>
#include "InteractionMapping.h"

class LampRenderer {
public:
    cv::Mat render(const LampState& lamp);
};

#endif //CAMERA_GRAB_LAMPRENDERER_H
