#pragma once
#include <vector>
#include <opencv2/opencv.hpp>

struct MarkerResult {
    bool found = false;
    int rotation = 0; // quarter turns clockwise (0-3)
    std::vector<cv::Point2f> corners; // reordered so corners[0] is always the marker's own/true top-left
};

class MarkerRecognizer {
public:
    cv::Mat warpMarker(const cv::Mat& grayscaleFrame, const std::vector<cv::Point>& corners) const;
    MarkerResult recognizeMarker(const cv::Mat& warped, const std::vector<cv::Point>& corners) const;

private:
    cv::Mat readBits(const cv::Mat& warped) const;

    static constexpr int warpSize = 60; // Our marker has 6x6 cells. Each can be 10 px, so our warped marker becomes 60x60 px
    static constexpr int cellCount = 6; // cells per side: 1 border cell + 4 bits + 1 border cell
    static constexpr int cellSize = warpSize / cellCount; // 10 px

    // Inner 4x4 bits of ArUco DICT_4X4_1000 ID 0 when upright (1 = white, 0 = black)
    // https://chev.me/arucogen/
    const cv::Mat markerBits = (cv::Mat_<uchar>(4, 4) <<
        1, 0, 1, 1,
        0, 1, 0, 1,
        0, 0, 1, 1,
        0, 0, 1, 0);
};
