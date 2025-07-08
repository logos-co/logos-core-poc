#!/bin/bash

# Exit on error
set -e

echo "Building core..."
./scripts/run_core.sh build

echo "Building and running Logos Core POC application with debugging enabled..."

# Create debug build directory if it doesn't exist
mkdir -p logos_app/app/build

# Navigate to debug build directory
cd logos_app/app/build

# Run CMake with debug flags
echo "Running CMake with debug configuration..."
cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_FLAGS="-g -O0 -fno-omit-frame-pointer -DMALLOC_FILL_SPACE=1" ..

# Build the application
echo "Building application in debug mode..."
make

# Create the plugins directory if it doesn't exist
echo "Setting up plugins directory..."
mkdir -p plugins
mkdir -p bin/modules

# Check if the "all" argument was provided
if [ "$1" = "all" ]; then
    cd ../../..
    echo "Building modules first..."
    ./scripts/build_core_modules.sh
    ./scripts/build_app_plugins.sh
    cd logos_app/app/build
fi

# Enable MallocStackLogging for leaks detection
export MallocStackLogging=1
export MallocScribble=1
export MallocPreScribble=1
export MallocCheckHeapStart=1
export MallocCheckHeapEach=1
export DYLD_INSERT_LIBRARIES=/usr/lib/libgmalloc.dylib

# Run the application
echo "Starting application in debug mode..."
./LogosApp

echo "Application closed."
echo ""
echo "To check for leaks while the app is running, open another terminal and run:"
echo "leaks \$(pgrep -n LogosApp)"
echo ""
echo "Or to get a memory leak report after the app closes, run:"
echo "leaks -atExit -- ./LogosApp" 