#include "ContourDetection.h"

ContourDetection::ContourDetection() {}

std::vector<cv::Point> ContourDetection::extractContour(const cv::Mat& binary, const cv::Rect& rect) {
    std::vector<cv::Point> contour;

    int dx[] = {0, 0, 1, -1};
    int dy[] = {1, -1, 0, 0};

    for (int y = rect.y; y < rect.y + rect.height; ++y) {
        for (int x = rect.x; x < rect.x + rect.width; ++x) {
            
            uchar currentVal = binary.at<uchar>(y, x);
            
            if (currentVal > 0) {
                bool isBoundary = false;
                
                for (int i = 0; i < 4; ++i) {
                    int nx = x + dx[i];
                    int ny = y + dy[i];
                    
                    if (nx < 0 || nx >= binary.cols || ny < 0 || ny >= binary.rows) {
                        isBoundary = true;
                        break;
                    }
                    
                    uchar neighborVal = binary.at<uchar>(ny, nx);
                    
                    // If neighbor is background (0) or a different region ID, it's an edge
                    if (neighborVal == 0 || neighborVal != currentVal) {
                        isBoundary = true;
                        break;
                    }
                }
                
                if (isBoundary) {
                    contour.push_back(cv::Point(x, y));
                }
            }
        }
    }
    
    return contour;
}
