#include <iostream>
#include <opencv2/opencv.hpp>
#include "CameraInput.h"
#include "RegionAnalyzer.h"
#include "ContourDetection.h"
#include "LineEstimation.h"
#include "IsValidQuad.h"
#include "MarkerRecognizer.h"
#include "PoseEstimator.h"
#include "InteractionMapping.h"
#include "LampRenderer.h"

// Usage: camera_grab [camera index | video path]   (default: camera 0)
static bool openSource(CameraInput &camera, int argc, char **argv) {
    if (argc < 2) {
        return camera.openCamera(0);
    }
    std::string arg = argv[1];
    bool isIndex = !arg.empty() && arg.find_first_not_of("0123456789") == std::string::npos;
    return isIndex ? camera.openCamera(std::stoi(arg)) : camera.openVideo(arg);
}

int main(int argc, char **argv) {
    CameraInput camera;
    if (!openSource(camera, argc, argv)) {
        std::cerr << "Error: " << camera.lastError() << "\n";
        return 1;
    }
    std::cout << "Source: " << camera.describeSource() << "\n";

    RegionAnalyzer analyzer;
    ContourDetection detecter;
    LineEstimation liner;
    IsValidQuad validator;
    MarkerRecognizer recognizer;
    PoseEstimator poseEstimator;
    InteractionMapping interactionMapping;
    LampRenderer lampRenderer;

    cv::Mat frame, binary;
    bool debugNextFrame = false;
    // Read and display one frame at a time. A dropped camera frame is logged
    // and skipped; only end of stream or a dead source ends the loop.
    while (true) {
        FrameStatus status = camera.readFrame(frame);
        if (status == FrameStatus::ReadFailed) {
            std::cerr << "Warning: " << camera.lastError() << ", retrying\n";
            if (cv::waitKey(20) == 27) {
                break;
            }
            continue;
        }
        if (status != FrameStatus::Ok) {
            std::cout << "Stopping: " << camera.lastError() << "\n";
            break;
        }
        cv::imshow("OpenCV video test", frame);

        //debug line
        bool debug = debugNextFrame;
        debugNextFrame = false;

        // Stage 2 + Stage 3
        std::vector<Region> regions = analyzer.analyze(frame, binary, 200);
        cv::imshow("Binarised", binary);

        //debug output if its within 2, 3
        if (debug) {
            std::cout << "\n--- DEBUG FRAME ---\n";
            std::cout << "Regions detected: " << regions.size() << "\n";
        }

        cv::Mat regionsView = frame.clone();
        for (const auto &region: regions) {
            cv::rectangle(regionsView, region.boundingBox, cv::Scalar(0, 255, 0), 2);
        }
        cv::imshow("Connected regions", regionsView);

        //Stage 4, 5 and 6
        std::vector<QuadResult> validQuads;

        cv::Mat displayImage = frame.clone();

        for (const auto &reg: regions) {
            std::vector<cv::Point> contour = detecter.extractContour(binary, reg.boundingBox);

            // check if contour is to small or big
            double area = cv::contourArea(contour);
            double totalImageArea = static_cast<double>(binary.cols * binary.rows);

            // debug output if its within stage 4, 5, 6
            if (debug) {
                std::cout << "Region: " << reg.boundingBox
                          << " | Contour points: " << contour.size()
                          << " | Area: " << area << "\n";
            }

            if (area < 200.0 || area > (totalImageArea * 0.75)) {
                continue;
            }

            for (const auto &pt: contour) {
                if (pt.y >= 0 && pt.y < displayImage.rows && pt.x >= 0 && pt.x < displayImage.cols) {
                    displayImage.at<cv::Vec3b>(pt.y, pt.x) = cv::Vec3b(0, 255, 0); // Bright Green
                }
            }

            QuadResult quad = validator.processAndValidate(contour, liner);

            // debug output if the problem is within 4, 5, 6
            if (debug) {
                std::cout << "Quad valid: " << std::boolalpha << quad.isValid << "\n";

                for (size_t i = 0; i < quad.corners.size(); i++) {
                    std::cout << "  Corner " << i << ": "
                              << quad.corners[i] << "\n";
                }

                for (size_t i = 0; i < quad.corners.size(); i++) {
                    for (size_t j = i + 1; j < quad.corners.size(); j++) {
                        if (quad.corners[i] == quad.corners[j]) {
                            std::cout << "  WARNING: Duplicate corners "
                                      << i << " and " << j << "\n";
                        }
                    }
                }
            }

            if (quad.isValid) {
                validQuads.push_back(quad);
            }
        }

        // Stage 7 + 8
        cv::Mat grayscaleFrame;
        cv::cvtColor(frame, grayscaleFrame, cv::COLOR_BGR2GRAY);
        std::vector<cv::Mat> warps;
        MarkerResult marker;
        for (const auto &quad: validQuads) {
            cv::Mat warped = recognizer.warpMarker(grayscaleFrame, quad.corners);
            warps.push_back(warped);
            if (!marker.found) {
                marker = recognizer.recognizeMarker(warped, quad.corners);
            }
        }

        //debug output
        if (debug) {
            std::cout << "Valid quadrilaterals: " << validQuads.size() << "\n";
            std::cout << "Marker recognized: "
                      << std::boolalpha << marker.found << "\n";
            std::cout << "END DEBUG\n";
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

            cv::putText(frame, indicatorText, validQuads[i].corners[0] - cv::Point(0, 10), cv::FONT_HERSHEY_SIMPLEX,
                        0.6, cv::Scalar(0, 255, 0), 2);
        }

        // Stage 8: tag the marker in the view
        if (marker.found) {
            cv::circle(frame, marker.corners[0], 10, cv::Scalar(0, 0, 255), 2);
            cv::putText(frame, "MARKER top left", marker.corners[0] + cv::Point2f(12, 20), cv::FONT_HERSHEY_SIMPLEX,
                        0.6, cv::Scalar(0, 0, 255), 2);
        }

        // Stage 9: marker pose and angle text
        PoseResult pose;
        if (marker.found) {
            pose = poseEstimator.estimatePose(marker.corners, frame.size());
            poseEstimator.drawAxes(frame, pose);
            std::string poseText = "angle " + std::to_string(static_cast<int>(pose.angle)) + " deg, distance " +
                                   std::to_string(static_cast<int>(pose.tvec[2])) + " mm";
            cv::putText(frame, poseText, cv::Point(10, 30), cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(0, 0, 255), 2);
        }

        // Stage 10: Convert marker angle to lamp brightness
        LampState lamp = interactionMapping.update(
            marker.found,
            pose.angle
        );

        // Stage 11: Display virtual lamp
        cv::Mat lampImage = lampRenderer.render(lamp);
        cv::imshow("Lamp", lampImage);

        cv::imshow("Detected Contour", displayImage);
        cv::imshow("Detected Quadrilateral", frame);

        // debug line for taking a single frame when 'd' is pressed
        if (cv::waitKey(20) == 'd') {
            debugNextFrame = true;
        }

        // esc closes program.
        if (cv::waitKey(20) == 27) {
            break;
        }
    }
    return 0;
}
