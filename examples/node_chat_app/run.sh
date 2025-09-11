#!/usr/bin/env bash

# Exit on error
set -euo pipefail

echo "Logos Core Node.js Example Runner"
echo "================================="

# Check if we're in the right directory
if [ ! -f "package.json" ]; then
    echo "Error: This script must be run from the examples/node_app directory"
    exit 1
fi

# Check if Node.js is installed
if ! command -v node &> /dev/null; then
    echo "Error: Node.js is not installed. Please install Node.js first."
    exit 1
fi

# Check if npm is installed
if ! command -v npm &> /dev/null; then
    echo "Error: npm is not installed. Please install npm first."
    exit 1
fi

# Install dependencies if node_modules doesn't exist
if [ ! -d "node_modules" ]; then
    echo "Installing Node.js dependencies..."
    npm install
fi

# Check if the core library is built
CORE_LIB_PATH="../../core/build/lib"
if [ ! -d "$CORE_LIB_PATH" ]; then
    echo "Warning: Core library not found at $CORE_LIB_PATH"
    echo "Building core library first..."
    cd ../../
    ./scripts/run_core.sh build
    cd examples/node_app
fi

# Run the test first
echo ""
echo "Running FFI test..."
npm test

echo ""
echo "Test completed successfully!"
echo ""

# Ask user if they want to run the full example
read -p "Do you want to run the full example? (y/n): " -n 1 -r
echo
if [[ $REPLY =~ ^[Yy]$ ]]; then
    echo "Running full example..."
    npm start
else
    echo "Skipping full example. You can run it later with: npm start"
fi

echo ""
echo "Done!" 