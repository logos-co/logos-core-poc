#!/bin/bash

# Set the base directory to the script's location
BASE_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
cd "$BASE_DIR"

echo "Building the Logos Core library..."

# Create build directory for core if it doesn't exist
mkdir -p core/build

# Navigate to the core build directory
cd core/build

# Run CMake to configure the core project
cmake ..

# Build the core project
make -j4

# Check if the build was successful
if [ $? -ne 0 ]; then
    echo "Core build failed!"
    exit 1
fi

# Return to the base directory
cd "$BASE_DIR"

echo "Building the Custom Application..."

# Create build directory for examples if it doesn't exist
mkdir -p examples/build

# Navigate to the examples build directory
cd examples/build

# Run CMake to configure the examples project
cmake ..

# Build the examples project
make -j4

# Check if the build was successful
if [ $? -ne 0 ]; then
    echo "Custom app build failed!"
    exit 1
fi

# Set the library path to include the lib directory
export LD_LIBRARY_PATH="$BASE_DIR/core/build/lib:$LD_LIBRARY_PATH"
export DYLD_LIBRARY_PATH="$BASE_DIR/core/build/lib:$DYLD_LIBRARY_PATH"

# Create plugins directory in the examples build directory
echo "Creating plugins directory and copying plugin files..."
mkdir -p bin/plugins

# Check if the core build plugins directory exists
if [ -d "$BASE_DIR/core/build/plugins" ]; then
    # Copy plugins from the core build directory
    cp -v "$BASE_DIR/core/build/plugins"/* bin/modules/ 2>/dev/null || true
elif [ -d "$BASE_DIR/modules/build/modules" ]; then
    # Copy plugins from the plugins build directory
    cp -v "$BASE_DIR/modules/build/modules"/* bin/modules/ 2>/dev/null || true
else
    echo "Warning: Could not find plugins directory. The application may not work correctly."
fi

# List the copied plugins
echo "Plugins in bin/plugins directory:"
ls -la bin/modules/

# Run the custom application
echo "Running custom application..."
./bin/custom_app

# Return to the base directory
cd "$BASE_DIR"

# Copy plugin libraries to build modules directory
echo "Copying plugin libraries to build modules directory..."

# Create modules directory in core build if it doesn't exist
mkdir -p core/build/modules
mkdir -p logos_app/app/build/bin/modules
# Create packages directory for the package manager
mkdir -p logos_app/app/build/packages 