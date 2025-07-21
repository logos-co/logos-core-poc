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

# Find MOC binary
if [ -f "$QT_PATH/bin/moc" ]; then
    MOC_BIN="$QT_PATH/bin/moc"
elif [ -f "$QT_PATH/libexec/moc" ]; then
    MOC_BIN="$QT_PATH/libexec/moc"
elif command -v moc >/dev/null 2>&1; then
    MOC_BIN="moc"
else
    echo "Error: MOC (Meta-Object Compiler) not found. Please ensure Qt development tools are installed."
    exit 1
fi

echo "Using MOC: $MOC_BIN"

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

# Generate MOC files for headers with Q_OBJECT
echo "Generating MOC files..."

# List of headers that need MOC processing (contain Q_OBJECT)
MOC_HEADERS=(
    "logos_api.h"
    "logos_api_client.h"
    "logos_api_provider.h" 
    "logos_api_consumer.h"
    "module_proxy.h"
    "token_manager.h"
)

for header in "${MOC_HEADERS[@]}"; do
    if [ -f "$header" ]; then
        echo "Generating MOC for $header..."
        $MOC_BIN $header -o "moc_${header%.h}.cpp"
        if [ $? -ne 0 ]; then
            echo "❌ MOC generation failed for $header"
            exit 1
        fi
    fi
done

# Try to compile the headers (syntax check)
echo "Checking header syntax..."

g++ $CXXFLAGS $QT_INCLUDES -c -x c++-header logos_api.h -o /tmp/logos_api.h.gch
if [ $? -eq 0 ]; then
    echo "✅ LogosAPI header syntax OK"
    rm -f /tmp/logos_api.h.gch
else
    echo "❌ LogosAPI header has syntax errors"
    exit 1
fi

g++ $CXXFLAGS $QT_INCLUDES -c -x c++-header logos_api_client.h -o /tmp/logos_api_client.h.gch
if [ $? -eq 0 ]; then
    echo "✅ Client header syntax OK"
    rm -f /tmp/logos_api_client.h.gch
else
    echo "❌ Client header has syntax errors"
    exit 1
fi

g++ $CXXFLAGS $QT_INCLUDES -c -x c++-header logos_api_provider.h -o /tmp/logos_api_provider.h.gch
if [ $? -eq 0 ]; then
    echo "✅ Provider header syntax OK"
    rm -f /tmp/logos_api_provider.h.gch
else
    echo "❌ Provider header has syntax errors"
    exit 1
fi

g++ $CXXFLAGS $QT_INCLUDES -c -x c++-header logos_api_consumer.h -o /tmp/logos_api_consumer.h.gch
if [ $? -eq 0 ]; then
    echo "✅ Consumer header syntax OK"
    rm -f /tmp/logos_api_consumer.h.gch
else
    echo "❌ Consumer header has syntax errors"
    exit 1
fi

g++ $CXXFLAGS $QT_INCLUDES -c -x c++-header module_proxy.h -o /tmp/module_proxy.h.gch
if [ $? -eq 0 ]; then
    echo "✅ Module proxy header syntax OK"
    rm -f /tmp/module_proxy.h.gch
else
    echo "❌ Module proxy header has syntax errors"
    exit 1
fi

g++ $CXXFLAGS $QT_INCLUDES -c -x c++-header token_manager.h -o /tmp/token_manager.h.gch
if [ $? -eq 0 ]; then
    echo "✅ Token manager header syntax OK"
    rm -f /tmp/token_manager.h.gch
else
    echo "❌ Token manager header has syntax errors"
    exit 1
fi

# Try to compile the implementations (without linking)
echo "Checking implementation syntax..."

g++ $CXXFLAGS $QT_INCLUDES -c logos_api.cpp -o /tmp/logos_api.o
if [ $? -eq 0 ]; then
    echo "✅ LogosAPI implementation compiles OK"
    rm -f /tmp/logos_api.o
else
    echo "❌ LogosAPI implementation has compilation errors"
    exit 1
fi

g++ $CXXFLAGS $QT_INCLUDES -c logos_api_client.cpp -o /tmp/logos_api_client.o
if [ $? -eq 0 ]; then
    echo "✅ Client implementation compiles OK"
    rm -f /tmp/logos_api_client.o
else
    echo "❌ Client implementation has compilation errors"
    exit 1
fi

g++ $CXXFLAGS $QT_INCLUDES -c logos_api_provider.cpp -o /tmp/logos_api_provider.o
if [ $? -eq 0 ]; then
    echo "✅ Provider implementation compiles OK"
    rm -f /tmp/logos_api_provider.o
else
    echo "❌ Provider implementation has compilation errors"
    exit 1
fi

g++ $CXXFLAGS $QT_INCLUDES -c logos_api_consumer.cpp -o /tmp/logos_api_consumer.o
if [ $? -eq 0 ]; then
    echo "✅ Consumer implementation compiles OK"
    rm -f /tmp/logos_api_consumer.o
else
    echo "❌ Consumer implementation has compilation errors"
    exit 1
fi

g++ $CXXFLAGS $QT_INCLUDES -c module_proxy.cpp -o /tmp/module_proxy.o
if [ $? -eq 0 ]; then
    echo "✅ Module proxy implementation compiles OK"
    rm -f /tmp/module_proxy.o
else
    echo "❌ Module proxy implementation has compilation errors"
    exit 1
fi

g++ $CXXFLAGS $QT_INCLUDES -c token_manager.cpp -o /tmp/token_manager.o
if [ $? -eq 0 ]; then
    echo "✅ Token manager implementation compiles OK"
    rm -f /tmp/token_manager.o
else
    echo "❌ Token manager implementation has compilation errors"
    exit 1
fi

# Clean up generated MOC files
echo "Cleaning up generated MOC files..."
for header in "${MOC_HEADERS[@]}"; do
    moc_file="moc_${header%.h}.cpp"
    if [ -f "$moc_file" ]; then
        rm -f "$moc_file"
    fi
done

echo "🎉 LogosAPI compilation test passed for all components!" 