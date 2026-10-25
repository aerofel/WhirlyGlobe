/*  QuadTreeNewTests.cpp
 *  WhirlyGlobeLib host tests (offto fork)
 *
 *  Runs on the host (macOS/Linux), no renderer. Build and run with test/run_tests.sh.
 */

#import "QuadTreeNew.h"
#import <cstdio>
#import <cmath>

using namespace WhirlyKit;

static int failures = 0;
#define CHECK(cond) do { if (!(cond)) { std::fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond); failures++; } } while (0)

// A quad tree whose "screen" is the western half of the world.
// Nodes that overlap it get an importance proportional to their area (1e8 / 4^level);
// nodes entirely in the eastern half get 0, as off-screen or back-facing tiles do in ScreenImportance.
class HalfScreenTree : public QuadTreeNew
{
public:
    HalfScreenTree(int minLevel, int maxLevel) : QuadTreeNew(MbrD(Point2d(0,0),Point2d(1,1)), minLevel, maxLevel) { }

    double importance(const Node &node) override
    {
        const double left = (double)node.x / (double)(1 << node.level);
        if (left >= 0.5)
            return 0.0;
        return 1e8 / std::pow(4.0, node.level);
    }
    bool visible(const Node &node) override { return importance(node) > 0.0; }
};

static std::vector<double> importances(int minLevel, int maxLevel, double minImportanceTop, double minImportance)
{
    std::vector<double> imp(maxLevel + 1);
    for (int ii = 0; ii <= maxLevel; ii++)
        imp[ii] = ii < minLevel ? MAXFLOAT : (ii == minLevel ? minImportanceTop : minImportance);
    return imp;
}

static int countLevel(const QuadTreeNew::ImportantNodeSet &nodes, int level)
{
    int n = 0;
    for (const auto &node : nodes)
        if (node.level == level)
            n++;
    return n;
}

// minImportanceTop = 0 must keep the WHOLE min level resident, including tiles under
// off-screen ancestors (z1/z2 importance 0 used to cut the walk in evalNodeVisible).
static void testWholeMinLevelResident()
{
    HalfScreenTree tree(3, 7);
    std::vector<double> rejected(8, 0.0);
    int level;
    QuadTreeNew::ImportantNodeSet nodes;
    // 64 (z3) + 128 (z4) + 512 (z5) = 704 nodes: the min level counts against maxNodes too.
    std::tie(level, nodes) = tree.calcCoverageVisible(importances(3, 7, 0.0, 176.0 * 176.0), 1024, {-1}, true, rejected);

    CHECK(level == 5);
    CHECK(countLevel(nodes, 3) == 64);
    CHECK(countLevel(nodes, 0) == 0);
    CHECK(countLevel(nodes, 1) == 0);
    CHECK(countLevel(nodes, 2) == 0);
    // Target and parent levels stay on-screen only
    CHECK(countLevel(nodes, 5) == 512);
    CHECK(countLevel(nodes, 4) == 128);
}

// The shipped setting (minImportanceTop = 1.0) is unchanged: off-screen min-level tiles stay out.
static void testShippedSettingUnchanged()
{
    HalfScreenTree tree(3, 7);
    std::vector<double> rejected(8, 0.0);
    int level;
    QuadTreeNew::ImportantNodeSet nodes;
    std::tie(level, nodes) = tree.calcCoverageVisible(importances(3, 7, 1.0, 176.0 * 176.0), 1024, {-1}, true, rejected);

    CHECK(level == 5);
    CHECK(countLevel(nodes, 3) == 32);
}

// minLevel 0 (the engine default) is unaffected by the fix.
static void testMinLevelZeroUnchanged()
{
    HalfScreenTree tree(0, 7);
    std::vector<double> rejected(8, 0.0);
    int level;
    QuadTreeNew::ImportantNodeSet nodes;
    std::tie(level, nodes) = tree.calcCoverageVisible(importances(0, 7, 0.0, 176.0 * 176.0), 1024, {-1}, true, rejected);

    CHECK(level == 5);
    CHECK(countLevel(nodes, 0) == 1);
    CHECK(countLevel(nodes, 5) == 512);
}

// maxNodes still drops the target level when the set does not fit.
static void testMaxNodesRespected()
{
    HalfScreenTree tree(3, 7);
    std::vector<double> rejected(8, 0.0);
    int level;
    QuadTreeNew::ImportantNodeSet nodes;
    std::tie(level, nodes) = tree.calcCoverageVisible(importances(3, 7, 0.0, 176.0 * 176.0), 300, {-1}, true, rejected);

    CHECK((int)nodes.size() <= 300);
    CHECK(level == 4);
    CHECK(countLevel(nodes, 3) == 64);
}

int main()
{
    testWholeMinLevelResident();
    testShippedSettingUnchanged();
    testMinLevelZeroUnchanged();
    testMaxNodesRespected();
    if (failures)
    {
        std::fprintf(stderr, "%d check(s) failed\n", failures);
        return 1;
    }
    std::printf("QuadTreeNewTests: all passed\n");
    return 0;
}
