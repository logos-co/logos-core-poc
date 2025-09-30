#!/bin/bash

# clean.sh - Script to remove all build directories in the project
# This script safely removes build folders without using dangerous commands

echo "Cleaning all build directories..."

# List of known build directories to remove
directories=(
  "./build"
  "./logos_app/app/build"
  "./logos-liblogos/build"
  "./modules/build"
  "./logos_app/logos_dapps/build"
  "./examples/build"
  "./modules/waku/build"
)

# Remove each directory if it exists
for dir in "${directories[@]}"; do
  if [ -d "$dir" ]; then
    echo "Removing $dir..."
    rm -r "$dir"
  else
    echo "Directory $dir does not exist, skipping."
  fi
done

# Clean generated SDK C++ files but keep the directory
GEN_DIR="./logos-cpp-sdk/cpp/generated"
if [ -d "$GEN_DIR" ]; then
  echo "Removing generated SDK files in $GEN_DIR ..."
  find "$GEN_DIR" -type f -name "*_api.cpp" -delete
  find "$GEN_DIR" -type f -name "*_api.h" -delete
  find "$GEN_DIR" -type f -name "logos_sdk.h" -delete
  find "$GEN_DIR" -type f -name "logos_sdk.cpp" -delete
fi

# Find and remove any additional build directories that might be missed
echo "Searching for other build directories..."
for build_dir in $(find . -type d -name "build" -not -path "*/node_modules/*" -not -path "*/.git/*" -not -path "*/vendor/*"); do
  echo "Removing $build_dir..."
  rm -r "$build_dir"
done

echo "Clean completed successfully!" 