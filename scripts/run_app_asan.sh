#!/bin/bash

# Exit on error
set -e

echo "Building core..."
./scripts/run_core.sh build

echo "Building and running Logos Core POC application with Address Sanitizer..."

# Create asan build directory if it doesn't exist
mkdir -p logos_app/app/build

# Navigate to asan build directory
cd logos_app/app/build

# Run CMake with ASan flags
echo "Running CMake with Address Sanitizer configuration..."
cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_FLAGS="-g -O0 -fsanitize=address -fno-omit-frame-pointer" ..

# Build the application
echo "Building application with Address Sanitizer..."
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
    cd logos_app/app/build_asan
fi

# Configure Address Sanitizer behavior
export ASAN_OPTIONS=detect_leaks=1:detect_stack_use_after_return=1:check_initialization_order=1:strict_init_order=1:detect_invalid_pointer_pairs=2

# Run the application
echo "Starting application with Address Sanitizer..."
./LogosApp

echo "Application closed."
echo ""
echo "Address Sanitizer will automatically report memory issues during runtime."
echo "Check the console output for any detected leaks or memory errors." 