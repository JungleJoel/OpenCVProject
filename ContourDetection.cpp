// Written by Joel 30/09
// Stage 4

#include "ContourDetection.h"

ContourDetection::ContourDetection() {}

std::vector<cv::Point> ContourDetection::extractContour(const cv::Mat& binary, const cv::Rect& rect) {
    std::vector<cv::Point> contour;

    // scan inwards to find contour 
    for (int x = rect.x; x < rect.x + rect.width; ++x) {
        for (int y = rect.y; y < rect.y + rect.height; ++y) {
            if (binary.at<uchar>(y, x) > 0) {
                contour.push_back(cv::Point(x, y));
                break;
            }
        }
    }

    
    for (int y = rect.y; y < rect.y + rect.height; ++y) {
        for (int x = rect.x + rect.width - 1; x >= rect.x; --x) {
            if (binary.at<uchar>(y, x) > 0) {
                contour.push_back(cv::Point(x, y));
                break;
            }
        }
    }

    
    for (int x = rect.x + rect.width - 1; x >= rect.x; --x) {
        for (int y = rect.y + rect.height - 1; y >= rect.y; --y) {
            if (binary.at<uchar>(y, x) > 0) {
                contour.push_back(cv::Point(x, y));
                break; 
            }
        }
    }

    
    for (int y = rect.y + rect.height - 1; y >= rect.y; --y) {
        for (int x = rect.x; x < rect.x + rect.width; ++x) {
            if (binary.at<uchar>(y, x) > 0) {
                contour.push_back(cv::Point(x, y));
                break;
            }
        }
    }

    return contour;
}
