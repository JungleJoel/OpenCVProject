#include <iostream>
#include <opencv2/opencv.hpp>

#include "CameraInput.h"
int main()
{
    CameraInput camera;

    if (!camera.openCamera(0)) {
        std::cerr << "Could not open the video.\n";
        return 1;
    }

    cv::Mat frame;

    while (camera.getFrame(frame)) {
        cv::imshow("OpenCV Camera Window", frame);

        if (cv::waitKey(20) == 27) {
            break;
        }
   }
return 0;
}
