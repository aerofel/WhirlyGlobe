/*  QIFDrawStateCache.h
 *  WhirlyGlobeLib (offto fork)
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#import <unordered_map>
#import <vector>
#import <cstdint>
#import "Identifiable.h"

namespace WhirlyKit
{

/// One texture slot of a tile drawable: which texture, and which part of it (an ancestor's
/// texture is used through a sub-rectangle while the tile's own one loads).
struct QIFDrawTex
{
    QIFDrawTex() = default;
    QIFDrawTex(SimpleIdentity texID,int size,int border,int relLevel,int relX,int relY) :
        texID(texID), size(size), border(border), relLevel(relLevel), relX(relX), relY(relY), relSet(true) { }

    bool operator==(const QIFDrawTex &that) const;
    bool operator!=(const QIFDrawTex &that) const { return !(*this == that); }

    SimpleIdentity texID = EmptyIdentity;
    int size = 0, border = 0, relLevel = 0, relX = 0, relY = 0;
    bool relSet = false;
};

/// What one tile drawable should look like after a render-state update.
struct QIFDrawState
{
    bool on = false;
    QIFDrawTex tex[2];
    // Uniforms are only meaningful (and only sent) while the drawable is on
    bool hasUniforms = false;
    float t = 0.0f;
    unsigned char color[4] = {0,0,0,0};
    // Allowed to fade in when its own texture first replaces a fallback (crossfade)
    bool fadeIn = false;
};

/// offto: one active frame's textures as the render state sees them: the texture IDs and the
/// tile they belong to (the tile itself, or an ancestor while its own load is pending).
struct QIFFrameTextures
{
    const std::vector<SimpleIdentity> *texIDs = nullptr;
    int nodeLevel = 0, nodeX = 0, nodeY = 0;
};

/**
    offto: which texture each slot of a tile drawable shows.
    Two active frames: slot = frame, the first texture of each (one texture per frame).
    One active frame: its textures fill slots 0 ..< min(count, texturesPerFrame, 2) (a frame that
    carries day + night together); the other slot stays empty.
    Returns false when an active frame has no texture yet (the drawable is then turned off).
 */
bool QIFTileTextures(int tileLevel,int tileX,int tileY,
                     const QIFFrameTextures *frames,int numFrames,int texturesPerFrame,
                     int texSize,int borderSize,QIFDrawTex out[2]);

/**
    Remembers what each tile drawable last received from QIFRenderState::updateScene, so an
    update only sends what changed. Without it every update re-sent textures, on/off and
    uniforms for every tile (thousands of change requests, each re-encoding a drawable).

    Render thread only. A value is remembered only if the drawable already exists in the scene
    when it is sent; a request for a missing drawable is dropped by the scene, so it is sent
    again next time instead.
 */
class QIFDrawStateCache
{
public:
    /// Receives the requests to send, in the order the old code sent them.
    struct Emitter
    {
        virtual ~Emitter() = default;
        virtual void onOff(SimpleIdentity drawID,bool on) = 0;
        virtual void texture(SimpleIdentity drawID,int which,const QIFDrawTex &tex) = 0;
        virtual void uniforms(SimpleIdentity drawID,float t,const unsigned char color[4]) = 0;
        /// Fade the drawable in: its own texture just replaced a fallback (or nothing)
        virtual void fadeIn(SimpleIdentity drawID) = 0;
    };

    /// Start an update; drawables not passed to apply() before endPass() are forgotten.
    void beginPass();
    /// Send what differs between `want` and what drawID last received.
    void apply(SimpleIdentity drawID,const QIFDrawState &want,bool existsInScene,Emitter &emit);
    void endPass();

    /// Forget everything (everything is sent again on the next update)
    void clear() { entries.clear(); }
    size_t size() const { return entries.size(); }

protected:
    struct Entry
    {
        QIFDrawState state;
        uint32_t pass = 0;
    };
    std::unordered_map<SimpleIdentity,Entry> entries;
    uint32_t pass = 0;
};

}
