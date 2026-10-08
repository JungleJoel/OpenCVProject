#include <iostream>
#include <opencv2/opencv.hpp>
#include "CameraInput.h"
#include "RegionAnalyzer.h"
#include "ContourDetection.h"
#include "LineEstimation.h"
#include "IsValidQuad.h"

int main()
{
    CameraInput camera;
    if (!camera.openCamera(0)) {
        std::cerr << "Could not open the video.\n";
        return 1;
    }

    RegionAnalyzer analyzer;
    ContourDetection detecter;
    LineEstimation liner;
    IsValidQuad validator;

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

        cv::Mat displayImage = frame.clone();

        for (const auto& reg : regions) {
            std::vector<cv::Point> contour = detecter.extractContour(binary, reg.boundingBox);
            
            // check if contour is to small or big
            double area = cv::contourArea(contour);
            double totalImageArea = static_cast<double>(binary.cols * binary.rows);

            if (area < 200.0 || area > (totalImageArea * 0.75)) {
                continue; 
            }

            for (const auto& pt : contour) {
                if (pt.y >= 0 && pt.y < displayImage.rows && pt.x >= 0 && pt.x < displayImage.cols) {
                    displayImage.at<cv::Vec3b>(pt.y, pt.x) = cv::Vec3b(0, 255, 0); // Bright Green
                }
            }
            
            QuadResult quad = validator.processAndValidate(contour, liner);

            if (quad.isValid) {
                validQuads.push_back(quad); 
            }
        }
        
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
        
        cv::imshow("Detected Contour", displayImage);
        cv::imshow("Detected Quadrilateral", frame);

        // esc closes program.
        if (cv::waitKey(20) == 27) {
            break;
        }
    }
    return 0;
}
