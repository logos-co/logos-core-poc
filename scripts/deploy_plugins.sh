#!/bin/bash
set -e

# Ensure we have a clean target directory
mkdir -p core/build/modules
rm -f core/build/modules/*

# Copy all plugin libraries and their dependencies
echo "Copying plugins to core/build/modules..."
cp modules/build/plugins/*.dylib core/build/modules/
cp modules/build/plugins/*.so core/build/modules/ 2>/dev/null || true

# Verify all plugins have their dependencies available
echo "Verifying plugin dependencies..."
for plugin in core/build/modules/*.dylib; do
    echo "Checking $plugin..."
    otool -L "$plugin" | grep -v "@loader_path" | grep "/Users" || true
done

echo "Deployment complete. Plugins are ready in core/build/modules/"
echo "You can now run: ./core/build/bin/logoscore" 