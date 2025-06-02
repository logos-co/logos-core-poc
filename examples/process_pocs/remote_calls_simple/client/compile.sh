#!/bin/bash

# Compile and run script for TCP Client
echo "Building TCP Client application..."

# Create build directory if it doesn't exist
if [ ! -d "build" ]; then
    mkdir build
fi

cd build

# Configure with CMake
cmake ..

# Build the application
make

if [ $? -eq 0 ]; then
    echo "Build successful! Running the client..."
    echo ""
    ./client
else
    echo "Build failed!"
    exit 1
fi 