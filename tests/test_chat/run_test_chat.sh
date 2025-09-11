#!/usr/bin/env bash

# Exit on error
set -euo pipefail

echo "Building and running Test Chat application..."

# Get the script directory (where this script is located)
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
JOBS=$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 2)
# Navigate to the project root
cd "$SCRIPT_DIR/../.."

# Build core first
echo "Building Logos Core..."
cd core
if [ -d "build" ]; then
    echo "Cleaning existing core build..."
    rm -rf build
fi
mkdir -p build
cd build
cmake ..
make -j$JOBS
echo "Core build completed!"

# Build chat and waku modules
echo "Building chat and waku modules..."
cd ../../modules
if [ -d "build" ]; then
    echo "Cleaning existing modules build..."
    rm -rf build
fi
mkdir -p build
cd build
cmake ..
make -j$JOBS chat waku_module_plugin
echo "Chat and Waku modules build completed!"

# Navigate to the tests directory
cd ../../tests

# Create and enter build directory
mkdir -p build
cd build

# Configure with CMake
echo "Configuring with CMake..."
cmake ..

# Build the application
echo "Building test_chat application..."
make -j$JOBS test_chat

echo "Build completed successfully!"

# Determine the platform-specific library extension
if [[ "$(uname)" == "Darwin" ]]; then
    LIB_EXT="dylib"
elif [[ "$(uname)" == "Linux" ]]; then
    LIB_EXT="so"
else
    LIB_EXT="dll"  # Windows
fi

# Create modules directory in the build output
echo "Setting up modules directory..."
mkdir -p bin/modules

# Copy the chat plugin from the modules build directory
CHAT_MODULE_SOURCE="../../modules/build/modules/chat_plugin.$LIB_EXT"
CHAT_MODULE_DEST="bin/modules/chat_plugin.$LIB_EXT"

if [ -f "$CHAT_MODULE_SOURCE" ]; then
    echo "Copying chat plugin..."
    cp "$CHAT_MODULE_SOURCE" "$CHAT_MODULE_DEST"
    echo "Successfully copied chat plugin to: $CHAT_MODULE_DEST"
else
    echo "Warning: chat plugin not found at: $CHAT_MODULE_SOURCE"
    echo "You may need to build the modules first by running:"
    echo "  cd ../../modules && mkdir -p build && cd build && cmake .. && make"
fi

# Copy the waku plugin from the modules build directory
WAKU_MODULE_SOURCE="../../modules/build/modules/waku_module_plugin.$LIB_EXT"
WAKU_MODULE_DEST="bin/modules/waku_module_plugin.$LIB_EXT"

if [ -f "$WAKU_MODULE_SOURCE" ]; then
    echo "Copying waku plugin..."
    cp "$WAKU_MODULE_SOURCE" "$WAKU_MODULE_DEST"
    echo "Successfully copied waku plugin to: $WAKU_MODULE_DEST"
else
    echo "Warning: waku plugin not found at: $WAKU_MODULE_SOURCE"
    echo "You may need to build the modules first by running:"
    echo "  cd ../../modules && mkdir -p build && cd build && cmake .. && make"
fi

# Copy libwaku.so dependency for waku plugin
LIBWAKU_SOURCE="../../modules/build/modules/libwaku.so"
LIBWAKU_DEST="bin/modules/libwaku.so"

if [ -f "$LIBWAKU_SOURCE" ]; then
    echo "Copying libwaku.so dependency..."
    cp "$LIBWAKU_SOURCE" "$LIBWAKU_DEST"
    echo "Successfully copied libwaku.so to: $LIBWAKU_DEST"
else
    echo "Warning: libwaku.so not found at: $LIBWAKU_SOURCE"
fi

# Copy the logos_host executable from the core build directory
LOGOS_HOST_SOURCE="../../core/build/bin/logos_host"
LOGOS_HOST_DEST="bin/logos_host"

if [ -f "$LOGOS_HOST_SOURCE" ]; then
    echo "Copying logos_host executable..."
    cp "$LOGOS_HOST_SOURCE" "$LOGOS_HOST_DEST"
    echo "Successfully copied logos_host executable to: $LOGOS_HOST_DEST"
else
    echo "Error: logos_host executable not found at: $LOGOS_HOST_SOURCE"
    echo "This is required for plugin loading. Make sure the core build completed successfully."
    exit 1
fi

# Set up library paths for running the application
if [[ "$(uname)" == "Darwin" ]]; then
    echo "Setting up library paths for macOS..."
    export DYLD_LIBRARY_PATH="$DYLD_LIBRARY_PATH:$(pwd)/../../core/build/lib"
    echo "DYLD_LIBRARY_PATH=$DYLD_LIBRARY_PATH"
else
    echo "Setting up library paths for Linux..."
    export LD_LIBRARY_PATH="$LD_LIBRARY_PATH:$(pwd)/../../core/build/lib"
    echo "LD_LIBRARY_PATH=$LD_LIBRARY_PATH"
fi

# Run the application
echo ""
echo "Running Test Chat application..."
echo "Press Ctrl+C to stop the application."
echo "=================================="
cd bin
./test_chat 