/*  VertexAttributeTests.cpp
 *  WhirlyGlobeLib host tests (offto fork)
 */

#import "VertexAttribute.h"
#import <cstdio>

using namespace WhirlyKit;

static int failures = 0;
#define CHECK(cond) do { if (!(cond)) { std::fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond); failures++; } } while (0)

// Uniform sets as QIFRenderState::updateScene builds them: frame interpolation + colour.
static SingleVertexAttributeSet uniforms(float t, unsigned char alpha)
{
    unsigned char color[4] = {255, 255, 255, alpha};
    SingleVertexAttributeSet attrs;
    attrs.insert(SingleVertexAttribute(1, -1, t));
    attrs.insert(SingleVertexAttribute(2, -1, color));
    return attrs;
}

int main()
{
    CHECK(SameAttributeValues(uniforms(0.5f, 255), uniforms(0.5f, 255)));
    CHECK(!SameAttributeValues(uniforms(0.5f, 255), uniforms(0.25f, 255)));
    CHECK(!SameAttributeValues(uniforms(0.5f, 255), uniforms(0.5f, 128)));
    CHECK(!SameAttributeValues(uniforms(0.5f, 255), SingleVertexAttributeSet()));
    CHECK(SameAttributeValues(SingleVertexAttributeSet(), SingleVertexAttributeSet()));

    // Same name, different type: not the same value
    SingleVertexAttributeSet asInt, asFloat;
    asInt.insert(SingleVertexAttribute(1, -1, 1));
    asFloat.insert(SingleVertexAttribute(1, -1, 1.0f));
    CHECK(!SameAttributeValues(asInt, asFloat));

    if (failures)
    {
        std::fprintf(stderr, "%d check(s) failed\n", failures);
        return 1;
    }
    std::printf("VertexAttributeTests: all passed\n");
    return 0;
}
