// Written by Love Skön 09/25, updated 10/07
// Stage 2: Binarisation + Stage 3: Connected-region labelling

#include "RegionAnalyzer.h"

namespace {

// Returns false if the frame is unsupported.
bool toGray(const cv::Mat &frame, cv::Mat &gray) {
    if (!RegionAnalyzer::isValidFrame(frame)) {
        return false;
    }
    switch (frame.channels()) {
        case 1:
            gray = frame;
            return true;
        case 3:
            cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);
            return true;
        case 4:
            cv::cvtColor(frame, gray, cv::COLOR_BGRA2GRAY);
            return true;
        default:
            return false;
    }
}

// adaptiveThreshold needs an odd block size >= 3.
int sanitizeBlockSize(int blockSize) {
    if (blockSize < 3) {
        blockSize = 3;
    }
    if (blockSize % 2 == 0) {
        ++blockSize;
    }
    return blockSize;
}

}

RegionAnalyzer::RegionAnalyzer() = default;

bool RegionAnalyzer::isValidFrame(const cv::Mat &frame) {
    if (frame.empty() || frame.depth() != CV_8U) {
        return false;
    }
    const int c = frame.channels();
    return c == 1 || c == 3 || c == 4;
}

bool RegionAnalyzer::binarize(const cv::Mat &frame, cv::Mat &outBinary,
                              const AnalyzerParams &params) const {
    cv::Mat gray;
    if (!toGray(frame, gray)) {
        outBinary.release();
        return false;
    }

    // Dark markers need inverted threshold.
    const int type = params.darkMarkers ? cv::THRESH_BINARY_INV : cv::THRESH_BINARY;

    if (params.mode == ThresholdMode::Adaptive) {
        cv::adaptiveThreshold(gray, outBinary, 255, cv::ADAPTIVE_THRESH_GAUSSIAN_C, type,
                              sanitizeBlockSize(params.adaptiveBlockSize), params.adaptiveC);
    } else {
        cv::threshold(gray, outBinary, 0, 255, type | cv::THRESH_OTSU);
    }
    return true;
}

std::vector<Region> RegionAnalyzer::label(const cv::Mat &binary, const AnalyzerParams &params) const {
    std::vector<Region> regions;
    if (binary.empty() || binary.type() != CV_8UC1) {
        return regions;
    }

    cv::Mat labels, stats, centroids;
    const int numLabels = cv::connectedComponentsWithStats(binary, labels, stats, centroids, 8, CV_32S);

    // Label 0 is background.
    for (int i = 1; i < numLabels; ++i) {
        const int area = stats.at<int>(i, cv::CC_STAT_AREA);
        if (area < params.minArea) {
            continue;
        }
        if (params.maxArea > 0 && area > params.maxArea) {
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

        const cv::Rect &b = region.boundingBox;
        region.fillRatio = static_cast<double>(area) / (static_cast<double>(b.width) * b.height);
        region.aspectRatio = static_cast<double>(b.width) / b.height;
        region.touchesBorder = b.x == 0 || b.y == 0 ||
                               b.x + b.width == binary.cols || b.y + b.height == binary.rows;

        if (params.rejectBorderTouching && region.touchesBorder) {
            continue;
        }

        regions.push_back(region);
    }

    return regions;
}

std::vector<Region> RegionAnalyzer::analyze(const cv::Mat &frame, cv::Mat &outBinary,
                                            const AnalyzerParams &params) const {
    if (!binarize(frame, outBinary, params)) {
        return {};
    }
    return label(outBinary, params);
}

std::vector<Region> RegionAnalyzer::analyze(const cv::Mat &frame, cv::Mat &outBinary, int minArea) const {
    AnalyzerParams params;
    params.minArea = minArea;
    return analyze(frame, outBinary, params);
}

void RegionAnalyzer::drawOverlay(const cv::Mat &frame, const std::vector<Region> &regions,
                                 cv::Mat &out) {
    if (!isValidFrame(frame)) {
        out.release();
        return;
    }

    if (frame.channels() == 1) {
        cv::cvtColor(frame, out, cv::COLOR_GRAY2BGR);
    } else if (frame.channels() == 4) {
        cv::cvtColor(frame, out, cv::COLOR_BGRA2BGR);
    } else {
        out = frame.clone();
    }

    for (const Region &r : regions) {
        cv::rectangle(out, r.boundingBox, cv::Scalar(0, 255, 0), 1);
        cv::circle(out, cv::Point(cvRound(r.centroid.x), cvRound(r.centroid.y)), 3,
                   cv::Scalar(0, 0, 255), cv::FILLED);
        cv::putText(out, std::to_string(r.area), r.boundingBox.tl() + cv::Point(0, -3),
                    cv::FONT_HERSHEY_SIMPLEX, 0.4, cv::Scalar(255, 0, 0), 1);
    }
}
