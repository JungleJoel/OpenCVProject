#include "MarkerRecognizer.h"

cv::Mat MarkerRecognizer::warpMarker(const cv::Mat& grayscaleFrame, const std::vector<cv::Point>& corners) const {
    std::vector<cv::Point2f> markerCorners(corners.begin(), corners.end());
    std::vector<cv::Point2f> warpCorners = { {0, 0}, {warpSize, 0}, {warpSize, warpSize}, {0, warpSize} }; // TL, TR, BR, BL

    cv::Mat homography = cv::getPerspectiveTransform(markerCorners, warpCorners);
    cv::Mat warped;
    cv::warpPerspective(grayscaleFrame, warped, homography, cv::Size(warpSize, warpSize));
    return warped;
}
