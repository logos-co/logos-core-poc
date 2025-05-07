#!/bin/bash
set -e

echo "Building and running Waku Qt Chat application..."

# Create build directory if it doesn't exist
WAKU_BUILD_DIR="plugins/waku/qt-chat/build"
mkdir -p $WAKU_BUILD_DIR
cd $WAKU_BUILD_DIR

# Run CMake if CMakeCache.txt doesn't exist
if [ ! -f "CMakeCache.txt" ]; then
    echo "Running CMake..."
    cmake ..
fi

# Build the application
echo "Building the application..."
make -j$(nproc 2>/dev/null || sysctl -n hw.ncpu)

echo "Build completed successfully!"

# Run the application
echo "Running Waku Qt Chat..."
./WakuChat

echo "Waku Qt Chat application closed." 