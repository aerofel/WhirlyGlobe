# Changelog — offto fork

Newest first. Upstream history before the fork lives in git (`v3.5` tag).

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
