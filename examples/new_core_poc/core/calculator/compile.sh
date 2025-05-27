#!/bin/bash

# Exit on error
set -e

# Set Qt6 directory
export QT_DIR=/Users/iurimatias/Qt6/6.8.2/macos

# Create build directory if it doesn't exist
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$SCRIPT_DIR/build"

echo "Building calculator application..."

# Clean build directory to ensure proper regeneration of Remote Objects files
rm -rf "$BUILD_DIR"

# Create build directory
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

# Run CMake with Qt6 path
cmake -DCMAKE_PREFIX_PATH="$QT_DIR" ..

# Build the process_host_calculator program
cmake --build . --target process_host_calculator

echo "Build completed."
echo "The executable is located at:"
echo "- $BUILD_DIR/process_host_calculator"
echo "To run the application: $BUILD_DIR/process_host_calculator --registry \"local:registry\"" 