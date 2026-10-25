/*  QuadImageFrameLoader_jni.cpp
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 4/8/19.
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

#import "QuadLoading_jni.h"
#import "Geometry_jni.h"
#import "Scene_jni.h"
#import "Renderer_jni.h"
#import "com_mousebird_maply_QuadImageFrameLoader.h"

using namespace WhirlyKit;

extern "C"
JNIEXPORT jboolean JNICALL Java_com_mousebird_maply_QuadImageFrameLoader_setLoadFrameModeNative
  (JNIEnv *env, jobject obj, jint mode)
{
    try
    {
        if (const auto loader = QuadImageFrameLoaderClassInfo::get(env,obj))
        {
            const auto newMode = (QuadImageFrameLoader::LoadMode)mode;
            if ((*loader)->getLoadMode() != newMode)
            {
                (*loader)->setLoadMode(newMode);
                return true;
            }
        }
    }
    MAPLY_STD_JNI_CATCH()
    return false;
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_QuadImageFrameLoader_addFocus
  (JNIEnv *env, jobject obj)
{
    try
    {
        if (const auto loader = QuadImageFrameLoaderClassInfo::get(env,obj))
        {
            (*loader)->addFocus();
        }
    }
    MAPLY_STD_JNI_CATCH()
}

extern "C"
JNIEXPORT jint JNICALL Java_com_mousebird_maply_QuadImageFrameLoader_getNumFocus
  (JNIEnv *env, jobject obj)
{
    try
    {
        if (const auto loader = QuadImageFrameLoaderClassInfo::get(env,obj))
        {
            return (*loader)->getNumFocus();
        }
    }
    MAPLY_STD_JNI_CATCH()
    return 0;
}

extern "C"
JNIEXPORT jboolean JNICALL Java_com_mousebird_maply_QuadImageFrameLoader_setCurrentImageNative
  (JNIEnv *env, jobject obj, jint focusID, jdouble curImage)
{
    try
    {
        if (const auto loader = QuadImageFrameLoaderClassInfo::get(env,obj))
        {
            PlatformInfo_Android platformInfo(env);
            const double oldPos = (*loader)->getCurFrame(focusID);
            (*loader)->setCurFrame(&platformInfo, focusID, curImage);

            return (int) oldPos != (int) curImage;
        }
    }
    MAPLY_STD_JNI_CATCH()
    return false;
}

extern "C"
JNIEXPORT jdouble JNICALL Java_com_mousebird_maply_QuadImageFrameLoader_getCurrentImage
  (JNIEnv *env, jobject obj, jint focusID)
{
    try
    {
        if (const auto loader = QuadImageFrameLoaderClassInfo::get(env,obj))
        {
            return (*loader)->getCurFrame(focusID);
        }
    }
    MAPLY_STD_JNI_CATCH()
    return 0.0;
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_QuadImageFrameLoader_setRequireTopTiles
  (JNIEnv *env, jobject obj, jboolean loadTopTiles)
{
    try {
        if (const auto loader = QuadImageFrameLoaderClassInfo::get(env,obj))
        {
            (*loader)->setRequireTopTilesLoaded(loadTopTiles);
        }
    }
    MAPLY_STD_JNI_CATCH()
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_QuadImageFrameLoader_setRenderTargetIDNative
  (JNIEnv *env, jobject obj, jint focusID, jlong targetID)
{
    try
    {
        if (const auto loader = QuadImageFrameLoaderClassInfo::get(env,obj))
        {
            (*loader)->setRenderTarget(focusID,targetID);
        }
    }
    MAPLY_STD_JNI_CATCH()
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_QuadImageFrameLoader_setTextureSize
  (JNIEnv *env, jobject obj, jint texSize, jint borderSize)
{
    try
    {
        if (const auto loader = QuadImageFrameLoaderClassInfo::get(env,obj))
        {
            (*loader)->setTexSize(texSize,borderSize);
        }
    }
    MAPLY_STD_JNI_CATCH()
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_QuadImageFrameLoader_setShaderIDNative
  (JNIEnv *env, jobject obj, jint focusID, jlong shaderID)
{
    try
    {
        if (const auto loader = QuadImageFrameLoaderClassInfo::get(env,obj))
        {
            (*loader)->setShaderID(focusID,shaderID);
        }
    }
    MAPLY_STD_JNI_CATCH()
}

extern "C"
JNIEXPORT jint JNICALL Java_com_mousebird_maply_QuadImageFrameLoader_getStatsNative
  (JNIEnv *env, jobject obj, jintArray totalTilesArr, jintArray tilesToLoadArr, jlongArray countsArr, jdoubleArray secondsArr)
{
    try
    {
        if (const auto loader = QuadImageFrameLoaderClassInfo::get(env,obj))
        {
            QuadImageFrameLoader::Stats stats = (*loader)->getStats();
            const int numFrames = stats.frameStats.size();
            std::vector<int> totalTiles(numFrames), tilesToLoad(numFrames);
            for (unsigned int ii = 0; ii < numFrames; ii++)
            {
                auto &frame = stats.frameStats[ii];
                totalTiles[ii] = frame.totalTiles;
                tilesToLoad[ii] = frame.tilesToLoad;
            }
            // Even taking the address of element zero is technically undefined on an empty vector
            if (!totalTiles.empty())
            {
                env->SetIntArrayRegion(totalTilesArr, 0, totalTiles.size(), &totalTiles[0]);
            }
            if (!tilesToLoad.empty())
            {
                env->SetIntArrayRegion(tilesToLoadArr, 0, tilesToLoad.size(), &tilesToLoad[0]);
            }

            // offto: pipeline counters, in QuadImageFrameLoader.Stats.Pipeline order
            const auto &p = stats.pipeline;
            const jlong counts[18] = {
                p.merges, p.renderStateBuilds, p.sceneUpdates, p.sceneUpdateChanges,
                p.enabledTiles, p.fallbackTiles, p.maxFallbackRelLevel,
                p.requests, p.cancels, p.mergesOK, p.mergesCancelled, p.mergesFailed,
                p.latencyHist[0], p.latencyHist[1], p.latencyHist[2],
                p.latencyHist[3], p.latencyHist[4], p.latencyHist[5] };
            const jdouble seconds[5] = {
                p.mergeSeconds, p.renderStateBuildSeconds, p.sceneUpdateSeconds,
                p.waitSeconds, p.parseSeconds };
            env->SetLongArrayRegion(countsArr, 0, 18, counts);
            env->SetDoubleArrayRegion(secondsArr, 0, 5, seconds);

            return stats.numTiles;
        }
    }
    MAPLY_STD_JNI_CATCH()
    return 0;
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_QuadImageFrameLoader_updatePriorities
  (JNIEnv *env, jobject obj)
{
    try
    {
        if (const auto loader = QuadImageFrameLoaderClassInfo::get(env,obj))
        {
            PlatformInfo_Android platformInfo(env);
            (*loader)->updatePriorities(&platformInfo);
        }
    }
    MAPLY_STD_JNI_CATCH()
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_QuadImageFrameLoader_setTextureFadeDuration
  (JNIEnv *env, jobject obj, jdouble value)
{
    try {
        if (const auto loader = QuadImageFrameLoaderClassInfo::get(env,obj))
        {
            (*loader)->setTextureFadeDuration(value);
        }
    }
    MAPLY_STD_JNI_CATCH()
}

extern "C"
JNIEXPORT jdouble JNICALL Java_com_mousebird_maply_QuadImageFrameLoader_getTextureFadeDuration
  (JNIEnv *env, jobject obj)
{
    try {
        if (const auto loader = QuadImageFrameLoaderClassInfo::get(env,obj))
        {
            return (*loader)->getTextureFadeDuration();
        }
    }
    MAPLY_STD_JNI_CATCH()
    return 0.0;
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_QuadImageFrameLoader_setUsesMipmaps
  (JNIEnv *env, jobject obj, jboolean value)
{
    try {
        if (const auto loader = QuadImageFrameLoaderClassInfo::get(env,obj))
        {
            (*loader)->setUsesMipmaps(value);
        }
    }
    MAPLY_STD_JNI_CATCH()
}

extern "C"
JNIEXPORT jboolean JNICALL Java_com_mousebird_maply_QuadImageFrameLoader_getUsesMipmaps
  (JNIEnv *env, jobject obj)
{
    try {
        if (const auto loader = QuadImageFrameLoaderClassInfo::get(env,obj))
        {
            return (*loader)->getUsesMipmaps();
        }
    }
    MAPLY_STD_JNI_CATCH()
    return false;
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_QuadImageFrameLoader_setImagesPerTile
  (JNIEnv *env, jobject obj, jint value)
{
    try {
        if (const auto loader = QuadImageFrameLoaderClassInfo::get(env,obj))
        {
            (*loader)->setImagesPerTile(value);
        }
    }
    MAPLY_STD_JNI_CATCH()
}

extern "C"
JNIEXPORT jint JNICALL Java_com_mousebird_maply_QuadImageFrameLoader_getImagesPerTile
  (JNIEnv *env, jobject obj)
{
    try {
        if (const auto loader = QuadImageFrameLoaderClassInfo::get(env,obj))
        {
            return (*loader)->getImagesPerTile();
        }
    }
    MAPLY_STD_JNI_CATCH()
    return 1;
}
