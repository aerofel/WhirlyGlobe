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
