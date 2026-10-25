/*
 *  MaplyQuadSampler.h
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Steve Gifford on 3/27/18.
 *  Copyright 2011-2022 Saildrone Inc
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

#import <WhirlyGlobe/MaplyControllerLayer.h>
#import <WhirlyGlobe/MaplyCoordinateSystem.h>
#import <WhirlyGlobe/MaplyTileSourceNew.h>
#import <WhirlyGlobe/MaplyRenderController.h>

/**
    Sampling parameters.
 
    These are used to describe how we want to break down the globe or
    flat projection onto the globe.
  */
@interface MaplySamplingParams : NSObject

/// The coordinate system we'll be sampling from.
@property (nonatomic,nonnull,strong) MaplyCoordinateSystem *coordSys;

/// Min zoom level for sampling.  Don't set this to anything other than 0 or 1
/// (larger values are ignored; use residentBaseLevel).
@property (nonatomic) int minZoom;

/**
    Keep EVERY tile of this level loaded, on- and off-screen, and never load anything coarser
    (offto fork). Sets minZoom to this level, forces it (forceMinLevel) and gives it an
    importance threshold of 0, which setMinImportance: then leaves alone.

    A loading tile then falls back to at worst this level, and turning the globe never reveals
    an area without imagery. Cost: 4^level tiles resident (64 at level 3). Unlike minZoom this
    accepts levels above 1: what made those unsafe (off-screen base tiles dropped, then missing
    after a spin) cannot happen when the whole level stays resident. Set it before
    setMinImportance: or after, either works. -1 = not used (default).
 */
@property (nonatomic) int residentBaseLevel;

/// Max zoom level for sampling
@property (nonatomic) int maxZoom;

/// We may want to calculate zoom levels beyond what we actually load.
/// Useful for zoom scaled features and enable/disable based on zoome
@property (nonatomic) int reportedMaxZoom;

/// Maximum number of tiles to load
@property (nonatomic) int maxTiles;

/// Cutoff for loading tiles.  This is size in screen space (pixels^2)
@property (nonatomic) double minImportance;

/// Normally we always load the lowest level
/// If this is set we only load those lowest level tiles that pass this test
/// Must be greater than zero and not equal to minImportance to take effect.
@property (nonatomic) double minImportanceTop;

/// Generate geometry to cover the north and south poles
/// Only works for world-wide projections
@property (nonatomic) bool coverPoles;

/// If set, generate skirt geometry to hide the edges between levels
@property (nonatomic) bool edgeMatching;

/// Tesselation values per level for breaking down the coordinate system (e.g. globe)
@property (nonatomic) int tessX,tessY;

/// If set, we'll scale the bounding boxes of individual tiles by this before evaluating
@property (nonatomic) float boundScale;

/// If set, we'll always load the lowest level first and then whatever the target level is
/// Turn this off to get true single level loading
/// `forceMinLevelHeight` must be greater than zero for this to have any effect.
@property (nonatomic) bool forceMinLevel;

/// If set, we'll turn on forceMinLevel and only use it when the viewer is above this height
@property (nonatomic) double forceMinLevelHeight;

/// If set, we'll try to load a single level
@property (nonatomic) bool singleLevel;

/// How often the quad tree re-evaluates which tiles to load while the view
/// is moving, in seconds.  0.0 = use the engine default (0.1s).
/// Smaller values sharpen raster basemaps during motion at some CPU cost.
@property (nonatomic) double viewUpdatePeriod;

/// If set, the tiles are clipped to this boundary
@property (nonatomic) MaplyBoundingBoxD clipBounds;
@property (nonatomic,readonly) bool hasClipBounds;

/**
 Detail the levels you want loaded in target level mode.
 
 The layer calculates the optimal target level.  The entries in this array are relative to that level or absolute.  For example [0,-4,-2] means the layer will always try to load levels 0, targetLevel-4 and targetLevel-2, but only the latter two if they make sense.
 */
@property (nonatomic,nullable,strong) NSArray *levelLoads;

/**
 Set the min importance for just one level.
 
 This is useful if you want your lower levels loaded more aggressively.
 */
- (void)setMinImportance:(double)minImportance forLevel:(int)level;

/// Decide if these sampling params are the same as others
- (bool)isEqualTo:(MaplySamplingParams *__nonnull)other;

@end
