#!/bin/sh
# Host tests for the shared C++ core (offto fork). No renderer, no CMake needed.
# Usage: common/WhirlyGlobeLib/test/run_tests.sh
set -e
here="$(cd "$(dirname "$0")" && pwd)"
lib="$here/.."
out="${TMPDIR:-/tmp}/wg_core_tests"
c++ -std=c++17 -O1 -w -x c++ \
    -I"$lib/include" -I"$lib/../local_libs/eigen" \
    "$here/QuadTreeNewTests.cpp" "$lib/src/QuadTreeNew.cpp" "$lib/src/WhirlyVector.cpp" \
    -o "$out"
"$out"
