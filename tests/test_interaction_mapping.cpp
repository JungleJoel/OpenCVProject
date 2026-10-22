// Written by Love Skön 10/22

#include "InteractionMapping.h"

#include <cmath>
#include <functional>
#include <iostream>
#include <limits>
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

static double brightnessAt(double angle) {
    InteractionMapping m;
    return m.update(true, angle).brightness;
}

TEST(marker_not_found_turns_lamp_off) {
    InteractionMapping m;
    LampState s = m.update(false, 45.0);
    CHECK(s.on == false);
    CHECK(s.brightness == 0.0);
}

TEST(non_finite_angle_turns_lamp_off) {
    InteractionMapping m;
    CHECK(m.update(true, std::numeric_limits<double>::quiet_NaN()).on == false);
    CHECK(m.update(true, std::numeric_limits<double>::infinity()).on == false);
    CHECK(m.update(true, -std::numeric_limits<double>::infinity()).on == false);
}

TEST(marker_found_turns_lamp_on) {
    InteractionMapping m;
    CHECK(m.update(true, 0.0).on == true);
}

TEST(brightness_always_within_0_and_1) {
    for (int a = -180; a <= 180; ++a) {
        double b = brightnessAt(a);
        CHECK(b >= 0.0 && b <= 1.0);
    }
}

// rotating the marker should always change the brightness
TEST(no_dead_zone_beyond_90_degrees) {
    CHECK(brightnessAt(100.0) != brightnessAt(170.0));
    CHECK(brightnessAt(-100.0) != brightnessAt(-170.0));
}

// atan2 wraps from +180 to -180 as the marker turns
TEST(no_jump_at_the_180_degree_wrap) {
    double maxStep = 2.0 / 180.0 + 1e-9;
    CHECK_NEAR(brightnessAt(179.0), brightnessAt(-179.0), maxStep);
    CHECK_NEAR(brightnessAt(180.0), brightnessAt(-180.0), 1e-9);
}

TEST(brightness_is_continuous_over_a_full_turn) {
    double maxStep = 1.0 / 180.0 + 1e-9;
    double worst = 0.0;
    int worstAngle = 0;
    double prev = brightnessAt(-180.0);
    for (int a = -179; a <= 180; ++a) {
        double cur = brightnessAt(a);
        double step = std::abs(cur - prev);
        if (step > worst) {
            worst = step;
            worstAngle = a;
        }
        prev = cur;
    }
    // wrap step: 180 -> -180, next frame after a full turn
    double wrapStep = std::abs(brightnessAt(-180.0) - brightnessAt(180.0));
    if (wrapStep > worst) {
        worst = wrapStep;
        worstAngle = -180;
    }
    std::cout << "  largest 1-degree brightness step: " << worst
              << " at angle " << worstAngle << "\n";
    CHECK(worst <= maxStep);
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
