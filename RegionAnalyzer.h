// Written by Love Skön 09/25
// Stage 2: Binarisation + Stage 3: Connected-region labelling

#ifndef CAMERA_GRAB_REGIONANALYZER_H
#define CAMERA_GRAB_REGIONANALYZER_H

#include <opencv2/opencv.hpp>
#include <vector>

struct Region {
    cv::Rect boundingBox;
    cv::Point2d centroid;
    int area;
};

class RegionAnalyzer {
public:
    RegionAnalyzer();

    void binarize(const cv::Mat &frame, cv::Mat &outBinary) const;

    std::vector<Region> label(const cv::Mat &binary, int minArea = 50) const;

    std::vector<Region> analyze(const cv::Mat &frame, cv::Mat &outBinary, int minArea = 50) const;
};

#endif //CAMERA_GRAB_REGIONANALYZER_H