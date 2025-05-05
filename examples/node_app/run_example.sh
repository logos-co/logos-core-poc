#!/bin/bash

set -e  # Exit on error

# Navigate to the script directory
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
cd "$SCRIPT_DIR"

# Check if core is built
if [ ! -d "../../core/build" ]; then
  echo "Error: Logos Core library needs to be built first"
  echo "Please run: cd ../../ && ./run_core.sh build"
  exit 1
fi

# Install dependencies if node_modules doesn't exist
if [ ! -d "node_modules" ]; then
  echo "Installing Node.js dependencies..."
  npm install
fi

# Run the example
echo "Running the Node.js example..."
npm start 