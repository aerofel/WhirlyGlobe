# Offto fork of WhirlyGlobe-Maply

Branch `offto` of `aerofel/WhirlyGlobe`. Base: upstream **v3.5** (2022-04-08), which is what both
Offto apps shipped with. Upstream is not actively maintained (see its README), so fixes live here.

One library, two apps:
- **iOS** (`~/Sites/Swift/Offto`) builds a synced copy of `common/` + `ios/` as a CocoaPods
  development pod under `External/WhirlyGlobe` (see "iOS consumption" below).
- **Android** (`~/Sites/Kotlin/Offto`) uses `app/libs/maply-{debug,release}.aar` built from this
  branch (see "Android consumption").

Ownership: the iOS Offto instance edits `common/` and `ios/`. The Android Offto instance edits
`android/` and rebuilds the AARs from the handoffs in `~/Sites/Kotlin/Offto/docs/handoffs/`.

## Divergences from v3.5

| Commit | Area | What | Why / who needs it |
|---|---|---|---|
| 07a710bf6 | android | Force performance wide vectors off; JNI cleanup | upstream main after v3.5 |
| 5a2166e08 | android | Link with 16 KB max page size | Google Play requirement (Android) |
| a522ca778 | common + ios | `SamplingParams.viewUpdatePeriod` (+ `MaplySamplingParams.viewUpdatePeriod`); `RawData.h` includes `<functional>` | iOS uses 0.02 s to request tiles sooner while the globe moves. Android Java/JNI setter: **not yet** (handoff) |
| 5c036a69d | common | `QuadTreeNew::evalNodeVisible`: the importance-0 early return applies only below the min level | With `minZoom > 0` and `minImportanceTop = 0`, the whole min level stays resident (off-screen and back-facing too). Before, off-screen z1/z2 ancestors hid their min-level children |
| 4741fd4f6 | common + ios | `ImageTile_iOS` deletes a texture it built but never handed off; `SphericalEarthChunkManager` calls `clearTexture()` after taking one | `preprocessTexture()` on a cancelled tile leaked; stickers would double-free with a deleting destructor (latent on Android) |
| c5b23741e | common | `QuadTreeNew::calcCoverageVisible`: `levelLoads` entries never resolve below the min level | With the target at the min level, `[-1]` loaded the whole level below it |
| 20a1c0583 | ios | `-[MaplyQuadLoaderBase getZoomLevel]` | The level (with fraction) the sampler is loading; Offto logs it to tune level selection |
| fef2808ae | ios | `MaplyImageLoaderInterpreter` declares `dataForTile:loader:` | So Swift subclasses can override it and call super (Offto decodes tiles on the parse workers) |
| 3ddfb19cd | ios | `MaplySamplingParams.residentBaseLevel` | `minZoom` silently refuses > 1 and `setMinImportance:` overwrites a 0 `minImportanceTop`; this sets the base level, `forceMinLevel` and the 0 threshold together. Android Java/JNI: **not yet** (handoff) |
| 041971fad | common + ios | `QuadImageFrameLoader::Stats::pipeline` counters (merges, render-state builds, scene updates with their change requests and time, fallback tiles and worst level gap); `MaplyQuadImageFrameLoaderStats` fields | Measure where time goes (Offto `BasemapPerfProbe`). Java/JNI: **not yet** (handoff) |
| 42851addb | common | `BasicDrawableInstance::setUniforms` skips equal values (`SameAttributeValues`) | Every render-state update re-encoded every tile drawable (on Metal a new argument buffer): 76 % of main-thread time while tiles loaded |
| 13fbae9cb | common | `QIFRenderState::updateScene` sends only what changed (`QIFDrawStateCache`) | ~3 400 change requests per update (466 000 in a 10 s pan) → 1 800 for the whole pan. New `QIFDrawStateCache.cpp` (in the Android CMake list) |
| f3ea48c86, 6ce458d88 | common + ios | Stats: requests, cancels (incl. removal via `QIFFrameAsset::clear`), merges OK/cancelled/failed, wait and parse time, fetch→merge latency histogram | Where a tile's time goes (Offto perf protocol). Java/JNI: **not yet** (handoff) |
| 8795519f7 | common | `QuadTreeNew` evaluates each node's importance once per pass; `QuadSamplingController` caches `DisplaySolid`s (cap 4096) | Sampling thread −45 % on a pinch (iPad): selection did importance twice and rebuilt solids every pass |
| 23384b761 | ios | Metal argument encoders cached per (function, buffer index) | `newArgumentEncoder` was 23 % of the sampling thread |
| 527776acb | common + ios | Tile look-ahead (`GlobeMotion`, `GlobeView::makeLookAheadViewState`, `QuadDisplayControllerNew` adds ahead nodes at half importance); `WhirlyGlobeViewController.tileLookAheadTime` | Off in Offto: measured worse on an iPad M4 (+20 % requests). Kept for slower devices. Android glue: **not yet** (handoff) |
| 9df0dc02f | common + ios | Tile crossfade (`BasicDrawableInstance::setFade`, `FadeChangeRequest` on instances, `QIFDrawStateCache` emits the fade, GLES ramp); `MaplyQuadImageFrameLoader.textureFadeDuration` | Tiles fade in instead of popping. A shader that multiplies rgb by the fade dips ~25 % mid-fade; Offto's fades alpha only. Java setter: **not yet** (handoff) |
| af649ab8b | common + ios | Mipmaps: `DownsampleRGBA8`, `TextureMTL` uploads CPU-built levels, `QuadImageFrameLoader::setUsesMipmaps`, `MaplyQuadImageLoaderBase.generateMipmaps`; default tile shaders `mip_filter::linear`; GLES trilinear | Minified tiles (limb, tilted views, night lights) shimmered. +⅓ texture memory. Java setter: **not yet** (handoff) |
| fa6a825c3 | common + ios | `Texture::prepareData`, `QuadImageFrameLoader::prepareLoadReturn` (called on the parse worker), `MaplyQuadImageLoaderBase.prepareTexturesOnWorkers` | Building mips on the sampling thread cost ~25 % of tiles under 50 ms in a pinch; on the workers most of it comes back. Android: its own decision (its conversion and upload are serial) |

