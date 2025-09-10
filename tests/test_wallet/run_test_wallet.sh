#!/bin/bash

set -euo pipefail

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
ROOT_DIR="$SCRIPT_DIR/../.."

# Ensure Go 1.23 in PATH as requested
export PATH=/opt/homebrew/opt/go@1.23/bin:$PATH

# Prefer Homebrew Qt6 (arm64) over any x86 Qt5
export CMAKE_PREFIX_PATH="/opt/homebrew/opt/qt/lib/cmake:${CMAKE_PREFIX_PATH:-}"

# Build core
echo "Building Logos Core..."
cd "$ROOT_DIR/core"
rm -rf build
mkdir -p build && cd build
cmake -DCMAKE_BUILD_TYPE=Debug ..
if command -v sysctl >/dev/null 2>&1; then JOBS=$(sysctl -n hw.ncpu); else JOBS=4; fi
make -j"$JOBS"

# Build wallet C lib and plugin
echo "Building wallet C library and plugin..."
cd "$ROOT_DIR/modules/wallet_module"
./build_wallet_lib.sh

cd "$ROOT_DIR/modules"
rm -rf build
mkdir -p build && cd build
cmake -DCMAKE_BUILD_TYPE=Debug ..
make -j"$JOBS" wallet_module_plugin

# Configure and build only the test_wallet target
echo "Building test_wallet..."
cd "$ROOT_DIR/tests"
mkdir -p build && cd build
cmake -DCMAKE_BUILD_TYPE=Debug ..
make -j"$JOBS" test_wallet

# Prepare runtime modules directory
echo "Setting up modules directory for test runtime..."
mkdir -p bin/modules

# Determine platform-specific extension
if [[ "$(uname)" == "Darwin" ]]; then
  LIB_EXT="dylib"
else
  LIB_EXT="so"
fi

# Copy wallet plugin and its dependency
cp "$ROOT_DIR/modules/build/modules/wallet_module_plugin.$LIB_EXT" bin/modules/
if [[ -f "$ROOT_DIR/modules/build/modules/libgowalletsdk.$LIB_EXT" ]]; then
  cp "$ROOT_DIR/modules/build/modules/libgowalletsdk.$LIB_EXT" bin/modules/
fi

# Copy logos_host for plugin loading
cp "$ROOT_DIR/core/build/bin/logos_host" bin/

# Setup library path for core runtime
if [[ "$(uname)" == "Darwin" ]]; then
  export DYLD_LIBRARY_PATH="${DYLD_LIBRARY_PATH:-}:$ROOT_DIR/core/build/lib"
else
  export LD_LIBRARY_PATH="${LD_LIBRARY_PATH:-}:$ROOT_DIR/core/build/lib"
fi

echo "Running test_wallet..."
cd bin
./test_wallet


