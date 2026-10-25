# Changelog — offto fork

Newest first. Upstream history before the fork lives in git (`v3.5` tag).

## 2026-10-25 (renderer: draw groups)

### Fixed
- `SceneRendererMTL::updateWorkGroups` rebuilt every non-empty render-target container's indirect
  command buffers on every frame (its skip test ignored `modified`). Now only modified containers
  are rebuilt; drawables moving out of a container, `setMaster` / `setInstMaster` /
  `setInstanceData`, render-target add/remove and resize mark them. Rollback:
  `SceneRenderer::setRebuildOnlyModifiedWorkGroups(false)` /
  `MaplyBaseViewController.renderOnlyChangedDrawGroups = NO`.

## 2026-10-25 (Offto tile plan, Phase 4)

### Added
- `QuadImageFrameLoader::setImagesPerTile` / `MaplyQuadImageLoaderBase.imagesPerTile` (1-2,
  default 1): one frame may carry two images (e.g. day + night from one fetch), shown in texture
  slots 0 and 1. Slot choice extracted into `QIFTileTextures` (host-tested); two-frame loaders
  bind exactly as before.

## 2026-10-25 (Offto tile plan, Phase 3)

### Added
- Tile crossfade: `MaplyQuadImageFrameLoader.textureFadeDuration` (0 = off). A tile fades in
  when its own texture replaces the stretched ancestor it showed (not on reloads, not at the
  min level). `BasicDrawableInstance::setFade`; `FadeChangeRequest` now reaches instances.
- Mipmaps: `MaplyQuadImageLoaderBase.generateMipmaps` (default NO). RGBA8 and 565 tiles get a
  CPU-built mip chain (`DownsampleRGBA8`, box filter). The default tile shaders sample with
  `mip_filter::linear` (no change for textures without mips).
- `MaplyQuadImageLoaderBase.prepareTexturesOnWorkers` (default NO): texture conversion and mip
  building run on the parse workers (`QuadImageFrameLoader::prepareLoadReturn`,
  `Texture::prepareData`); the sampling thread only uploads.
- Tile look-ahead: `WhirlyGlobeViewController.tileLookAheadTime` (0 = off). The sampler also
  loads, at half importance, what will be visible where the camera is heading (`GlobeMotion`).
- Stats: requests, cancels, merge outcomes, wait/parse time, latency histogram.
- Host tests: `GlobeMotionTests`, `MipmapRGBA8Tests`, crossfade cases in `QIFDrawStateCacheTests`.

### Changed
- Tile selection computes each node's importance once per evaluation, and caches each tile's
  `DisplaySolid` (the sampling thread spent 28 % rebuilding them).
- Metal: argument encoders are shared per (shader function, buffer index) instead of made per
  drawable.
- GLES: mipmapped textures use `GL_LINEAR_MIPMAP_LINEAR` (was `GL_NEAREST_MIPMAP_LINEAR`).

### Fixed (Codex review)
- `DownsampleRGBA8` dropped the last row/column of odd-sized levels; it now averages them into
  the last output pixel (even sizes unchanged).
- `DisplaySolid` cache could hold 4097 entries; the cap is now a hard 4096.

## 2026-10-25 (Offto tile plan, Phase 2)

### Added
- `QuadImageFrameLoader::Stats::pipeline`: cumulative counters for tile merges, render-state
  builds, scene updates and their change requests, and the last update's fallback tiles.
- `QIFDrawStateCache`: remembers what each tile drawable last received.
- Host tests: `VertexAttributeTests`, `QIFDrawStateCacheTests`.

### Changed
- `QIFRenderState::updateScene` sends only the textures, on/off and uniforms that changed
  (same end state). A value is only remembered once the drawable exists in the scene.
- `BasicDrawableInstance::setUniforms` does nothing when the values are equal (no re-encode).

## 2026-10-25 (Offto tile plan, Phase 1)

### Added
- `MaplySamplingParams.residentBaseLevel`: keep a whole coarse level resident (on- and
  off-screen) as the sampler's min level, above level 1 too.
- `-[MaplyQuadLoaderBase getZoomLevel]`: the level the sampler is loading, with its fraction.
- `MaplyImageLoaderInterpreter` declares `dataForTile:loader:` so subclasses can override it.

### Fixed
- `QuadTreeNew::calcCoverageVisible`: `levelLoads` no longer reaches below the min level.

## 2026-10-25 (Phase 0)

### Added
- `SamplingParams.viewUpdatePeriod` (iOS: `MaplySamplingParams.viewUpdatePeriod`): how often the
  quad tree re-evaluates tiles while the view moves. 0 = engine default (0.1 s).
- Host tests for the shared core: `common/WhirlyGlobeLib/test/run_tests.sh`.
- `PATCHES.md`: every divergence from v3.5 and how each app consumes this branch.

### Fixed
- `QuadTreeNew::evalNodeVisible`: off-screen or back-facing ancestors no longer hide min-level
  tiles, so `minImportanceTop = 0` keeps the whole min level resident.
- `ImageTile_iOS` leaked a texture built by `preprocessTexture()` when the tile was cancelled
  before merge.
- `SphericalEarthChunkManager` (stickers) now hands off the texture it takes from an image tile
  (`clearTexture()`), so the tile no longer frees it a second time.

### Changed
- Android: link with a 16 KB max page size (Google Play requirement).
