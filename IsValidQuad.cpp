// Written by Joel Seger 30/09
// Stage 6

#include "IsValidQuad.h"
#include <iostream>
#include <cmath>

IsValidQuad::IsValidQuad() = default;

bool IsValidQuad::isCustomConvex(const std::vector<cv::Point>& corners) {
    if (corners.size() != 4) return false;
    bool hasPositive = false;
    bool hasNegative = false;

    for (size_t i = 0; i < 4; ++i) {
        cv::Point p1 = corners[i];
        cv::Point p2 = corners[(i + 1) % 4];
        cv::Point p3 = corners[(i + 2) % 4];

        double dx1 = p2.x - p1.x;
        double dy1 = p2.y - p1.y;
        double dx2 = p3.x - p2.x;
        double dy2 = p3.y - p2.y;

        double crossProduct = (dx1 * dy2) - (dy1 * dx2);

        if (crossProduct > 0) hasPositive = true;
        if (crossProduct < 0) hasNegative = true;

        if (hasPositive && hasNegative) return false;
    }
    return true;
}

bool IsValidQuad::hasValidAngles(const std::vector<cv::Point>& corners) {
    if (corners.size() != 4) return false;

    for (size_t i = 0; i < 4; ++i) {
        cv::Point prev = corners[(i + 3) % 4];
        cv::Point curr = corners[i];
        cv::Point next = corners[(i + 1) % 4];

        double v1x = prev.x - curr.x;
        double v1y = prev.y - curr.y;
        double v2x = next.x - curr.x;
        double v2y = next.y - curr.y;

        double dot = (v1x * v2x) + (v1y * v2y);
        double mag1 = std::sqrt(v1x * v1x + v1y * v1y);
        double mag2 = std::sqrt(v2x * v2x + v2y * v2y);

        if (mag1 == 0 || mag2 == 0) return false;

        double cosAngle = std::max(-1.0, std::min(1.0, dot / (mag1 * mag2)));
        double angleDeg = std::acos(cosAngle) * (180.0 / 3.14159265358979323846);

        if (angleDeg < 30.0 || angleDeg > 150.0) return false;
    }
    return true;
}

bool IsValidQuad::isReliableQuadrilateral(const std::vector<cv::Point>& corners, 
                                          const std::vector<std::vector<cv::Point>>& segments, 
                                          const std::vector<Line>& lines) {
    if (corners.size() != 4 || segments.size() != 4 || lines.size() != 4) {
        return false;
    }

    // minimum length
    for (size_t i = 0; i < 4; ++i) {
        double length = cv::norm(corners[i] - corners[(i + 1) % 4]);
        if (length < 20.0) {
            return false; 
        }
    }

    // check if convex
    if (!isCustomConvex(corners)) {
        return false; 
    }

    // check if camera at angle
    if (!hasValidAngles(corners)) {
        return false;
    }

    // check lenght on sides to calc aspect ratio
    double side1Len = cv::norm(corners[0] - corners[1]);
    double side2Len = cv::norm(corners[1] - corners[2]);

    if (side1Len == 0 || side2Len == 0) return false;

    double aspectRatio = side1Len / side2Len;

    if (aspectRatio < 0.7 || aspectRatio > 1.4) {
        return false;
    }
    
    return true;
}

QuadResult IsValidQuad::processAndValidate(const std::vector<cv::Point>& contour, LineEstimation& liner) {
    QuadResult result;
    if (contour.empty()) return result;

    // find corners and segments
    result.corners = liner.findCorners(contour);
    if (result.corners.size() != 4) return result;

    auto segments = liner.makeSegments(result.corners, contour);
    result.lines = liner.findLine(segments);

    // validate against rules
    if (isReliableQuadrilateral(result.corners, segments, result.lines)) {
        result.isValid = true;
    }

    return result;
}
