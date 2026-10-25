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

    // Each output pixel averages source pixels [2x, 2x+1] x [2y, 2y+1]; with an odd size the
    // last output column/row also takes the leftover source column/row (3 wide), so none is lost.
    for (int y=0;y<dstHeight;y++) {
        const int y0 = std::min(2*y,height-1);
        const int y1 = (y == dstHeight-1) ? height-1 : 2*y+1;
        uint8_t *out = &dst[(size_t)y * dstWidth * 4];
        for (int x=0;x<dstWidth;x++) {
            const int x0 = std::min(2*x,width-1);
            const int x1 = (x == dstWidth-1) ? width-1 : 2*x+1;
            const int count = (y1-y0+1) * (x1-x0+1);
            unsigned sum[4] = {0,0,0,0};
            for (int sy=y0;sy<=y1;sy++) {
                const uint8_t *row = src + ((size_t)sy * width + x0) * 4;
                for (int sx=x0;sx<=x1;sx++,row+=4)
                    for (int c=0;c<4;c++)
                        sum[c] += row[c];
            }
            for (int c=0;c<4;c++)
                out[x*4+c] = (uint8_t)((sum[c] + count/2) / count);
        }
    }
}

}
