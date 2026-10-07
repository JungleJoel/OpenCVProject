//
// Created by Kevin Taverner on 2026-09-22.
//

#include "CameraInput.h"

CameraInput::CameraInput() = default;

bool CameraInput::openCamera(int cameraIndex) {
    return capture.open(cameraIndex);
}

bool CameraInput::openVideo(const std::string &videoPath) {
    return capture.open(videoPath);
}

bool CameraInput::isOpen() const {
    return capture.isOpened();
}

bool CameraInput::getFrame(cv::Mat &frame) {
    if (!capture.isOpened()) {
        return false;
    }

    if (!capture.read(frame)) {
        return false;
    }

    return !frame.empty();
}
