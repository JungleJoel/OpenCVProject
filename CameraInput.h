//
// Created by Kevin Taverner on 2026-09-22.
//

#ifndef CAMERA_GRAB_CAMERAINPUT_H
#define CAMERA_GRAB_CAMERAINPUT_H

#include <opencv2/opencv.hpp>
#include <opencv2/videoio.hpp>
#include <string>

// Result of one attempt to read a frame.
enum class FrameStatus {
    Ok,           
    NotOpen,      
    EndOfStream,  
    ReadFailed    
                  
};

class CameraInput {
public:
    CameraInput();
    virtual ~CameraInput() = default;

    bool openCamera(int cameraIndex);
    bool openVideo(const std::string& videoPath);

    bool isOpen() const;
    virtual bool isLiveSource() const;

    FrameStatus readFrame(cv::Mat& frame);

    bool getFrame(cv::Mat& frame);

    int maxRetries() const;
    void setMaxRetries(int retries);

    // Human readable description of the open source, e.g. "camera 0, 640x480 @ 30 fps".
    std::string describeSource() const;
    // Explains the most recent failure of open*/readFrame/getFrame.
    const std::string& lastError() const;

protected:
    // Single seam around VideoCapture::read so tests can inject failures.
    virtual bool rawRead(cv::Mat& frame);

private:
    cv::VideoCapture capture;
    bool live = false;
    std::string sourceName;
    int retries = 5;
    std::string lastErrorMessage;
};

#endif //CAMERA_GRAB_CAMERAINPUT_H
