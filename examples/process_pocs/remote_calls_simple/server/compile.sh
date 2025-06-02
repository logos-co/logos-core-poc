#!/bin/bash

# Compile and run script for TCP Server
echo "Building TCP Server application..."

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
    echo "Build successful! Running the server..."
    echo ""
    ./server
else
    echo "Build failed!"
    exit 1
fi 