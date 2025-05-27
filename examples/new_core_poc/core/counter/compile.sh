#!/bin/bash

# Exit on error
set -e

# Set Qt6 directory
export QT_DIR=/Users/iurimatias/Qt6/6.8.2/macos

# Create build directory if it doesn't exist
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PARENT_DIR="$(dirname "$SCRIPT_DIR")"
BUILD_DIR="$SCRIPT_DIR/build"

# Function to build the counter application
build_counter() {
    echo "Building counter application..."

    # Clean build directory to ensure proper regeneration of Remote Objects files
    rm -rf "$BUILD_DIR"

    # Create build directory
    mkdir -p "$BUILD_DIR"
    cd "$BUILD_DIR"

    # Run CMake with Qt6 path
    cmake -DCMAKE_PREFIX_PATH="$QT_DIR" ..

    # Build the process_host_counter program
    cmake --build . --target process_host_counter

    echo "Counter build completed."
    echo "The executable is located at:"
    echo "- $BUILD_DIR/process_host_counter"
    echo "To run the application: $BUILD_DIR/process_host_counter --guid <guid> --registry \"local:registry\""
}

# Function to build the calculator application
build_calculator() {
    echo "Building calculator application..."
    
    # Check if calculator directory exists
    CALCULATOR_DIR="$PARENT_DIR/calculator"
    if [ -d "$CALCULATOR_DIR" ]; then
        # Run the calculator's compile script
        if [ -f "$CALCULATOR_DIR/compile.sh" ]; then
            # Return to the original directory
            cd "$SCRIPT_DIR"
            echo "Running calculator build script..."
            bash "$CALCULATOR_DIR/compile.sh"
        else
            echo "Error: Calculator compile script not found at $CALCULATOR_DIR/compile.sh"
            return 1
        fi
    else
        echo "Error: Calculator directory not found at $CALCULATOR_DIR"
        return 1
    fi
}

# Check if "all" parameter is provided
if [ "$1" = "all" ]; then
    # Build both counter and calculator
    build_counter
    build_calculator
else
    # Build only counter
    build_counter
fi 