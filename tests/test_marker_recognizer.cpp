#include "MarkerRecognizer.h"
#include <iostream>

// Same bits as markerBits in MarkerRecognizer.h (that one is private, so we copy it)
static const uchar BITS[4][4] = {{1,0,1,1},{0,1,0,1},{0,0,1,1},{0,0,1,0}};

static cv::Mat makeMarker() {
    int cellPx = 40, n = 6 * cellPx, margin = 2 * cellPx;
    cv::Mat img(n + 2 * margin, n + 2 * margin, CV_8UC1, cv::Scalar(255));
    cv::Mat grid = cv::Mat::zeros(6, 6, CV_8UC1);   // black border
    for (int r = 0; r < 4; r++)
        for (int c = 0; c < 4; c++)
            grid.at<uchar>(r + 1, c + 1) = BITS[r][c] * 255;
    cv::Mat big;
    cv::resize(grid, big, cv::Size(n, n), 0, 0, cv::INTER_NEAREST);
    big.copyTo(img(cv::Rect(margin, margin, n, n)));
    return img;
}

static double recognitionRate(double sigma, int trials, cv::RNG &rng) {
    MarkerRecognizer rec;
    int ok = 0;
    for (int t = 0; t < trials; t++) {
        cv::Mat img = makeMarker();
        if (sigma > 0) {
            cv::Mat noise(img.size(), CV_8SC1);
            rng.fill(noise, cv::RNG::NORMAL, 0, sigma);
            cv::add(img, noise, img, cv::noArray(), CV_8U);
        }
        std::vector<cv::Point> c = {{80,80},{320,80},{320,320},{80,320}};
        MarkerResult r = rec.recognizeMarker(rec.warpMarker(img, c), c);
        if (r.found) ok++;
    }
    return double(ok) / trials;
}

int main() {
    cv::RNG rng(12345);                       // fixed seed = same result every run
    if (recognitionRate(0, 1, rng) < 1.0) {   // no noise: must work
        std::cout << "FAIL: clean marker not recognised\n";
        return 1;
    }
    for (double s : {20.0, 40.0, 60.0, 80.0})
        std::cout << "noise " << s << " -> recognised "
                  << recognitionRate(s, 200, rng) * 100 << "%\n";
    return 0;
}
