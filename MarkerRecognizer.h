#pragma once
#include <vector>
#include <opencv2/opencv.hpp>

class MarkerRecognizer {
public:
    cv::Mat warpMarker(const cv::Mat& grayscaleFrame, const std::vector<cv::Point>& corners) const;

private:
    static constexpr int warpSize = 60; // Our marker has 6x6 cells. Each can be 10 px, so our warped marker becomes 60x60 px
};
