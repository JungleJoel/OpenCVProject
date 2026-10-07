// Written by Love Skön 09/25, updated 10/07
// Stage 2: Binarisation + Stage 3: Connected-region labelling

#ifndef CAMERA_GRAB_REGIONANALYZER_H
#define CAMERA_GRAB_REGIONANALYZER_H

#include <opencv2/opencv.hpp>
#include <vector>

enum class ThresholdMode {
    Otsu,     // One global threshold
    Adaptive  // Per-pixel threshold, for when lighting is uneven
};

struct AnalyzerParams {
    bool darkMarkers = true;
    ThresholdMode mode = ThresholdMode::Otsu;
    int adaptiveBlockSize = 51;  // bigger than marker part-size
    double adaptiveC = 10.0;

    int minArea = 50;
    int maxArea = 0;  // 0 = no limit
    bool rejectBorderTouching = false;
};

struct Region {
    cv::Rect boundingBox;
    cv::Point2d centroid;
    int area = 0;
    double fillRatio = 0.0;    // area / bounding-box area
    double aspectRatio = 0.0;  // width / height
    bool touchesBorder = false;
};

class RegionAnalyzer {
public:
    RegionAnalyzer();

    // Non-empty, 8-bit, 1/3/4 channels.
    static bool isValidFrame(const cv::Mat &frame);

    // Foreground = 255. Returns false on invalid frame.
    bool binarize(const cv::Mat &frame, cv::Mat &outBinary,
                  const AnalyzerParams &params = AnalyzerParams()) const;

    std::vector<Region> label(const cv::Mat &binary,
                              const AnalyzerParams &params = AnalyzerParams()) const;

    std::vector<Region> analyze(const cv::Mat &frame, cv::Mat &outBinary,
                                const AnalyzerParams &params = AnalyzerParams()) const;

    std::vector<Region> analyze(const cv::Mat &frame, cv::Mat &outBinary, int minArea) const;

    static void drawOverlay(const cv::Mat &frame, const std::vector<Region> &regions,
                            cv::Mat &out);
};

#endif //CAMERA_GRAB_REGIONANALYZER_H
