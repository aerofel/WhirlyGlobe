/*  MipmapRGBA8Tests.cpp
 *  WhirlyGlobeLib host tests (offto fork)
 */

#import "MipmapRGBA8.h"
#import <cstdio>

using namespace WhirlyKit;

static int failures = 0;
#define CHECK(cond) do { if (!(cond)) { std::fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond); failures++; } } while (0)

int main()
{
    std::vector<uint8_t> dst;
    int w = 0, h = 0;

    // 2x2 -> 1x1: rounded mean per channel
    const uint8_t quad[16] = { 0,10,255,1,  1,10,255,2,  2,20,255,3,  3,21,255,4 };
    DownsampleRGBA8(quad, 2, 2, dst, w, h);
    CHECK(w == 1 && h == 1 && dst.size() == 4);
    CHECK(dst[0] == 2 && dst[1] == 15 && dst[2] == 255 && dst[3] == 3);   // 6/4=1.5->2, 61/4=15.25->15, 10/4=2.5->3

    // 4x2 -> 2x1: each output only sees its own 2x2 block
    std::vector<uint8_t> img(4 * 2 * 4, 0);
    for (int y = 0; y < 2; y++)
        for (int x = 2; x < 4; x++)
            img[(y * 4 + x) * 4] = 200;
    DownsampleRGBA8(img.data(), 4, 2, dst, w, h);
    CHECK(w == 2 && h == 1);
    CHECK(dst[0] == 0 && dst[4] == 200);

    // Odd and thin sizes: 3x1 -> 1x1, 1x1 -> 1x1 (repeat the edge, never read past it)
    const uint8_t row[12] = { 100,0,0,0,  200,0,0,0,  50,0,0,0 };
    DownsampleRGBA8(row, 3, 1, dst, w, h);
    CHECK(w == 1 && h == 1 && dst[0] == 150);
    DownsampleRGBA8(row, 1, 1, dst, w, h);
    CHECK(w == 1 && h == 1 && dst[0] == 100);

    // A full 256 chain ends at 1x1 after 8 steps and keeps a flat colour flat
    std::vector<uint8_t> level(256 * 256 * 4, 77);
    int lw = 256, lh = 256, steps = 0;
    while (lw > 1 || lh > 1) {
        std::vector<uint8_t> next;
        DownsampleRGBA8(level.data(), lw, lh, next, lw, lh);
        level.swap(next);
        steps++;
    }
    CHECK(steps == 8 && level.size() == 4 && level[0] == 77 && level[3] == 77);

    if (failures == 0)
        std::printf("MipmapRGBA8Tests: all passed\n");
    return failures == 0 ? 0 : 1;
}
