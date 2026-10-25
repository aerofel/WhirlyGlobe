/*  QIFDrawStateCache.cpp
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

#import "QIFDrawStateCache.h"
#import <cstring>

namespace WhirlyKit
{

bool QIFDrawTex::operator==(const QIFDrawTex &that) const
{
    return texID == that.texID && relSet == that.relSet && size == that.size && border == that.border &&
           relLevel == that.relLevel && relX == that.relX && relY == that.relY;
}

void QIFDrawStateCache::beginPass()
{
    pass++;
}

void QIFDrawStateCache::apply(SimpleIdentity drawID,const QIFDrawState &want,bool existsInScene,Emitter &emit)
{
    const auto it = existsInScene ? entries.find(drawID) : entries.end();
    const QIFDrawState *had = (it != entries.end()) ? &it->second.state : nullptr;

    if (want.on) {
        for (int ii=0;ii<2;ii++)
            if (!had || had->tex[ii] != want.tex[ii])
                emit.texture(drawID,ii,want.tex[ii]);
        if (!had || !had->on)
            emit.onOff(drawID,true);
        if (want.hasUniforms &&
            (!had || !had->hasUniforms || had->t != want.t || memcmp(had->color,want.color,4) != 0))
            emit.uniforms(drawID,want.t,want.color);
        // Crossfade: slot 0 now shows the tile's own texture, and before it showed an ancestor's
        // (relLevel > 0), nothing, or the drawable was off or new. Not on own -> own (reload).
        const auto ownTex = [](const QIFDrawTex &tex) { return tex.texID != EmptyIdentity && tex.relLevel == 0; };
        if (want.fadeIn && ownTex(want.tex[0]) && (!had || !had->on || !ownTex(had->tex[0])))
            emit.fadeIn(drawID);
    } else {
        if (!had || had->on)
            emit.onOff(drawID,false);
        for (int ii=0;ii<2;ii++)
            if (!had || had->tex[ii] != want.tex[ii])
                emit.texture(drawID,ii,want.tex[ii]);
    }

    if (!existsInScene)
        return;

    Entry &entry = entries[drawID];
    const bool keepUniforms = !want.hasUniforms && had && had->hasUniforms;
    const QIFDrawState previous = had ? *had : QIFDrawState();
    entry.state = want;
    if (keepUniforms) {
        // Off: the drawable still holds the uniforms it was last sent
        entry.state.hasUniforms = true;
        entry.state.t = previous.t;
        memcpy(entry.state.color,previous.color,4);
    }
    entry.pass = pass;
}

void QIFDrawStateCache::endPass()
{
    for (auto it = entries.begin(); it != entries.end(); ) {
        if (it->second.pass != pass)
            it = entries.erase(it);
        else
            ++it;
    }
}

}
