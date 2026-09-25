#include <iostream>
#include <opencv2/opencv.hpp>
#include "CameraInput.h"
#include "RegionAnalyzer.h"
#include "ContourDetection.h"

int main()
{
    CameraInput camera;
    if (!camera.openCamera(0)) {
        std::cerr << "Could not open the video.\n";
        return 1;
    }

    RegionAnalyzer analyzer;
    ContourDetection detecter;

    cv::Mat frame, binary;
    // Read and display one frame at a time.
    while (camera.getFrame(frame)) {
        cv::imshow("OpenCV video test", frame);

        // Stage 2 + Stage 3
        std::vector<Region> regions = analyzer.analyze(frame, binary, 200);
        cv::imshow("Binarised", binary);

        cv::Mat regionsView = frame.clone();
        for (const auto &region : regions) {
            cv::rectangle(regionsView, region.boundingBox, cv::Scalar(0, 255, 0), 2);
        }
        cv::imshow("Connected regions", regionsView);

        // Stage 4?
        for (const auto& reg : regions) {
    
            // Get the contour
            std::vector<cv::Point> contour = detecter.extractContour(binary, reg.boundingBox);
            
            // Paint each pixel in the contour green
            for (const auto& pt : contour) {
                if (pt.y >= 0 && pt.y < frame.rows && pt.x >= 0 && pt.x < frame.cols) {
                    frame.at<cv::Vec3b>(pt.y, pt.x) = cv::Vec3b(0, 255, 0);
                }
            }
        }
        // displays the green pixels on top of normal frame.
        cv::imshow("Detected Contours", frame);

        // esc closes program.
        if (cv::waitKey(20) == 27) {
            break;
        }
    }
    return 0;
}
