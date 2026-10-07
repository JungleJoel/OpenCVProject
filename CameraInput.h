//
// Created by Kevin Taverner on 2026-09-22.
//

#ifndef CAMERA_GRAB_CAMERAINPUT_H
#define CAMERA_GRAB_CAMERAINPUT_H

#include <opencv2/opencv.hpp>
#include <string>

class CameraInput {
public:
    CameraInput();

    bool openCamera(int cameraIndex);
    bool openVideo(const std::string& videoPath);

    bool isOpen() const;
    bool getFrame(cv::Mat& frame);

private:
    cv::VideoCapture capture;
};

#endif //CAMERA_GRAB_CAMERAINPUT_H
