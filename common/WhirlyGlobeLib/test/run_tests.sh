#!/bin/sh
# Host tests for the shared C++ core (offto fork). No renderer, no CMake needed.
# Usage: common/WhirlyGlobeLib/test/run_tests.sh
set -e
here="$(cd "$(dirname "$0")" && pwd)"
lib="$here/.."
out="${TMPDIR:-/tmp}/wg_core_tests"
flags="-std=c++17 -O1 -w -x c++ -I$lib/include -I$lib/../local_libs/eigen"
c++ $flags "$here/QuadTreeNewTests.cpp" "$lib/src/QuadTreeNew.cpp" "$lib/src/WhirlyVector.cpp" -o "$out"
"$out"
c++ $flags "$here/VertexAttributeTests.cpp" "$lib/src/VertexAttribute.cpp" -o "$out-va"
"$out-va"
c++ $flags "$here/QIFDrawStateCacheTests.cpp" "$lib/src/QIFDrawStateCache.cpp" -o "$out-ds"
"$out-ds"
c++ $flags "$here/GlobeMotionTests.cpp" "$lib/src/GlobeMotion.cpp" -o "$out-gm"
"$out-gm"