Contract to keep: whoever takes a texture from `ImageTile::buildTexture()` and hands it to an
`AddTextureReq` must call `clearTexture()`. An `ImageTile` deletes any texture it still holds.

## Tests

`common/WhirlyGlobeLib/test/run_tests.sh` builds and runs host tests of the shared core with plain
`c++` (no CMake, no renderer). Run it after any change under `common/`.

## iOS consumption

- `tools/sync-whirlyglobe.sh` in the iOS repo copies `common/`, `ios/library/`,
  `WhirlyGlobe.podspec` and `LICENSE` from a given commit of this branch into
  `External/WhirlyGlobe/`, and writes the commit to `External/WhirlyGlobe/OFFTO_FORK_SHA`.
- Its Podfile uses `pod 'WhirlyGlobe', :path => 'External/WhirlyGlobe'`. Run `pod install` after
  a sync, then commit `External/`, `Podfile.lock` and `Pods/`. Xcode Cloud does not run
  `pod install`; it builds the committed `Pods/`.
- Never point the Podfile back at upstream, and never run `pod update WhirlyGlobe`.
- GLES-only core files must be named `*GLES.cpp` / `*GLES.h`: the podspec excludes them.

## Android consumption

Build with the pinned legacy toolchain (JDK 11, NDK 21.3.6528147, CMake 3.10.2, Gradle 7.0.2):

```
cd android/apps/AutoTesterAndroid
JAVA_HOME=<jdk11> ANDROID_HOME=~/Library/Android/sdk ./gradlew :maply:assembleDebug :maply:assembleRelease
```

Copy `android/library/maply/build/outputs/aar/maply-{debug,release}.aar` to the app's `app/libs/`
and record this branch's commit next to them.
