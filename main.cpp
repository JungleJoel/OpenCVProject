#include <iostream>
#include <opencv2/opencv.hpp>
#include "CameraInput.h"
#include "RegionAnalyzer.h"

int main()
{
    CameraInput camera;
    if (!camera.openCamera(0)) {
        std::cerr << "Could not open the video.\n";
        return 1;
    }

    RegionAnalyzer analyzer;

    cv::Mat frame, binary;
    // Read and display one frame at a time.
    while (camera.getFrame(frame)) {
        cv::imshow("OpenCV video test", frame);

        // Stage 2 + Stage 3
        std::vector<Region> regions = analyzer.analyze(frame, binary);
        cv::imshow("Binarised", binary);

        cv::Mat regionsView = frame.clone();
        for (const auto &region : regions) {
            cv::rectangle(regionsView, region.boundingBox, cv::Scalar(0, 255, 0), 2);
        }
        cv::imshow("Connected regions", regionsView);

        // esc closes program.
        if (cv::waitKey(20) == 27) {
            break;
        }
    }
    return 0;
}