#!/bin/bash
set -e

# Create and enter build directory
mkdir -p build
cd build

# Configure with CMake
echo "Configuring with CMake..."
cmake ..

# Build the application
echo "Building application..."
make -j$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 2)

# Create plugins directory
mkdir -p bin/plugins

# Set up library paths
# For macOS, use DYLD_LIBRARY_PATH
if [[ "$(uname)" == "Darwin" ]]; then
    echo "Setting up library paths for macOS..."
    export DYLD_LIBRARY_PATH="$DYLD_LIBRARY_PATH:$(pwd)/../../core/build/lib"
    echo "DYLD_LIBRARY_PATH=$DYLD_LIBRARY_PATH"
    LIB_EXT="dylib"
else
    # For Linux, use LD_LIBRARY_PATH
    echo "Setting up library paths for Linux..."
    export LD_LIBRARY_PATH="$LD_LIBRARY_PATH:$(pwd)/../../core/build/lib"
    echo "LD_LIBRARY_PATH=$LD_LIBRARY_PATH"
    LIB_EXT="so"
fi

# Manually copy specific modules as a fallback
echo "Copying specific modules..."
MODULES=("chat_plugin" "libwaku" "waku_plugin")
CORE_MODULES_DIR="../../core/build/modules"

for PLUGIN in "${PLUGINS[@]}"; do
    # Try with the platform-specific extension first
    if [ -f "$CORE_PLUGINS_DIR/$PLUGIN.$LIB_EXT" ]; then
        echo "Copying $PLUGIN.$LIB_EXT"
        cp "$CORE_MODULES_DIR/$PLUGIN.$LIB_EXT" "bin/modules/"
    # Try with alternative extension
    elif [ "$LIB_EXT" = "dylib" ] && [ -f "$CORE_MODULES_DIR/$PLUGIN.so" ]; then
        echo "Copying $PLUGIN.so"
        cp "$CORE_MODULES_DIR/$PLUGIN.so" "bin/modules/"
    elif [ "$LIB_EXT" = "so" ] && [ -f "$CORE_MODULES_DIR/$PLUGIN.dylib" ]; then
        echo "Copying $PLUGIN.dylib"
        cp "$CORE_MODULES_DIR/$PLUGIN.dylib" "bin/modules/"
    else
        echo "Plugin $PLUGIN not found in $CORE_PLUGINS_DIR"
    fi
done

# Run the application
echo "Running Standalone Chat application..."
cd bin
./standalone_chat 