/*  GlobeMotionTests.cpp
 *  WhirlyGlobeLib host tests (offto fork)
 */

#import "GlobeMotion.h"
#import <cstdio>
#import <cmath>

using namespace WhirlyKit;

static int failures = 0;
#define CHECK(cond) do { if (!(cond)) { std::fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond); failures++; } } while (0)

static Eigen::Quaterniond rotZ(double rad) { return Eigen::Quaterniond(Eigen::AngleAxisd(rad, Eigen::Vector3d::UnitZ())); }

int main()
{
    Eigen::Quaterniond q; double h;

    // Not moving: no prediction
    CHECK(!PredictGlobeMotion(0.0, rotZ(0.3), 1.0, 0.016, rotZ(0.3), 1.0, 0.12, q, h));

    // Steady spin: 0.01 rad per 16 ms -> 0.075 rad more after 120 ms, height unchanged
    CHECK(PredictGlobeMotion(0.0, rotZ(0.30), 1.0, 0.016, rotZ(0.31), 1.0, 0.12, q, h));
    CHECK(std::abs(q.angularDistance(rotZ(0.31 + 0.075))) < 1e-6);
    CHECK(std::abs(h - 1.0) < 1e-9);

    // Zooming in: height x0.98 per 16 ms -> x0.98^7.5 after 120 ms
    CHECK(PredictGlobeMotion(0.0, rotZ(0.3), 1.0, 0.016, rotZ(0.3), 0.98, 0.12, q, h));
    CHECK(std::abs(h - 0.98 * std::pow(0.98, 7.5)) < 1e-9);
    CHECK(std::abs(q.angularDistance(rotZ(0.3))) < 1e-9);

    // Fast spin is clamped to 0.15 rad ahead
    CHECK(PredictGlobeMotion(0.0, rotZ(0.0), 1.0, 0.016, rotZ(0.1), 1.0, 0.12, q, h));
    CHECK(std::abs(q.angularDistance(rotZ(0.1)) - 0.15) < 1e-6);

    // Fast zoom is clamped to a factor of 2
    CHECK(PredictGlobeMotion(0.0, rotZ(0.0), 1.0, 0.016, rotZ(0.0), 0.5, 0.12, q, h));
    CHECK(std::abs(h - 0.25) < 1e-9);

    // Samples too far apart (the camera stopped, then moved once): no prediction
    CHECK(!PredictGlobeMotion(0.0, rotZ(0.0), 1.0, 0.5, rotZ(0.1), 1.0, 0.12, q, h));
    // Samples too close together (two watchers in the same frame): no prediction from them
    CHECK(!PredictGlobeMotion(0.0, rotZ(0.0), 1.0, 0.001, rotZ(0.1), 1.0, 0.12, q, h));
    // Look-ahead off
    CHECK(!PredictGlobeMotion(0.0, rotZ(0.0), 1.0, 0.016, rotZ(0.01), 1.0, 0.0, q, h));

    if (failures)
    {
        std::fprintf(stderr, "%d check(s) failed\n", failures);
        return 1;
    }
    std::printf("GlobeMotionTests: all passed\n");
    return 0;
}
