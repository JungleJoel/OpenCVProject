// This largely follows this guide from OpenCV:
// https://docs.opencv.org/4.x/d5/dae/tutorial_aruco_detection.html

#include "MarkerRecognizer.h"
#include <algorithm>

cv::Mat MarkerRecognizer::warpMarker(const cv::Mat& grayscaleFrame, const std::vector<cv::Point>& corners) const {
    std::vector<cv::Point2f> markerCorners(corners.begin(), corners.end());
    std::vector<cv::Point2f> warpCorners = { {0, 0}, {warpSize, 0}, {warpSize, warpSize}, {0, warpSize} }; // TL, TR, BR, BL

    cv::Mat homography = cv::getPerspectiveTransform(markerCorners, warpCorners);
    cv::Mat warped;
    cv::warpPerspective(grayscaleFrame, warped, homography, cv::Size(warpSize, warpSize));
    return warped;
}

MarkerResult MarkerRecognizer::recognizeMarker(const cv::Mat& warped, const std::vector<cv::Point>& corners) const {
    MarkerResult result;
    cv::Mat bits = readBits(warped);

    // Early elimination of quad, check if any border bit is non-black
    bool borderIsBlack = cv::countNonZero(bits.row(0)) == 0 && cv::countNonZero(bits.row(5)) == 0
                      && cv::countNonZero(bits.col(0)) == 0 && cv::countNonZero(bits.col(5)) == 0;
    if (!borderIsBlack) {
        return result;
    }

    // Try all 4 rotations of the inner part of marker
    cv::Mat inner = bits(cv::Rect(1, 1, 4, 4)).clone(); // copy the inner 4x4 out of the 6x6 grid
    for (int rotation = 0; rotation < 4; rotation++) {
        bool matches = cv::countNonZero(inner != markerBits) == 0; // no cell differs from our marker
        if (matches) {
            result.found = true;
            result.rotation = rotation;
            result.corners.assign(corners.begin(), corners.end());
            std::rotate(result.corners.begin(), result.corners.begin() + rotation, result.corners.end());
            return result;
        }
        cv::rotate(inner, inner, cv::ROTATE_90_COUNTERCLOCKWISE);
    }
    return result;
}

cv::Mat MarkerRecognizer::readBits(const cv::Mat& warped) const {
    cv::Mat binary;
    cv::threshold(warped, binary, 0, 1, cv::THRESH_BINARY | cv::THRESH_OTSU);

    cv::Mat bits(cellCount, cellCount, CV_8UC1);
    for (int row = 0; row < cellCount; row++) {
        for (int col = 0; col < cellCount; col++) {
            bits.at<uchar>(row, col) = binary.at<uchar>(row * cellSize + cellSize / 2, col * cellSize + cellSize / 2);
        }
    }
    return bits;
}
