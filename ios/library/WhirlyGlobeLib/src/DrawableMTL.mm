/*  DrawableMTL.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 3/10/20.
 *  Copyright 2011-2022 mousebird consulting
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

#import "DrawableMTL.h"
#import <mutex>
#import <map>
#import <set>

using namespace Eigen;

namespace
{
// offto: argument encoders and their reflection, shared per (shader function, buffer index).
// Creating them (Metal reflection) for every new tile drawable was ~23 % of the sampling-layer
// thread on an iPad. Sharing is safe because an encoder is only used on the render thread, in
// preProcess, one drawable at a time, and every use binds its own buffer first
// (setArgumentBuffer); construction elsewhere only reads encodedLength. The key is the function
// object itself, so a separate renderer (e.g. an offline one on another thread) has its own
// programs, functions and encoders.
struct CachedArgEncoder
{
    id<MTLFunction> func;              // kept alive so its address cannot be reused as a key
    id<MTLArgumentEncoder> encode;
    std::vector<int> structEntries;    // argument indices of the struct members (uniform blocks)
    std::set<std::string> constants;   // names of the bool members
};

std::mutex argEncoderLock;
std::map<std::pair<const void *,int>,CachedArgEncoder> argEncoders;

CachedArgEncoder cachedArgEncoder(id<MTLFunction> func,int bufferArgIdx)
{
    std::lock_guard<std::mutex> guardLock(argEncoderLock);
    const auto key = std::make_pair((__bridge const void *)func,bufferArgIdx);
    const auto it = argEncoders.find(key);
    if (it != argEncoders.end())
        return it->second;

    CachedArgEncoder entry;
    entry.func = func;
    MTLAutoreleasedArgument argInfo;
    entry.encode = [func newArgumentEncoderWithBufferIndex:bufferArgIdx reflection:&argInfo];
    if (entry.encode) {
        if (argInfo.bufferDataType != MTLDataTypeStruct) {
            NSLog(@"Unexpected buffer data type in Metal Function %@",func.name);
        }
        NSArray<MTLStructMember *> *members = argInfo.bufferStructType.members;
        if (!members) {
            NSLog(@"Unexpected buffer structure in Metal Function %@",func.name);
        }
        for (MTLStructMember *mem in members) {
            if (mem.dataType == MTLDataTypeStruct) {
                entry.structEntries.push_back((int)mem.argumentIndex);
            } else if (mem.dataType == MTLDataTypeBool) {
                entry.constants.insert([mem.name cStringUsingEncoding:NSASCIIStringEncoding]);
            }
        }
    }
    argEncoders[key] = entry;
    return entry;
}
}

namespace WhirlyKit
{

DrawableMTL::DrawableMTL()
{
    // no premultiply by default on Metal
    setBlendPremultipliedAlpha(false);
}

ArgBuffContentsMTL::ArgBuffContentsMTL(id<MTLDevice> mtlDevice,RenderSetupInfoMTL *inSetupInfoMTL,id<MTLFunction> func,int bufferArgIdx,BufferBuilderMTL *buffBuild)
{
    valid = false;
    setupInfoMTL = inSetupInfoMTL;
    
    // Shared encoder + reflection for this function and buffer (see cachedArgEncoder)
    const CachedArgEncoder cached = cachedArgEncoder(func,bufferArgIdx);
    encode = cached.encode;
    if (!encode)
        return;
    
    // Work through the members, adding an entry for each one we're to fill in
    for (const int entryID : cached.structEntries) {
        entries[entryID] = std::make_shared<Entry>(entryID);
    }
    constants = cached.constants;
    
    // Create a buffer to store the arguments in
    if (buffBuild)
        buffBuild->reserveData([encode encodedLength], &buff);
    
    isSetup = true;
    valid = true;
}

bool ArgBuffContentsMTL::isEmpty()
{
    return [encode encodedLength] == 0;
}

bool ArgBuffContentsMTL::hasEntry(int entryID)
{
    return entries.find(entryID) != entries.end();
}

bool ArgBuffContentsMTL::hasConstant(const std::string &name)
{
    return constants.find(name) != constants.end();
}

void ArgBuffContentsMTL::startEncoding(id<MTLDevice> mtlDevice)
{
    NSUInteger len = [encode encodedLength];
    if (len > 0) {
        tmpBuff = setupInfoMTL->heapManage.allocateBuffer(HeapManagerMTL::Drawable, len);
        [encode setArgumentBuffer:tmpBuff.buffer offset:tmpBuff.offset];
    }
}

void ArgBuffContentsMTL::endEncoding(id<MTLDevice> mtlDevice, id<MTLBlitCommandEncoder> blitEncode)
{
    if (!tmpBuff.buffer)
        return;
    [blitEncode copyFromBuffer:tmpBuff.buffer sourceOffset:tmpBuff.offset toBuffer:buff.buffer destinationOffset:buff.offset size:[tmpBuff.buffer length]];
}

void ArgBuffContentsMTL::updateEntry(id<MTLDevice> mtlDevice,id<MTLBlitCommandEncoder> blitEncode,int entryID,void *rawData,size_t size)
{
    const auto it = entries.find(entryID);
    if (it == entries.end())
        return;
    const auto entry = it->second;
    
    memcpy([encode constantDataAtIndex:entry->entryID], rawData, size);
}

void ArgBuffContentsMTL::addResources(ResourceRefsMTL &resources)
{
    resources.addEntry(buff);
    resources.addEntry(tmpBuff);
}

ArgBuffRegularTexturesMTL::ArgBuffRegularTexturesMTL(id<MTLDevice> mtlDevice, RenderSetupInfoMTL *setupInfoMTL, id<MTLFunction> mtlFunction, int bufferArgIdx, BufferBuilderMTL *buildBuff)
{
    encode = cachedArgEncoder(mtlFunction,bufferArgIdx).encode;
    size = [encode encodedLength];
    buffer = setupInfoMTL->heapManage.allocateBuffer(HeapManagerMTL::HeapType::Drawable, size);
    if (buildBuff)
        buildBuff->reserveData(size, &buffer);
}

void ArgBuffRegularTexturesMTL::addTexture(const Point2f &offset,const Point2f &scale,id<MTLTexture> tex)
{
    offsets.push_back(offset);
    scales.push_back(scale);
    texs.push_back(tex);
}

void ArgBuffRegularTexturesMTL::updateBuffer(id<MTLDevice> mtlDevice,RenderSetupInfoMTL *setupInfoMTL,id<MTLBlitCommandEncoder> bltEncode)
{
    srcBuffer = setupInfoMTL->heapManage.allocateBuffer(HeapManagerMTL::Drawable, size);

    [encode setArgumentBuffer:srcBuffer.buffer offset:srcBuffer.offset];
    
    // TexIndirect constants first
    memcpy([encode constantDataAtIndex:WKSTexBuffIndirectOffset], &offsets[0], sizeof(float)*2*offsets.size());
    memcpy([encode constantDataAtIndex:WKSTexBuffIndirectScale], &scales[0], sizeof(float)*2*scales.size());

    // Then the textures, which are largely opaque
    uint32_t texturesPresent = 0;
    for (unsigned int ii=0;ii<WKSTextureMax;ii++) {
        id<MTLTexture> tex = ii>=texs.size() ? nil : texs[ii];
        [encode setTexture:tex atIndex:WKSTexBuffTextures+ii];
        if (tex) {
            // numTextures refers to the base textures, rather than program provide textures
            texturesPresent |= (1<<ii);
        }
    }
    memcpy([encode constantDataAtIndex:WKSTexBufTexPresent], &texturesPresent, sizeof(uint32_t));
    
    [bltEncode copyFromBuffer:srcBuffer.buffer sourceOffset:srcBuffer.offset toBuffer:buffer.buffer destinationOffset:buffer.offset size:size];
    
    offsets.clear();
    scales.clear();
    texs.clear();
}

size_t ArgBuffRegularTexturesMTL::encodedLength()
{
    return size;
}

void ArgBuffRegularTexturesMTL::addResources(ResourceRefsMTL &resources)
{
    resources.addEntry(srcBuffer);
    resources.addEntry(buffer);
}
    
}

