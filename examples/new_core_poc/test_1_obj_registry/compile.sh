#!/bin/bash

# Exit on error
set -e

# Set Qt6 directory
# export QT_DIR=/Users/iurimatias/Qt6/6.8.2/macos

# Create build directory if it doesn't exist
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$SCRIPT_DIR/build"
COUNTER_DIR="$SCRIPT_DIR/counter"
CALCULATOR_DIR="$SCRIPT_DIR/calculator"

# Function to build the main application
build_main() {
    echo "Building main application..."

    # Clean build directory to ensure proper regeneration of Remote Objects files
    rm -rf "$BUILD_DIR"

    # Create build directory
    mkdir -p "$BUILD_DIR"
    cd "$BUILD_DIR"

    # Run CMake with Qt6 path
    cmake -DCMAKE_PREFIX_PATH="$QT_DIR" ..

    # Build the main application
    cmake --build . --target hello_world

    echo "Main application build completed."
    echo "The executable is located at:"
    echo "- $BUILD_DIR/hello_world"
    echo "To run the application: $BUILD_DIR/hello_world"
}

# Function to build the counter application
build_counter() {
    echo "Building counter application..."
    if [ -d "$COUNTER_DIR" ] && [ -f "$COUNTER_DIR/compile.sh" ]; then
        cd "$COUNTER_DIR"
        bash ./compile.sh
        cd "$SCRIPT_DIR"
    else
        echo "Error: Counter directory or compile script not found"
        return 1
    fi
}

# Function to build the calculator application
build_calculator() {
    echo "Building calculator application..."
    if [ -d "$CALCULATOR_DIR" ] && [ -f "$CALCULATOR_DIR/compile.sh" ]; then
        cd "$CALCULATOR_DIR"
        bash ./compile.sh
        cd "$SCRIPT_DIR"
    else
        echo "Error: Calculator directory or compile script not found"
        return 1
    fi
}

# Check if "all" parameter is provided
if [ "$1" = "all" ]; then
    # Build all components
    build_main
    build_counter
    build_calculator
    
    echo "All applications have been built successfully."
    echo "Main application: $BUILD_DIR/hello_world"
    echo "Counter application: $COUNTER_DIR/build/process_host_counter"
    echo "Calculator application: $CALCULATOR_DIR/build/process_host_calculator"
else
    # Build only the main application
    build_main
fi 