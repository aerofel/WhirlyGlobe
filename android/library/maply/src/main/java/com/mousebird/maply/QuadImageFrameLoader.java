

package com.mousebird.maply;

import android.os.Handler;

/**
 * The Maply Quad Image Frame Loader is for paging individual frames of image pyramids.
 * <br>
 * This works much like the Quad Image Loader, but handles more than one frame.  You can animate
 * between the frames with the QuadImageFrameAnimator.
 */
public class QuadImageFrameLoader extends QuadImageLoaderBase
{
    protected boolean valid = false;

    protected QuadImageFrameLoader() { }

    public QuadImageFrameLoader(BaseController control)
    {
        super(control);

        valid = true;
    }

    public QuadImageFrameLoader(final SamplingParams params,TileInfoNew inTileInfos[],BaseController control)
    {
        super(control, params, inTileInfos.length);
        tileInfos = inTileInfos;

        valid = true;
        Handler handler = new Handler(control.getActivity().getMainLooper());
        handler.post(new Runnable() {
            @Override
            public void run() {
                if (!valid)
                    return;

                delayedInit(params);
            }
        });
    }

    public enum FrameLoadMode {Broad,Narrow};

    /**
     * How frames are loaded (top down vs broad).  Top down is the default.
     */
    public void setLoadFrameMode(FrameLoadMode mode)
    {
        if (setLoadFrameModeNative(mode.ordinal()) && samplingLayer != null) {
            // If we changed the frame mode we may need to refresh the priorities
            QuadSamplingLayer layer = samplingLayer.get();
            if (layer == null || layer.layerThread == null)
                return;
            layer.layerThread.addTask(new Runnable() {
                @Override
                public void run() {
                    updatePriorities();
                }
            });
        }
    }

    protected native boolean setLoadFrameModeNative(int mode);
    protected native void updatePriorities();

    /**
     *   Add another rendering focus to the frame loader.
     *
     *   Normally you'd have one point of focus for a frame loader resulting in one image
     *   to be displayed.  But if you're using render targets, you may want to have two
     *   and combine them in some way yourself.  Or more.  No idea why you'd do that.
     *
     *   If you're going to do this, call addFocus right after you create the FrameLoader.
     */
    public native void addFocus();

    /**
     *   Return the number of focii.  Normally it's 1.
     *
     *   See addFocus for what these are.  You probably don't need to be using them.
     */
    public native int getNumFocus();

    /**
     * Set the interpolated location within the array of frames.
     * <br>
     * Each set of frames can be accessed from [0.0,numFrames].  Images will be interpolated between
     * those values and may be snapped if data has not yet loaded.
     * <br>
     * This value is used once per frame, so feel free to call this as much as you'd like.
     */
    public void setCurrentImage(double where)
    {
        setCurrentImage(0,where);
    }

    /**
     * Set the currentImage for the given focus.  See addFocus for what those are.
     * @param focusID Which focus to use.  Only use this method if you've got more than one.
     * @param where What to set the currentImage to.
     */
    public void setCurrentImage(int focusID,double where)
    {
//        double curFrame = std::min(std::max(where,0.0),(double)([loader->frameInfos count]-1));
        double curFrame = Math.min(Math.max(where,0.0),(double)(tileInfos.length-1));

        if (setCurrentImageNative(focusID,where) && samplingLayer != null) {
            // setCurrentImage tells us if we changed the actual image
            QuadSamplingLayer layer = samplingLayer.get();
            if (layer == null || layer.layerThread == null)
                return;
            layer.layerThread.addTask(new Runnable() {
                @Override
                public void run() {
                    updatePriorities();
                }
            });
        }
    }

    protected native boolean setCurrentImageNative(int focusID,double where);

    /**
     *   Return the interpolated location within the array of frames.
     */
    public double getCurrentImage()
    {
        return getCurrentImage(0);
    }

    /**
     * Return the interpolated location for a given focus within the array of frames.
     */
    public native double getCurrentImage(int focusID);

    /**
     *  Set whether we require the top tiles to be loaded before a frame can be displayed.
     *
     *  Normally the system wants all the top level tiles to be loaded (just one at level 0)
     *  to be in memory before it will display a frame at all.  You can turn this off.
     */
    public native void setRequireTopTiles(boolean newVal);

    /**
     *  An optional render target for this loader.
     *
     *  The loader can draw to a render target rather than to the screen.
     *  You use this in a multi-pass rendering setup.
     *
     *  This version takes a specific focus.  See addFocus for what that means.
     */
    public void setRenderTarget(int focusID,RenderTarget renderTarget)
    {
        setRenderTargetIDNative(focusID,renderTarget.renderTargetID);
    }

    protected native void setRenderTargetIDNative(int focusID,long renderTargetID);

    /**
     *  In special cases we may have tiles that already have borders baked in.  In that case, call this
     *  method to set both the total textures size and the number of border pixels around the outside.
     *
     *  By default this functionality is off.
     */
    public native void setTextureSize(int tileSize,int borderSize);

