/*
 *  TextureMTL.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 5/16/19.
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
 *
 */

#import "Platform.h"
#import "RawData.h"
#import "Identifiable.h"
#import "WhirlyVector.h"
#import "Texture.h"
#import "WrapperMTL.h"
#import <UIKit/UIKit.h>

namespace WhirlyKit
{
    
/// Multiple texture variants derive from the base
class TextureBaseMTL : virtual public TextureBase
{
public:
    TextureBaseMTL(SimpleIdentity thisId) : TextureBase(thisId) { }
    TextureBaseMTL(const std::string &name) : TextureBase(name) { }
    ~TextureBaseMTL();
    
    /// Return the unique GL ID.
    id<MTLTexture> getMTLID() const { return texBuf.tex; }
    TextureEntryMTL getMTLTex() const { return texBuf; }
protected:
    /// Set to 0 if we haven't loaded yet
    TextureEntryMTL texBuf;
};

typedef std::shared_ptr<TextureBaseMTL> TextureBaseMTLRef;

/// This is your basic texture created from data
class TextureMTL : virtual public Texture, virtual public TextureBaseMTL
{
public:
    TextureMTL(const std::string &name);
    /// Construct with raw texture data.
    TextureMTL(const std::string &name,RawDataRef texData,bool isPVRTC);
    /// Construct by scaling the image to the given size
    TextureMTL(const std::string &name,UIImage *inImage,int width,int height);
    /// Construct with just the image
    TextureMTL(const std::string &name,UIImage *inImage);

    /// Creates the MTL resources
    virtual bool createInRenderer(const RenderSetupInfo *setupInfo);
    
    /// Tears down MTL resources
    virtual void destroyInRenderer(const RenderSetupInfo *setupInfo,Scene *inScene);

protected:
    // Convert our own raw data into bytes of the appropriate format
    RawDataRef convertData();
    RawDataRef convertData(const RawDataRef &data,int width,int height);
    /// offto: the data converted for upload, plus its mip levels (RGBA8 box filter) when
    /// usesMipmaps and the format allows it
    std::vector<RawDataRef> buildLevels(bool withMipmaps);
    bool canBuildMipLevels() const;

public:
    /// offto: build the upload levels now (on a parse worker); createInRenderer then only
    /// uploads them, unless the format or mipmap setting changed since
    virtual void prepareData() override;

protected:
    std::vector<RawDataRef> preparedLevels;
    TextureType preparedFormat = TexTypeUnsignedByte;
    bool preparedMipmaps = false;
};

typedef std::shared_ptr<TextureMTL> TextureMTLRef;
    
}
