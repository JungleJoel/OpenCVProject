// Written by Joel Seger 29/09
// Stage 5: Line Estimation

#ifndef CAMERA_GRAB_LINEESTIMATION_H
#define CAMERA_GRAB_LINEESTIMATION_H
 
#include <opencv2/opencv.hpp>
#include <vector>

class LineEstimation {
public:
    LineEstimation();

    std::vector<cv::Point> findCorners(const std::vector<cv::Point> &contour);
};
#endif
