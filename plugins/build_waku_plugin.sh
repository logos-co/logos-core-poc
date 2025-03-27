#!/bin/bash
set -e

# Change to the directory of this script
cd "$(dirname "$0")"

# Create build directory if it doesn't exist
mkdir -p build
cd build

# Run CMake
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build .

# Make sure libwaku.so is in the plugins directory
if [ -f plugins/libwaku.so ]; then
    echo "libwaku.so is in place"
else
    echo "Copying libwaku.so to plugins directory"
    cp ../waku/lib/libwaku.so plugins/
fi

# Verify library dependencies
echo "Checking library dependencies..."
if [ "$(uname)" == "Darwin" ]; then
    otool -L plugins/libwaku.dylib
    
    # Fix any remaining absolute paths if needed
    if otool -L plugins/libwaku.dylib | grep -v "@loader_path" | grep "/Users"; then
        echo "Found absolute paths, fixing..."
        PATHS_TO_FIX=$(otool -L plugins/libwaku.dylib | grep -v "@loader_path" | grep "/Users" | awk '{print $1}')
        for path in $PATHS_TO_FIX; do
            FILENAME=$(basename "$path")
            echo "Fixing path for $FILENAME"
            install_name_tool -change "$path" "@loader_path/$FILENAME" plugins/libwaku.dylib
        done
    fi
else
    ldd plugins/libwaku.so
fi

echo "Build and packaging complete. The plugin and its dependencies are in $(pwd)/plugins/" 