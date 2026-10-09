// Written by Love Skön 10/07

#include "RegionAnalyzer.h"

#include <cmath>
#include <functional>
#include <iostream>
#include <string>
#include <vector>

static int g_failures = 0;
static std::vector<std::pair<std::string, std::function<void()>>> &registry() {
    static std::vector<std::pair<std::string, std::function<void()>>> r;
    return r;
}
struct Registrar {
    Registrar(const std::string &name, std::function<void()> fn) { registry().emplace_back(name, std::move(fn)); }
};

#define TEST(name)                                         \
    static void name();                                    \
    static Registrar registrar_##name(#name, name);        \
    static void name()

#define CHECK(cond)                                                                       \
    do {                                                                                  \
        if (!(cond)) {                                                                    \
            std::cerr << "  FAILED: " #cond "  (" << __FILE__ << ":" << __LINE__ << ")\n"; \
            ++g_failures;                                                                 \
        }                                                                                 \
    } while (0)

#define CHECK_NEAR(a, b, tol) CHECK(std::abs((a) - (b)) <= (tol))

static cv::Mat whiteCanvas(int w = 300, int h = 200) {
    return cv::Mat(h, w, CV_8UC3, cv::Scalar(255, 255, 255));
}

static void blackBox(cv::Mat &img, cv::Rect r) {
    cv::rectangle(img, r, cv::Scalar(0, 0, 0), cv::FILLED);
}

// Returns the region whose bounding box equals r exactly, or nullptr.
static const Region *findRegion(const std::vector<Region> &regions, cv::Rect r) {
    for (const Region &reg : regions) {
        if (reg.boundingBox == r) {
            return &reg;
        }
    }
    return nullptr;
}

TEST(detects_two_rectangles_with_correct_stats) {
    cv::Mat img = whiteCanvas();
    blackBox(img, {20, 20, 50, 40});    // area 2000
    blackBox(img, {150, 100, 30, 30});  // area 900

    RegionAnalyzer ra;
    cv::Mat bin;
    auto regions = ra.analyze(img, bin);

    CHECK(regions.size() == 2);
    CHECK(bin.type() == CV_8UC1);
    CHECK(bin.size() == img.size());

    const Region *a = findRegion(regions, {20, 20, 50, 40});
    const Region *b = findRegion(regions, {150, 100, 30, 30});
    CHECK(a != nullptr);
    CHECK(b != nullptr);
    if (a) {
        CHECK(a->area == 2000);
        CHECK_NEAR(a->centroid.x, 44.5, 1e-6);  // x + (w-1)/2
        CHECK_NEAR(a->centroid.y, 39.5, 1e-6);
    }
    if (b) {
        CHECK(b->area == 900);
    }
}

TEST(blank_white_image_gives_no_regions) {
    cv::Mat blank = whiteCanvas(100, 100);
    RegionAnalyzer ra;
    cv::Mat bin;
    CHECK(ra.analyze(blank, bin).empty());
}

TEST(minArea_rejects_small_specks) {
    cv::Mat img = whiteCanvas();
    blackBox(img, {20, 20, 50, 40});
    blackBox(img, {250, 150, 5, 5});  // area 25

    RegionAnalyzer ra;
    cv::Mat bin;
    AnalyzerParams p;
    p.minArea = 50;
    CHECK(ra.analyze(img, bin, p).size() == 1);

    p.minArea = 10;  // speck now allowed
    CHECK(ra.analyze(img, bin, p).size() == 2);
}

TEST(minArea_int_overload_matches_params) {
    cv::Mat img = whiteCanvas();
    blackBox(img, {20, 20, 50, 40});
    blackBox(img, {250, 150, 5, 5});  // area 25

    RegionAnalyzer ra;
    cv::Mat bin;
    CHECK(ra.analyze(img, bin, 50).size() == 1);
    CHECK(ra.analyze(img, bin, 10).size() == 2);
}

