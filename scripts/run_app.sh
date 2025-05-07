#!/bin/bash

# Exit on error
set -e

echo "Building core..."
./scripts/run_core.sh build

echo "Building and running Logos Core POC application..."

# Create build directory if it doesn't exist
mkdir -p logos_app/app/build

# Navigate to build directory
cd logos_app/app/build

# Run CMake if CMakeCache.txt doesn't exist (first time build)
if [ ! -f "CMakeCache.txt" ]; then
    echo "Running CMake..."
    cmake ..
fi

# Build the application
echo "Building application..."
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

# Run the application
echo "Starting application..."
./LogosApp

echo "Application closed." 