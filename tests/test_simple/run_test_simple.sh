#!/bin/bash

# Exit on error
set -e

echo "Building and running Test Simple application..."

# Get the script directory (where this script is located)
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

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
if command -v nproc >/dev/null 2>&1; then
    JOBS=$(nproc)
elif command -v sysctl >/dev/null 2>&1; then
    JOBS=$(sysctl -n hw.ncpu)
else
    JOBS=2
fi
make -j$JOBS
echo "Core build completed!"

# Build template_module
echo "Building template_module..."
cd ../../modules
if [ -d "build" ]; then
    echo "Cleaning existing modules build..."
    rm -rf build
fi
mkdir -p build
cd build
cmake ..
make -j$JOBS template_module_plugin
echo "Template module build completed!"

# Navigate to the tests directory
cd ../../tests

# Create and enter build directory
mkdir -p build
cd build

# Configure with CMake
echo "Configuring with CMake..."
cmake ..

# Build the application
echo "Building test_simple application..."
if command -v nproc >/dev/null 2>&1; then
    JOBS=$(nproc)
elif command -v sysctl >/dev/null 2>&1; then
    JOBS=$(sysctl -n hw.ncpu)
else
    JOBS=2
fi

make -j$JOBS

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

# Copy the template_module plugin from the modules build directory
TEMPLATE_MODULE_SOURCE="../../modules/build/modules/template_module_plugin.$LIB_EXT"
TEMPLATE_MODULE_DEST="bin/modules/template_module_plugin.$LIB_EXT"

if [ -f "$TEMPLATE_MODULE_SOURCE" ]; then
    echo "Copying template_module plugin..."
    cp "$TEMPLATE_MODULE_SOURCE" "$TEMPLATE_MODULE_DEST"
    echo "Successfully copied template_module plugin to: $TEMPLATE_MODULE_DEST"
else
    echo "Warning: template_module plugin not found at: $TEMPLATE_MODULE_SOURCE"
    echo "You may need to build the modules first by running:"
    echo "  cd ../../modules && mkdir -p build && cd build && cmake .. && make"
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
echo "Running Test Simple application..."
echo "Press Ctrl+C to stop the application."
echo "=================================="
cd bin
./test_simple 