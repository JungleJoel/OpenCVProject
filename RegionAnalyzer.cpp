// Written by Love Skön 09/25
// Stage 2: Binarisation + Stage 3: Connected-region labelling
//

#include "RegionAnalyzer.h"

RegionAnalyzer::RegionAnalyzer() = default;

void RegionAnalyzer::binarize(const cv::Mat &frame, cv::Mat &outBinary) const {
    cv::Mat gray;
    cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);
    // THRESH_BINARY_INV assumes markers are darker than surroundings.
    // Use THRESH_BINARY if markers are lighter than background.
    cv::threshold(gray, outBinary, 0, 255, cv::THRESH_BINARY_INV | cv::THRESH_OTSU);
}

std::vector<Region> RegionAnalyzer::label(const cv::Mat &binary, int minArea) const {
    cv::Mat labels, stats, centroids;
    int numLabels = cv::connectedComponentsWithStats(binary, labels, stats, centroids, 8, CV_32S);

    std::vector<Region> regions;
    // Label 0 is background
    for (int i = 1; i < numLabels; ++i) {
        int area = stats.at<int>(i, cv::CC_STAT_AREA);
        if (area < minArea) {
            continue;
        }

        Region region;
        region.boundingBox = cv::Rect(
            stats.at<int>(i, cv::CC_STAT_LEFT),
            stats.at<int>(i, cv::CC_STAT_TOP),
            stats.at<int>(i, cv::CC_STAT_WIDTH),
            stats.at<int>(i, cv::CC_STAT_HEIGHT));
        region.centroid = cv::Point2d(centroids.at<double>(i, 0), centroids.at<double>(i, 1));
        region.area = area;

        regions.push_back(region);
    }

    return regions;
}

std::vector<Region> RegionAnalyzer::analyze(const cv::Mat &frame, cv::Mat &outBinary, int minArea) const {
    binarize(frame, outBinary);
    return label(outBinary, minArea);
}