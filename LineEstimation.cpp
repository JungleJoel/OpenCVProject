// Writen by Joel Seger 29/09
// Stage 5 and 6

#include "LineEstimation.h"

LineEstimation::LineEstimation() {}

std::vector<cv::Point> LineEstimation::findCorners(const std::vector<cv::Point> &contour) {
    std::vector<cv::Point> corners;

    // Where x+y is at it's minimum
    cv::Point topLeft(contour[0]);

    // Where x+y is at it's maximum
    cv::Point bottomRight(contour[0]);

    // Where x-y is at it's minimum
    cv::Point topRight(contour[0]);
    
    // Where x-y is at it's maximum
    cv::Point bottomLeft(contour[0]);

    for (const auto& pt : contour) {
        if (pt.x + pt.y < topLeft.x + topLeft.y) {
            topLeft = pt;
            continue;
        } else if (pt.x + pt.y > bottomRight.x + bottomRight.y) {
            bottomRight = pt;
            continue;
        } else if (pt.x - pt.y > topRight.x + topRight.y) {
            topRight = pt;
            continue;
        } else if (pt.x - pt.y < bottomLeft.x + bottomLeft.y) {
            bottomLeft = pt;
            continue;
        } else {
            continue;
        }
    }
    
    // Ordered clockwise for easy access
    corners.push_back(topLeft);
    corners.push_back(topRight);
    corners.push_back(bottomRight);
    corners.push_back(bottomLeft);

    return corners;
}

