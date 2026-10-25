/*  QIFDrawStateCacheTests.cpp
 *  WhirlyGlobeLib host tests (offto fork)
 */

#import "QIFDrawStateCache.h"
#import <cstdio>
#import <string>
#import <vector>

using namespace WhirlyKit;

static int failures = 0;
#define CHECK(cond) do { if (!(cond)) { std::fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond); failures++; } } while (0)

// Records what would have been sent to the scene
struct Recorder : QIFDrawStateCache::Emitter
{
    std::vector<std::string> sent;
    void onOff(SimpleIdentity id,bool on) override { sent.push_back(std::to_string(id) + (on ? " on" : " off")); }
    void texture(SimpleIdentity id,int which,const QIFDrawTex &tex) override
    { sent.push_back(std::to_string(id) + " tex" + std::to_string(which) + "=" + std::to_string(tex.texID) + " rel" + std::to_string(tex.relLevel)); }
    void uniforms(SimpleIdentity id,float t,const unsigned char color[4]) override
    { sent.push_back(std::to_string(id) + " uni t=" + std::to_string(t) + " a=" + std::to_string(color[3])); }
    void fadeIn(SimpleIdentity id) override { sent.push_back(std::to_string(id) + " fade"); fades++; }
    int fades = 0;
};

static QIFDrawState onWith(SimpleIdentity day,SimpleIdentity night,int rel = 0,float t = 0.5f)
{
    QIFDrawState s;
    s.on = true;
    s.tex[0] = QIFDrawTex(day, 256, 0, rel, 0, 0);
    s.tex[1] = QIFDrawTex(night, 256, 0, rel, 0, 0);
    s.hasUniforms = true;
    s.t = t;
    for (auto &c : s.color) c = 255;
    return s;
}

static QIFDrawState off()
{
    QIFDrawState s;
    s.on = false;
    s.tex[0] = QIFDrawTex();
    s.tex[1] = QIFDrawTex();
    return s;
}

int main()
{
    // First time: everything is sent
    {
        QIFDrawStateCache cache; Recorder r;
        cache.beginPass(); cache.apply(7, onWith(100, 101), true, r); cache.endPass();
        CHECK(r.sent.size() == 4);   // tex0, tex1, on, uniforms
    }
    // Same state again: nothing is sent
    {
        QIFDrawStateCache cache; Recorder r;
        cache.beginPass(); cache.apply(7, onWith(100, 101), true, r); cache.endPass();
        r.sent.clear();
        cache.beginPass(); cache.apply(7, onWith(100, 101), true, r); cache.endPass();
        CHECK(r.sent.empty());
    }
    // Only the night texture changes: only that is sent
    {
        QIFDrawStateCache cache; Recorder r;
        cache.beginPass(); cache.apply(7, onWith(100, 101), true, r); cache.endPass();
        r.sent.clear();
        cache.beginPass(); cache.apply(7, onWith(100, 202), true, r); cache.endPass();
        CHECK(r.sent.size() == 1 && r.sent[0] == "7 tex1=202 rel0");
    }
    // A better ancestor arrives: same texture IDs are not enough, the sub-rect changed
    {
        QIFDrawStateCache cache; Recorder r;
        cache.beginPass(); cache.apply(7, onWith(100, 101, 2), true, r); cache.endPass();
        r.sent.clear();
        cache.beginPass(); cache.apply(7, onWith(100, 101, 1), true, r); cache.endPass();
        CHECK(r.sent.size() == 2);
    }
    // Turning off clears both textures; turning back on resends them and the uniforms
    {
        QIFDrawStateCache cache; Recorder r;
        cache.beginPass(); cache.apply(7, onWith(100, 101), true, r); cache.endPass();
        r.sent.clear();
        cache.beginPass(); cache.apply(7, off(), true, r); cache.endPass();
        CHECK(r.sent.size() == 3 && r.sent[0] == "7 off");
        r.sent.clear();
        cache.beginPass(); cache.apply(7, onWith(100, 101), true, r); cache.endPass();
        CHECK(r.sent.size() == 3);   // tex0, tex1, on — uniforms unchanged while off
    }
    // A drawable not in the scene yet: sent, but not remembered (sent again next time)
    {
        QIFDrawStateCache cache; Recorder r;
        cache.beginPass(); cache.apply(7, onWith(100, 101), false, r); cache.endPass();
        r.sent.clear();
        cache.beginPass(); cache.apply(7, onWith(100, 101), false, r); cache.endPass();
        CHECK(r.sent.size() == 4);
        CHECK(cache.size() == 0);
    }
    // Drawables no longer in the render state are forgotten
    {
        QIFDrawStateCache cache; Recorder r;
        cache.beginPass(); cache.apply(7, onWith(100, 101), true, r); cache.apply(8, onWith(100, 101), true, r); cache.endPass();
        cache.beginPass(); cache.apply(8, onWith(100, 101), true, r); cache.endPass();
        CHECK(cache.size() == 1);
    }
    // Uniforms: a new interpolation value is sent alone
    {
        QIFDrawStateCache cache; Recorder r;
        cache.beginPass(); cache.apply(7, onWith(100, 101, 0, 0.5f), true, r); cache.endPass();
        r.sent.clear();
        cache.beginPass(); cache.apply(7, onWith(100, 101, 0, 0.75f), true, r); cache.endPass();
        CHECK(r.sent.size() == 1 && r.sent[0].find("uni") != std::string::npos);
    }

    // Crossfade: only when slot 0 goes from a fallback (ancestor, relLevel > 0) or nothing to the
    // tile's own texture (relLevel 0), and only when the state allows it
    {
        QIFDrawStateCache cache; Recorder r;
        auto fallback = onWith(100, 101, 2); fallback.fadeIn = true;
        auto own = onWith(300, 301, 0); own.fadeIn = true;
        cache.beginPass(); cache.apply(7, fallback, true, r); cache.endPass();
        CHECK(r.fades == 0);                       // a fallback never fades
        cache.beginPass(); cache.apply(7, own, true, r); cache.endPass();
        CHECK(r.fades == 1);                       // own texture arrives: fade
        cache.beginPass(); cache.apply(7, own, true, r); cache.endPass();
        CHECK(r.fades == 1);                       // unchanged: no new fade
        auto reloaded = onWith(400, 401, 0); reloaded.fadeIn = true;
        cache.beginPass(); cache.apply(7, reloaded, true, r); cache.endPass();
        CHECK(r.fades == 1);                       // own -> own (reload): no fade
    }
    {
        QIFDrawStateCache cache; Recorder r;
        auto own = onWith(300, 301, 0); own.fadeIn = false;   // e.g. the base level
        cache.beginPass(); cache.apply(7, onWith(100, 101, 1), true, r); cache.endPass();
        cache.beginPass(); cache.apply(7, own, true, r); cache.endPass();
        CHECK(r.fades == 0);
    }
    {
        // First sight of a drawable already showing its own texture fades too (a new tile)
        QIFDrawStateCache cache; Recorder r;
        auto own = onWith(300, 301, 0); own.fadeIn = true;
        cache.beginPass(); cache.apply(9, own, true, r); cache.endPass();
        CHECK(r.fades == 1);
        // The fade request comes after the texture it reveals
        CHECK(r.sent.back() == "9 fade" || r.sent[r.sent.size()-2] == "9 fade");
    }

    if (failures)
    {
        std::fprintf(stderr, "%d check(s) failed\n", failures);
        return 1;
    }
    std::printf("QIFDrawStateCacheTests: all passed\n");
    return 0;
}
