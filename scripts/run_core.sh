#!/bin/bash

# Exit on error
set -e

echo "Building and running Logos Core application..."

# First, build the core
echo "Building core application..."
cd core

# remove build to ensure clean build
if [ -d "build" ]; then
    echo "Removing old core build directory..."
    rm -rf build
fi

# Create build directory if it doesn't exist
mkdir -p build

# Navigate to build directory
cd build

# Run CMake
echo "Running CMake..."
cmake ..

# Build the application and core library
echo "Building core library and application..."
cmake --build .

# Create plugins directory in build
echo "Creating plugins directory..."
mkdir -p plugins

# Go back to the root directory
cd ../../

# Check if this is a build-only run
if [ "$1" = "build" ]; then
    echo "Core build completed. Exiting without running application."
    exit 0
fi

# Note: Module building has been moved to scripts/build_core_modules.sh
# Make sure to run scripts/build_core_modules.sh before this script if you need updated modules

# Check if the "all" argument was provided
if [ "$1" = "all" ]; then
    echo "Building modules first..."
    ./scripts/build_core_modules.sh
fi

# Run the application
echo "Starting logoscore application..."
cd core/build/bin
./logoscore

echo "Application closed." 