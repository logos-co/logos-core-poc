#!/bin/bash

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/../.." && pwd)"
BUILD_DIR="$REPO_ROOT/build/cpp-generator"
SRC_DIR="$REPO_ROOT/logos-cpp-sdk/cpp-generator"

BUILD_TYPE="Release"
CMAKE_PREFIX_ARG=""

for arg in "$@"; do
    case "$arg" in
        clean)
            echo "Cleaning build directory: $BUILD_DIR"
            if [ -d "$BUILD_DIR" ]; then
                rm -rf "$BUILD_DIR"
            fi
            echo "Clean complete."
            exit 0
            ;;
        --debug)
            BUILD_TYPE="Debug"
            ;;
        --release)
            BUILD_TYPE="Release"
            ;;
        --prefix)
            shift || true
            if [ "${1-}" != "" ]; then
                CMAKE_PREFIX_ARG="-DCMAKE_PREFIX_PATH=$1"
                shift || true
            fi
            ;;
        *)
            ;;
    esac
done

if [ -n "${QT_DIR-}" ]; then
    CMAKE_PREFIX_ARG="-DCMAKE_PREFIX_PATH=$QT_DIR"
    echo "Using QT_DIR as CMAKE_PREFIX_PATH: $QT_DIR"
fi

echo "Configuring (type=$BUILD_TYPE) ..."
cmake -S "$SRC_DIR" -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE="$BUILD_TYPE" ${CMAKE_PREFIX_ARG}

echo "Building logos-cpp-generator ..."
cmake --build "$BUILD_DIR" --target logos-cpp-generator -j

BIN_PATH="$BUILD_DIR/bin/logos-cpp-generator"
if [ -f "$BIN_PATH" ]; then
    echo "Build succeeded: $BIN_PATH"
else
    echo "Build finished, but binary not found at expected path: $BIN_PATH"
    exit 1
fi
