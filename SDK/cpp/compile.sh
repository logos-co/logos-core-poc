#!/bin/bash

# Simple compilation test for LogosAPI
# This script compiles the LogosAPI files to check for syntax and compilation errors

echo "Testing LogosAPI compilation..."

# Find Qt installation
if [ -n "$QT_DIR" ]; then
    QT_PATH="$QT_DIR"
    echo "Using QT_DIR: $QT_PATH"
elif command -v qmake >/dev/null 2>&1; then
    QT_PATH=$(qmake -query QT_INSTALL_PREFIX)
    echo "Found Qt via qmake at: $QT_PATH"
else
    echo "Error: QT_DIR not set and qmake not found. Please set QT_DIR environment variable or ensure Qt is installed and in PATH."
    exit 1
fi

# Set Qt include paths - handle both Qt5 and Qt6 on different platforms
if [ -d "$QT_PATH/lib" ]; then
    # Qt6 style with lib directory (common on macOS)
    # Add framework headers and the lib directory itself for framework-style includes
    QT_INCLUDES="-F$QT_PATH/lib"
    QT_INCLUDES="$QT_INCLUDES -I$QT_PATH/lib/QtCore.framework/Headers"
    QT_INCLUDES="$QT_INCLUDES -I$QT_PATH/lib/QtRemoteObjects.framework/Headers"
    # Also add the general include paths as fallback
    QT_INCLUDES="$QT_INCLUDES -I$QT_PATH/include -I$QT_PATH/include/QtCore -I$QT_PATH/include/QtRemoteObjects"
else
    # Standard include directory structure
    QT_INCLUDES="-I$QT_PATH/include -I$QT_PATH/include/QtCore -I$QT_PATH/include/QtRemoteObjects"
fi

echo "Using Qt includes: $QT_INCLUDES"

# Compiler flags
CXXFLAGS="-std=c++17 -fPIC"

# Try to compile the header (syntax check)
echo "Checking header syntax..."
g++ $CXXFLAGS $QT_INCLUDES -c -x c++-header logos_api.h -o /tmp/logos_api.h.gch
if [ $? -eq 0 ]; then
    echo "✅ Header syntax OK"
    rm -f /tmp/logos_api.h.gch
else
    echo "❌ Header has syntax errors"
    exit 1
fi

# Try to compile the implementation (without linking)
echo "Checking implementation syntax..."
g++ $CXXFLAGS $QT_INCLUDES -c logos_api.cpp -o /tmp/logos_api.o
if [ $? -eq 0 ]; then
    echo "✅ Implementation compiles OK"
    rm -f /tmp/logos_api.o
else
    echo "❌ Implementation has compilation errors"
    exit 1
fi

echo "🎉 LogosAPI compilation test passed!" 