TEST(maxArea_rejects_large_regions) {
    cv::Mat img = whiteCanvas();
    blackBox(img, {20, 20, 50, 40});    // 2000
    blackBox(img, {150, 100, 30, 30});  // 900

    RegionAnalyzer ra;
    cv::Mat bin;
    AnalyzerParams p;
    p.maxArea = 1000;
    auto regions = ra.analyze(img, bin, p);
    CHECK(regions.size() == 1);
    if (!regions.empty()) {
        CHECK(regions[0].area == 900);
    }
}

TEST(border_touching_regions_flagged_and_optionally_rejected) {
    cv::Mat img = whiteCanvas();
    blackBox(img, {0, 50, 40, 40});    // touches left edge
    blackBox(img, {150, 100, 30, 30});  // interior

    RegionAnalyzer ra;
    cv::Mat bin;
    AnalyzerParams p;
    auto regions = ra.analyze(img, bin, p);
    CHECK(regions.size() == 2);
    const Region *edge = findRegion(regions, {0, 50, 40, 40});
    const Region *inner = findRegion(regions, {150, 100, 30, 30});
    CHECK(edge && edge->touchesBorder);
    CHECK(inner && !inner->touchesBorder);

    p.rejectBorderTouching = true;
    regions = ra.analyze(img, bin, p);
    CHECK(regions.size() == 1);
}

TEST(fillRatio_and_aspectRatio) {
    cv::Mat img = whiteCanvas();
    blackBox(img, {20, 20, 50, 40});  // solid
    // Hollow frame: 60x60 outline, 4px thick
    cv::rectangle(img, cv::Rect(150, 100, 60, 60), cv::Scalar(0, 0, 0), 4);

    RegionAnalyzer ra;
    cv::Mat bin;
    auto regions = ra.analyze(img, bin);
    CHECK(regions.size() == 2);

    const Region *solid = findRegion(regions, {20, 20, 50, 40});
    CHECK(solid != nullptr);
    if (solid) {
        CHECK_NEAR(solid->fillRatio, 1.0, 1e-9);
        CHECK_NEAR(solid->aspectRatio, 1.25, 1e-9);
    }
    for (const Region &r : regions) {
        if (r.boundingBox.x == 150) {
            CHECK(r.fillRatio < 0.5);  // hollow frame is mostly empty
        }
    }
}

TEST(darkMarkers_false_detects_light_on_dark) {
    cv::Mat img(200, 300, CV_8UC3, cv::Scalar(0, 0, 0));
    cv::rectangle(img, cv::Rect(20, 20, 50, 40), cv::Scalar(255, 255, 255), cv::FILLED);

    RegionAnalyzer ra;
    cv::Mat bin;
    AnalyzerParams p;
    p.darkMarkers = false;
    auto regions = ra.analyze(img, bin, p);
    CHECK(regions.size() == 1);
    if (!regions.empty()) {
        CHECK(regions[0].area == 2000);
    }
}

TEST(empty_frame_does_not_crash) {
    RegionAnalyzer ra;
    cv::Mat empty, bin;
    CHECK(ra.analyze(empty, bin).empty());
    CHECK(bin.empty());
    CHECK(!ra.binarize(empty, bin));
}

TEST(unsupported_formats_are_rejected) {
    RegionAnalyzer ra;
    cv::Mat bin;
    cv::Mat twoChannel(50, 50, CV_8UC2, cv::Scalar(0, 0));
    cv::Mat floatImg(50, 50, CV_32FC3, cv::Scalar(0, 0, 0));
    CHECK(!RegionAnalyzer::isValidFrame(twoChannel));
    CHECK(!RegionAnalyzer::isValidFrame(floatImg));
    CHECK(ra.analyze(twoChannel, bin).empty());
    CHECK(ra.analyze(floatImg, bin).empty());
}

