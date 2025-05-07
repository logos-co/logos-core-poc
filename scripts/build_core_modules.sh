#!/bin/bash

# Exit on error
set -e

echo "Building Logos Core modules..."

# Get the script directory
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

# Build libwaku
echo "Building libwaku..."
"$SCRIPT_DIR/../modules/waku/build_libwaku.sh"

# Navigate to modules directory
cd modules

# Remove build to ensure clean build
if [ -d "build" ]; then
    echo "Removing old plugin build directory..."
    rm -rf build
fi

# Create build directory if it doesn't exist
mkdir -p build
cd build

# Run CMake for modules and point to the core library
echo "Running CMake for modules..."
cmake .. -DCMAKE_PREFIX_PATH=../../core/build

# Build the modules
echo "Building modules..."
cmake --build .

# Fix library paths for platform-specific dynamic libraries
if [ "$(uname)" = "Darwin" ]; then
    echo "Fixing library paths for waku_plugin.dylib on macOS..."
    # Use otool to get the current library path
    CURRENT_PATH=$(otool -L "modules/waku_plugin.dylib" | grep libwaku.so | awk '{print $1}')
    
    # Update the paths using relative references
    install_name_tool -change "${CURRENT_PATH}" "@rpath/libwaku.so" "modules/waku_plugin.dylib"
    install_name_tool -id "@rpath/libwaku.so" "modules/libwaku.so"
elif [ "$(uname)" = "Linux" ]; then
    echo "Fixing library paths for waku_plugin.so on Linux..."
    # No need for install_name_tool on Linux
    # Set rpath directly for Linux shared objects
    if [ -f "modules/waku_plugin.so" ]; then
        patchelf --set-rpath '$ORIGIN' "modules/waku_plugin.so"
    fi
fi

# Go back to the root directory
cd ../../

# Copy plugin libraries to build modules directory
echo "Copying plugin libraries to build modules directory..."

# Create modules directory in core build if it doesn't exist
mkdir -p core/build/modules
mkdir -p logos_app/app/build/bin/modules
# Create packages directory for the package manager
mkdir -p logos_app/app/build/packages

# Find all plugin libraries and copy them to the build modules directory
echo "Looking for plugin libraries in modules/build/modules directory..."
find modules/build/modules -type f \( -name "*.dylib" -o -name "*.so" -o -name "*.dll" \) | while read plugin; do
    echo "Copying plugin: $plugin"
    cp "$plugin" "core/build/modules/"
    # cp "$plugin" "logos_app/app/build/bin/modules/"
    # Also copy to packages directory for the package manager
    cp "$plugin" "logos_app/app/build/packages/"
done

# Copy package_manager plugin to the build modules directory
OS_EXT="so"
if [[ "$OSTYPE" == "darwin"* ]]; then
  OS_EXT="dylib"
elif [[ "$OSTYPE" == "msys"* || "$OSTYPE" == "win32"* ]]; then
  OS_EXT="dll"
fi

PACKAGE_MANAGER_LIB="modules/build/modules/package_manager_plugin.${OS_EXT}"
BUILD_MODULES_DIR="logos_app/app/build/bin/plugins/"

echo "Copying package_manager plugin to the build modules directory..."
cp "$PACKAGE_MANAGER_LIB" "$BUILD_MODULES_DIR"

echo "Modules built successfully." 