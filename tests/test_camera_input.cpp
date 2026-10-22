// Written by Love Skön 10/22

#include "CameraInput.h"

#include <opencv2/videoio.hpp>

#include <filesystem>
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

static const cv::Size kFrameSize(64, 48);

// Writes a short MJPG video to the temp directory and returns its path.
static std::string makeVideo(int frames) {
    std::string path = (std::filesystem::temp_directory_path() / "camera_input_test.avi").string();
    cv::VideoWriter w(path, cv::VideoWriter::fourcc('M', 'J', 'P', 'G'), 10.0, kFrameSize);
    for (int i = 0; i < frames; ++i) {
        cv::Mat f(kFrameSize, CV_8UC3, cv::Scalar(i * 40, 0, 0));
        w.write(f);
    }
    return path;
}

// A video file pretending to be a live camera whose reads fail now and then.
// Lets the test reproduce a dropped frame deterministically.
class FlakyCamera : public CameraInput {
public:
    explicit FlakyCamera(int failEvery) : failEvery(failEvery) {}
    bool isLiveSource() const override { return true; }
    int reads = 0;
    int failures = 0;

protected:
    bool rawRead(cv::Mat &frame) override {
        ++reads;
        if (failEvery > 0 && reads % failEvery == 0) {
            ++failures;
            frame.release();
            return false;
        }
        return CameraInput::rawRead(frame);
    }

private:
    int failEvery;
};

// A camera that never delivers anything.
class DeadCamera : public CameraInput {
public:
    bool isLiveSource() const override { return true; }
    int reads = 0;

protected:
    bool rawRead(cv::Mat &) override {
        ++reads;
        return false;
    }
};

TEST(read_before_open_reports_not_open) {
    CameraInput c;
    cv::Mat f;
    CHECK(!c.isOpen());
    CHECK(c.readFrame(f) == FrameStatus::NotOpen);
    CHECK(!c.getFrame(f));
    CHECK(!c.lastError().empty());
}

TEST(open_missing_video_fails_with_message) {
    CameraInput c;
    CHECK(!c.openVideo("/nonexistent/video.avi"));
    CHECK(!c.isOpen());
    CHECK(c.lastError().find("nonexistent") != std::string::npos);
}

TEST(video_frames_are_delivered_with_expected_format) {
    std::string path = makeVideo(5);
    CameraInput c;
    CHECK(c.openVideo(path));
    CHECK(c.isOpen());
    CHECK(!c.isLiveSource());
    std::cout << "  source: " << c.describeSource() << "\n";

    int n = 0;
    cv::Mat f;
    while (c.getFrame(f)) {
        CHECK(f.size() == kFrameSize);
        CHECK(f.type() == CV_8UC3);
        ++n;
    }
    CHECK(n == 5);
}

TEST(end_of_video_is_reported_and_closes_the_source) {
    std::string path = makeVideo(3);
    CameraInput c;
    CHECK(c.openVideo(path));

    cv::Mat f;
    for (int i = 0; i < 3; ++i) {
        CHECK(c.readFrame(f) == FrameStatus::Ok);
    }
    CHECK(c.readFrame(f) == FrameStatus::EndOfStream);
    CHECK(!c.isOpen());
    CHECK(c.readFrame(f) == FrameStatus::NotOpen);
    CHECK(c.lastError().find("no source is open") != std::string::npos);
}

TEST(dropped_frame_on_live_source_is_reported_not_fatal) {
    std::string path = makeVideo(4);
    FlakyCamera c(/*failEvery=*/2);  // every second read fails
    CHECK(c.openVideo(path));

    cv::Mat f;
    CHECK(c.readFrame(f) == FrameStatus::Ok);
    CHECK(c.readFrame(f) == FrameStatus::ReadFailed);
    CHECK(c.isOpen());  // still open after the dropped frame
    CHECK(c.readFrame(f) == FrameStatus::Ok);
}

TEST(get_frame_retries_through_dropped_frames) {
    std::string path = makeVideo(5);
    FlakyCamera c(/*failEvery=*/3);  // reads 3, 6, 9, ... fail
    CHECK(c.openVideo(path));

    int delivered = 0;
    cv::Mat f;
    while (delivered < 5 && c.getFrame(f)) {
        ++delivered;
    }
    std::cout << "  delivered " << delivered << " frames over " << c.reads
              << " reads with " << c.failures << " dropped\n";
    CHECK(delivered == 5);
    CHECK(c.failures >= 2); 
}

TEST(get_frame_gives_up_after_max_retries) {
    std::string path = makeVideo(2);
    DeadCamera c;
    c.setMaxRetries(3);
    CHECK(c.openVideo(path));

    cv::Mat f;
    CHECK(!c.getFrame(f));
    CHECK(c.reads == 4);  // 1 attempt + 3 retries
    CHECK(c.isOpen()); 
    std::cout << "  lastError: " << c.lastError() << "\n";
    CHECK(c.lastError().find("giving up") != std::string::npos);
}

int main() {
    for (auto &[name, fn] : registry()) {
        int before = g_failures;
        std::cout << "[ RUN  ] " << name << "\n";
        fn();
        std::cout << (g_failures == before ? "[  OK  ] " : "[ FAIL ] ") << name << "\n";
    }
    std::cout << (g_failures == 0 ? "ALL PASSED" : std::to_string(g_failures) + " CHECK(S) FAILED") << "\n";
    return g_failures == 0 ? 0 : 1;
}
