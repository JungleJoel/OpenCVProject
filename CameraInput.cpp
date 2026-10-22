//
// Created by Kevin Taverner on 2026-09-22.
//

#include "CameraInput.h"

#include <sstream>

CameraInput::CameraInput() = default;

bool CameraInput::openCamera(int cameraIndex) {
    capture.release();
    live = true;
    sourceName = "camera " + std::to_string(cameraIndex);
    if (!capture.open(cameraIndex)) {
        lastErrorMessage = "could not open " + sourceName;
        return false;
    }
    lastErrorMessage.clear();
    return true;
}

bool CameraInput::openVideo(const std::string &videoPath) {
    capture.release();
    live = false;
    sourceName = "video \"" + videoPath + "\"";
    if (!capture.open(videoPath)) {
        lastErrorMessage = "could not open " + sourceName;
        return false;
    }
    lastErrorMessage.clear();
    return true;
}

bool CameraInput::isOpen() const {
    return capture.isOpened();
}

bool CameraInput::isLiveSource() const {
    return live;
}

bool CameraInput::rawRead(cv::Mat &frame) {
    return capture.read(frame) && !frame.empty();
}

FrameStatus CameraInput::readFrame(cv::Mat &frame) {
    if (!capture.isOpened()) {
        lastErrorMessage = "no source is open";
        return FrameStatus::NotOpen;
    }

    if (rawRead(frame)) {
        return FrameStatus::Ok;
    }

    if (isLiveSource()) {
        // if a live camera doesn't deliver a frame, the stream tries again
        lastErrorMessage = sourceName + " did not deliver a frame";
        return FrameStatus::ReadFailed;
    }

    // a file that cannot deliver another frame is ended
    capture.release();
    lastErrorMessage = sourceName + " reached end of stream";
    return FrameStatus::EndOfStream;
}

bool CameraInput::getFrame(cv::Mat &frame) {
    for (int attempt = 0; ; ++attempt) {
        FrameStatus status = readFrame(frame);
        if (status == FrameStatus::Ok) {
            return true;
        }
        if (status != FrameStatus::ReadFailed || attempt >= retries) {
            if (status == FrameStatus::ReadFailed) {
                lastErrorMessage = sourceName + " failed " + std::to_string(attempt + 1)
                                   + " reads in a row, giving up";
            }
            return false;
        }
    }
}

int CameraInput::maxRetries() const {
    return retries;
}

void CameraInput::setMaxRetries(int maxRetries) {
    retries = maxRetries < 0 ? 0 : maxRetries;
}

std::string CameraInput::describeSource() const {
    if (!capture.isOpened()) {
        return sourceName.empty() ? "no source" : sourceName + " (not open)";
    }
    std::ostringstream out;
    out << sourceName << ", "
        << static_cast<int>(capture.get(cv::CAP_PROP_FRAME_WIDTH)) << "x"
        << static_cast<int>(capture.get(cv::CAP_PROP_FRAME_HEIGHT)) << " @ "
        << capture.get(cv::CAP_PROP_FPS) << " fps";
    if (!live) {
        out << ", " << static_cast<long>(capture.get(cv::CAP_PROP_FRAME_COUNT)) << " frames";
    }
    return out.str();
}

const std::string &CameraInput::lastError() const {
    return lastErrorMessage;
}
