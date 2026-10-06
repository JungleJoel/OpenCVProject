#include <iostream>
#include <opencv2/opencv.hpp>
#include "CameraInput.h"
#include "RegionAnalyzer.h"
#include "ContourDetection.h"
#include "LineEstimation.h"
#include "IsValidQuad.h"
#include "MarkerRecognizer.h"

int main()
{
    CameraInput camera;
    if (!camera.openCamera(4)) {
        std::cerr << "Could not open the video.\n";
        return 1;
    }

    RegionAnalyzer analyzer;
    ContourDetection detecter;
    LineEstimation liner;
    IsValidQuad validator;
    MarkerRecognizer recognizer;

    cv::Mat frame, binary;
    // Read and display one frame at a time.
    while (camera.getFrame(frame)) {
        cv::imshow("OpenCV video test", frame);

        // Stage 2 + Stage 3
        std::vector<Region> regions = analyzer.analyze(frame, binary, 200);
        cv::imshow("Binarised", binary);

        cv::Mat regionsView = frame.clone();
        for (const auto &region : regions) {
            cv::rectangle(regionsView, region.boundingBox, cv::Scalar(0, 255, 0), 2);
        }
        cv::imshow("Connected regions", regionsView);
        
        //Stage 4, 5 and 6
        std::vector<QuadResult> validQuads;

        for (const auto& reg : regions) {
            std::vector<cv::Point> contour = detecter.extractContour(binary, reg.boundingBox);
            
            // check if contour is to small or big
            double area = cv::contourArea(contour);
            double totalImageArea = static_cast<double>(binary.cols * binary.rows);

            if (area < 200.0 || area > (totalImageArea * 0.75)) {
                continue; 
            }
            
            QuadResult quad = validator.processAndValidate(contour, liner);

            if (quad.isValid) {
                validQuads.push_back(quad); 
            }
        }

        // Stage 7 + 8
        cv::Mat grayscaleFrame;
        cv::cvtColor(frame, grayscaleFrame, cv::COLOR_BGR2GRAY);
        std::vector<cv::Mat> warps;
        MarkerResult marker;
        for (const auto& quad : validQuads) {
            cv::Mat warped = recognizer.warpMarker(grayscaleFrame, quad.corners);
            warps.push_back(warped);
            if (!marker.found) {
                marker = recognizer.recognizeMarker(warped, quad.corners);
            }
        }
        cv::Mat strip = cv::Mat::zeros(60, 60, CV_8UC1); // black square when there are no quads detected
        if (!warps.empty()) {
            cv::hconcat(warps, strip); // all warps side by side
        }
        cv::resize(strip, strip, cv::Size(), 4, 4, cv::INTER_NEAREST); // enlarge
        cv::imshow("Warped", strip);

        for (int i = 0; i < validQuads.size(); i++) {
            // draw lines between corners
                cv::line(frame, validQuads[i].corners[0], validQuads[i].corners[1], cv::Scalar(0, 255, 255), 2);
                cv::line(frame, validQuads[i].corners[1], validQuads[i].corners[2], cv::Scalar(0, 255, 255), 2);
                cv::line(frame, validQuads[i].corners[2], validQuads[i].corners[3], cv::Scalar(0, 255, 255), 2);
                cv::line(frame, validQuads[i].corners[3], validQuads[i].corners[0], cv::Scalar(0, 255, 255), 2);

                // draw corners
                cv::circle(frame, validQuads[i].corners[0], 4, cv::Scalar(0, 0, 255), -1);
                cv::circle(frame, validQuads[i].corners[1], 4, cv::Scalar(0, 255, 255), -1);
                cv::circle(frame, validQuads[i].corners[2], 4, cv::Scalar(255, 0, 0), -1);
                cv::circle(frame, validQuads[i].corners[3], 4, cv::Scalar(255, 0, 255), -1);

                std::string indicatorText = "Quad" + std::to_string(i);

                cv::putText(frame, indicatorText, validQuads[i].corners[0] - cv::Point(0, 10), cv::FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(0, 255, 0), 2);
        }

        // Stage 8: tag the marker in the view
        if (marker.found) {
            cv::circle(frame, marker.corners[0], 10, cv::Scalar(0, 0, 255), 2);
            cv::putText(frame, "MARKER top left", marker.corners[0] + cv::Point2f(12, 20), cv::FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(0, 0, 255), 2);
        }

        cv::imshow("Detected Quadrilateral", frame);

        // esc closes program.
        if (cv::waitKey(20) == 27) {
            break;
        }
    }
    return 0;
}