TEST(label_rejects_wrong_binary_type) {
    RegionAnalyzer ra;
    cv::Mat colour(50, 50, CV_8UC3, cv::Scalar(0, 0, 0));
    CHECK(ra.label(colour).empty());
    CHECK(ra.label(cv::Mat()).empty());
}

TEST(grayscale_and_bgra_inputs_are_accepted) {
    cv::Mat bgr = whiteCanvas();
    blackBox(bgr, {20, 20, 50, 40});

    cv::Mat gray, bgra, bin;
    cv::cvtColor(bgr, gray, cv::COLOR_BGR2GRAY);
    cv::cvtColor(bgr, bgra, cv::COLOR_BGR2BGRA);

    RegionAnalyzer ra;
    CHECK(ra.analyze(gray, bin).size() == 1);
    CHECK(ra.analyze(bgra, bin).size() == 1);
}

TEST(adaptive_mode_handles_lighting_gradient) {
    // Background brightens left to right (90 -> 230); two darker squares sit on it.
    cv::Mat gray(200, 300, CV_8UC1);
    for (int y = 0; y < gray.rows; ++y) {
        for (int x = 0; x < gray.cols; ++x) {
            gray.at<uchar>(y, x) = static_cast<uchar>(90 + (140 * x) / (gray.cols - 1));
        }
    }
    auto darken = [&](cv::Rect r) {
        cv::Mat roi = gray(r);
        roi -= 70;  // clearly darker than the local background
    };
    darken({20, 80, 20, 20});
    darken({260, 80, 20, 20});

    RegionAnalyzer ra;
    cv::Mat bin;
    AnalyzerParams p;
    p.mode = ThresholdMode::Adaptive;
    p.adaptiveBlockSize = 51;
    p.adaptiveC = 10.0;
    auto regions = ra.analyze(gray, bin, p);
    CHECK(regions.size() == 2);
}

TEST(adaptive_block_size_is_sanitised) {
    // Even and too-small block sizes must not make OpenCV throw.
    cv::Mat img = whiteCanvas();
    blackBox(img, {20, 20, 20, 20});
    RegionAnalyzer ra;
    cv::Mat bin;
    AnalyzerParams p;
    p.mode = ThresholdMode::Adaptive;
    p.adaptiveBlockSize = 50;
    ra.analyze(img, bin, p);
    CHECK(!bin.empty());
    p.adaptiveBlockSize = 1;
    ra.analyze(img, bin, p);
    CHECK(!bin.empty());
}

static void saveDebugImages(const std::string &dir) {
    cv::Mat img = whiteCanvas(400, 300);
    blackBox(img, {30, 30, 80, 60});
    blackBox(img, {200, 120, 50, 50});
    cv::rectangle(img, cv::Rect(300, 40, 70, 70), cv::Scalar(0, 0, 0), 5);
    blackBox(img, {350, 250, 6, 6});  // speck, filtered out

    RegionAnalyzer ra;
    cv::Mat bin, overlay;
    auto regions = ra.analyze(img, bin);
    RegionAnalyzer::drawOverlay(img, regions, overlay);
    cv::imwrite(dir + "/synthetic_input.png", img);
    cv::imwrite(dir + "/synthetic_binary.png", bin);
    cv::imwrite(dir + "/synthetic_overlay.png", overlay);
    std::cout << "Saved debug images to " << dir << " (" << regions.size() << " regions)\n";
}

int main(int argc, char **argv) {
    int ran = 0;
    for (auto &t : registry()) {
        const int before = g_failures;
        try {
            t.second();
        } catch (const std::exception &e) {
            std::cerr << "  EXCEPTION: " << e.what() << "\n";
            ++g_failures;
        }
        std::cout << (g_failures == before ? "[ OK ] " : "[FAIL] ") << t.first << "\n";
        ++ran;
    }
    if (argc > 1) {
        saveDebugImages(argv[1]);
    }
    std::cout << ran << " tests, " << g_failures << " failed check(s)\n";
    return g_failures == 0 ? 0 : 1;
}
