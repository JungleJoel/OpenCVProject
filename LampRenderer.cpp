//
// Created by Kevin Taverner on 2026-10-13.
//


#include "LampRenderer.h"
#include <string>

cv::Mat LampRenderer::render(const LampState& lamp) {

    // Create a dark background
    cv::Mat image(300, 400, CV_8UC3, cv::Scalar(25, 25, 25));

    // Determine the brightness of the lamp
    double brightness = lamp.on ? lamp.brightness : 0.0;

    // Calculate the lamp's color
    cv::Scalar lampColor(
        30 + 20 * brightness,
        30 + 195 * brightness,
        30 + 225 * brightness
    );

    // Draw the lamp, negative value for filled circle
    cv::circle(image, cv::Point(200, 160),
               70, lampColor, -1);

    // Draw the lamp outline
    cv::circle(image, cv::Point(200, 160),
               70, cv::Scalar(100, 100, 100), 2);

    // Display on or off status
    std::string status = lamp.on ? "ON" : "OFF";

    cv::putText(image, status, cv::Point(20, 40),
                cv::FONT_HERSHEY_SIMPLEX, 0.8,
                cv::Scalar(255, 255, 255), 2);

    // Display brightness percentage value
    int percentage = static_cast<int>(brightness * 100);

    cv::putText(image,
                "Brightness: " + std::to_string(percentage) + "%",
                cv::Point(20, 75),
                cv::FONT_HERSHEY_SIMPLEX, 0.7,
                cv::Scalar(255, 255, 255), 2);

    return image;
}

