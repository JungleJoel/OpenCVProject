// Written by Joel Seger 30/09
// Stage 6

#pragma once
#include <vector>
#include <opencv2/opencv.hpp>
#include "LineEstimation.h" // For the Line struct

struct QuadResult {
    std::vector<cv::Point> corners;
    std::vector<Line> lines;
    bool isValid = false;
};

class IsValidQuad {
public:
    IsValidQuad();

    QuadResult processAndValidate(const std::vector<cv::Point>& contour, LineEstimation& liner);

private:
    bool isReliableQuadrilateral(const std::vector<cv::Point>& corners, 
                                const std::vector<std::vector<cv::Point>>& segments, 
                                const std::vector<Line>& lines);
                                
    bool isCustomConvex(const std::vector<cv::Point>& corners);
    bool hasValidAngles(const std::vector<cv::Point>& corners);
};
