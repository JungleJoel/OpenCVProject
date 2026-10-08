// Written by Joel Seger 29/09
// Stage 4: Contour Detection

#ifndef CAMERA_GRAB_CONTOURDETECTOR_H
#define CAMERA_GRAB_CONTOURDETECTOR_H

#include <opencv2/opencv.hpp>
#include <vector>

class ContourDetection {
public:
    ContourDetection();

    std::vector<cv::Point> extractContour(const cv::Mat& binary, const cv::Rect& rect);
};

#endif
