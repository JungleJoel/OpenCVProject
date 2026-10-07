// Written by Joel Seger 29/09
// Stage 5: Line Estimation

#pragma once
#include <vector>
#include <opencv2/opencv.hpp>

struct Line {
    double m;
    double b;
};

class LineEstimation {
public:
    LineEstimation();
    std::vector<cv::Point> findCorners(const std::vector<cv::Point>& contour);
    std::vector<std::vector<cv::Point>> makeSegments(std::vector<cv::Point>& corners, const std::vector<cv::Point>& contour);
    std::vector<Line> findLine(const std::vector<std::vector<cv::Point>>& segments);
};
