#!/bin/bash

# Exit on error
set -e

# Check if QT_DIR environment variable is set
if [ -z "$QT_DIR" ]; then
  echo "Error: QT_DIR environment variable is not set."
  echo "Please set QT_DIR to your Qt installation directory."
  echo "Example: export QT_DIR=/path/to/your/qt/installation"
  exit 1
fi

echo "Using Qt installation at: $QT_DIR"

# Create build directory if it doesn't exist
mkdir -p build
cd build

# Configure using CMake
echo "Configuring client with CMake..."
cmake .. -DCMAKE_PREFIX_PATH="$QT_DIR"

# Build
echo "Building..."
cmake --build . --config Release

echo "Client compilation complete!"
echo "Client executable: $(pwd)/counter_client"

# Return to the original directory
cd .. 