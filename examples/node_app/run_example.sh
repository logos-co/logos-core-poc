#!/bin/bash

set -e  # Exit on error

# Navigate to the script directory
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
cd "$SCRIPT_DIR"

# Check if core library exists
if [ "$(uname)" = "Darwin" ]; then
  LIB_PATH="../../core/build/lib/liblogos_core.dylib"
else
  LIB_PATH="../../core/build/lib/liblogos_core.so"
fi

if [ ! -f "$LIB_PATH" ]; then
  echo "Warning: liblogos_core library not found at $LIB_PATH"
  echo "The script will continue but may fail if the library isn't in the expected location."
fi

# Install dependencies if node_modules doesn't exist
if [ ! -d "node_modules" ]; then
  echo "Installing Node.js dependencies..."
  npm install
fi

# Run the example
echo "Running the Node.js example..."
npm start 