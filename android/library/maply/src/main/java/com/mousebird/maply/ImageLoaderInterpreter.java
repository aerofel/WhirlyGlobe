/*  ImageLoaderInterpreter.java
 *  WhirlyGlobeLib
 *
 *  Created by jmnavarro on 3/20/19.
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

package com.mousebird.maply;

import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.os.Build;

import androidx.annotation.NonNull;

import java.lang.reflect.Field;

/**
 *  Image loader interpreter turns data objects into ImageTiles.
 *
 *  This is the default interpreter used by the QuadImageLoader.
 */
public class ImageLoaderInterpreter implements LoaderInterpreter
{
    // Set if we can use the premultiply option for images
    boolean hasPremultiplyOption = false;

    // Turn this off we need the loader to not mess with values in the channels
    // This happens for data loading
    public boolean usePremultiply = false;

    public ImageLoaderInterpreter()
    {
        // See if the premultiplied option is available
        try {
            Object opts = new BitmapFactory.Options();
            Class<?> theClass = opts.getClass();
            Field field = theClass.getField("inPremultiplied");
            int version = Build.VERSION.SDK_INT;
            if (field != null && version >= 23) {
                hasPremultiplyOption = true;
            }
        }
        catch (Exception x)
        {
            // Premultiply is missing
        }
    }

    public void setLoader(QuadLoaderBase loader)
    {
    }

    // Convert byte arrays into images
    // offto: each worker thread decodes into its own bitmap, over and over. ImageTile copies the
    // pixels out (ImageTile_Android::setBitmap), so the bitmap is free again once addBitmap
    // returns. Saves a 256 KB allocation per tile: on the S20 the allocator's lock took about a
    // third of a parse worker's time in a pinch; decode benchmark 4 500 -> 6 100-10 700 tiles/s
    // on 8 threads, identical pixels.
    private static final ThreadLocal<Bitmap> reusableBitmap = new ThreadLocal<>();

    private static Bitmap decode(byte[] image, BitmapFactory.Options options)
    {
        options.inBitmap = reusableBitmap.get();
        Bitmap bm;
        try {
            bm = BitmapFactory.decodeByteArray(image, 0, image.length, options);
        } catch (IllegalArgumentException e) {
            // The image doesn't fit the reusable bitmap (larger or another format)
            options.inBitmap = null;
            bm = BitmapFactory.decodeByteArray(image, 0, image.length, options);
        }
        if (bm != null)
            reusableBitmap.set(bm);
        return bm;
    }

    public void dataForTile(LoaderReturn inLoadReturn,QuadLoaderBase loader)
    {
        ImageLoaderReturn loadReturn = (ImageLoaderReturn)inLoadReturn;

        BitmapFactory.Options options = new BitmapFactory.Options();
        //options.inScaled = false;
        if (hasPremultiplyOption && usePremultiply)
            options.inPremultiplied = false;
        options.inMutable = true;

        byte[][] images = loadReturn.getTileData();
        for (byte[] image : images) {
            if (loadReturn.isCanceled()) {
                return;
            }
            Bitmap bm = decode(image, options);
            if (bm != null)
                loadReturn.addBitmap(bm);
            else
                loadReturn.errorString = "Failed to decode bitmap";
        }
    }

    /**
     * Some tiles were just removed.
     */
    @Override
    public void tilesUnloaded(@NonNull TileID[] ids) {
    }
}
