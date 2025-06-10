#!/bin/bash

set -e

# Set base directory
SCRIPT_DIR="$(dirname "$0")"
MAKE_J=${1:-2}
NWAKU_DIR="$SCRIPT_DIR/vendor/nwaku"
LIBWAKU_PATH="$SCRIPT_DIR/lib/libwaku.so"

# Check if vendor/nwaku directory exists
if [ ! -d "$NWAKU_DIR" ]; then
    echo "Error: vendor/nwaku directory not found. Run git submodule update --init --recursive"
    exit 1
fi

# Create lib directory if it doesn't exist
mkdir -p "$SCRIPT_DIR/lib"

# Check if libwaku.so already exists
if [ ! -f "$LIBWAKU_PATH" ]; then
    # Navigate to vendor/nwaku directory
    cd "$NWAKU_DIR" || exit 1

    # Build libwaku
    echo "Running make libwaku..."
    make -j${MAKE_J} libwaku

    # Copy libwaku.so to the lib directory
    cp build/libwaku.so "$LIBWAKU_PATH"

    echo "Successfully built and copied libwaku.so"
else
    echo "libwaku.so already exists, skipping build"
fi 