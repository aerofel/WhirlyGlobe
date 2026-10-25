/*  MipmapRGBA8.h
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

#import <vector>
#import <cstdint>

namespace WhirlyKit
{

/// Next mipmap level of an RGBA8 image: half the size (odd sizes round down, minimum 1, as Metal
/// and GL size levels), each pixel the rounded mean of the 2x2 block above it. With an odd size
/// the last column/row averages 3 source columns/rows, so no source pixel is dropped.
/// Used to fill texture mip levels on the CPU, so the GPU never has to generate them.
void DownsampleRGBA8(const uint8_t *src,int width,int height,
                     std::vector<uint8_t> &dst,int &dstWidth,int &dstHeight);

}