    /**
     *  Shader to use for rendering the image frames for a particular focus.
     *
     *  Consult addFocus for what this means.
     */
    public void setShader(int focusID,Shader shader)
    {
        setShaderIDNative(focusID,(shader != null) ? shader.getID() : 0);
    }

    protected native void setShaderIDNative(int focusID,long renderTargetID);

    /**
     * Number of tile sources passed in as individual frames.
     */
    public int getNumFrames() {
        return tileInfos.length;
    }

    /**
     * Change the tile sources all at once.  This also forces a reload.
     */
    public void changeTileInfo(final TileInfoNew[] newTileInfo) {
        super.changeTileInfo(newTileInfo);
    }

    @Override
    public void shutdown() {
        valid = false;

        super.shutdown();
    }

    /**
     * The Maply Quad Image Frame Loader can generation per-frame stats.  These are them.
     */
    public class FrameStats
    {
        /**
         * Number of tiles this frame is in (loading and loaded)
         */
        public int totalTiles;

        /**
         * Number of tiles this frame has yet to load
         */
        public int tilesToLoad;
    }

    /**
     * Stats generated by the Maply Quad Image Frame Loader.
     */
    public class Stats
    {
        /**
         * Total number of tiles managed by the loader
         */
        public int numTiles = 0;

        /**
         * Per frame stats for current loading state
         */
        public FrameStats[] frameStats = null;

        /**
         * offto: pipeline counters, cumulative since the loader started (read the deltas)
         */
        public Pipeline pipeline = new Pipeline();
    }

    /**
     * offto: the core's QuadImageFrameLoader::Stats::Pipeline, field for field.
     */
    public static class Pipeline
    {
        // Layer thread: tile merges and render-state builds, with their time
        public long merges;
        public double mergeSeconds;
        public long renderStateBuilds;
        public double renderStateBuildSeconds;
        // Render thread: scene updates, their time and the change requests they emitted
        public long sceneUpdates;
        public double sceneUpdateSeconds;
        public long sceneUpdateChanges;
        // Render thread, last scene update only
        public int enabledTiles;
        public int fallbackTiles;
        public int maxFallbackRelLevel;
        // Frame requests, cancels while loading, and merges by outcome
        public long requests;
        public long cancels;
        public long mergesOK;
        public long mergesCancelled;
        public long mergesFailed;
        // Successful merges: request -> data and data -> merged, summed
        public double waitSeconds;
        public double parseSeconds;
        // Request -> merged latency of successful merges: <50, <100, <200, <400, <800, >=800 ms
        public long[] latencyHistogram = new long[6];
    }

    /**
     * Pull out the per-frame instantaneous stats.
     */
    public Stats getStats()
    {
        int numFrames = getNumFrames();
        if (numFrames == 0)
            return null;

        Stats stats = new Stats();
        stats.frameStats = new FrameStats[numFrames];
        int totalTiles[] = new int[numFrames];
        int tilesToLoad[] = new int[numFrames];
        long counts[] = new long[18];
        double seconds[] = new double[5];

        // Fetch the data like this because I'm lazy
        stats.numTiles = getStatsNative(totalTiles, tilesToLoad, counts, seconds);
        Pipeline p = stats.pipeline;
        p.merges = counts[0];
        p.renderStateBuilds = counts[1];
        p.sceneUpdates = counts[2];
        p.sceneUpdateChanges = counts[3];
        p.enabledTiles = (int)counts[4];
        p.fallbackTiles = (int)counts[5];
        p.maxFallbackRelLevel = (int)counts[6];
        p.requests = counts[7];
        p.cancels = counts[8];
        p.mergesOK = counts[9];
        p.mergesCancelled = counts[10];
        p.mergesFailed = counts[11];
        System.arraycopy(counts, 12, p.latencyHistogram, 0, 6);
        p.mergeSeconds = seconds[0];
        p.renderStateBuildSeconds = seconds[1];
        p.sceneUpdateSeconds = seconds[2];
        p.waitSeconds = seconds[3];
        p.parseSeconds = seconds[4];
        for (int ii=0;ii<numFrames;ii++)
        {
            FrameStats frameStats = new FrameStats();
            frameStats.tilesToLoad = tilesToLoad[ii];
            frameStats.totalTiles = totalTiles[ii];
            stats.frameStats[ii] = frameStats;
        }

        return stats;
    }

    private native int getStatsNative(int[] totalTiles,int[] tilesToLoad,long[] counts,double[] seconds);

    /**
     * offto: fade a tile in over this many seconds when its own texture replaces the stretched
     * ancestor it was showing (0 = off, the default).
     */
    public native void setTextureFadeDuration(double seconds);

    public native double getTextureFadeDuration();

    /**
     * offto: give tile textures mipmaps (GLES: glGenerateMipmap at texture creation).
     */
    public native void setUsesMipmaps(boolean use);

    public native boolean getUsesMipmaps();

    /**
     * offto: images one frame fetch carries (1-2). With 2, a single frame's images fill texture
     * slots 0 and 1 (e.g. day + night from one fetch).
     */
    public native void setImagesPerTile(int num);

    public native int getImagesPerTile();
}
