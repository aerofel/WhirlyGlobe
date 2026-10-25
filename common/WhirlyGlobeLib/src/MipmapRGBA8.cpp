/*  MipmapRGBA8.cpp
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

#import "MipmapRGBA8.h"
#import <algorithm>

namespace WhirlyKit
{

void DownsampleRGBA8(const uint8_t *src,int width,int height,
                     std::vector<uint8_t> &dst,int &dstWidth,int &dstHeight)
{
    dstWidth = std::max(1,width/2);
    dstHeight = std::max(1,height/2);
    dst.resize((size_t)dstWidth * dstHeight * 4);

    for (int y=0;y<dstHeight;y++) {
        const uint8_t *row0 = src + (size_t)std::min(2*y,height-1) * width * 4;
        const uint8_t *row1 = src + (size_t)std::min(2*y+1,height-1) * width * 4;
        uint8_t *out = &dst[(size_t)y * dstWidth * 4];
        for (int x=0;x<dstWidth;x++) {
            const int x0 = std::min(2*x,width-1) * 4;
            const int x1 = std::min(2*x+1,width-1) * 4;
            for (int c=0;c<4;c++)
                out[x*4+c] = (uint8_t)((row0[x0+c] + row0[x1+c] + row1[x0+c] + row1[x1+c] + 2) >> 2);
        }
    }
}

}
