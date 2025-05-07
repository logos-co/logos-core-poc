#!/bin/bash

# build_app_plugins.sh
# Script to build plugins from logos_app/logos_dapps/ and place them in appropriate directories:
# - main_ui goes to logos_app/app/build/plugins/
# - all other plugins go to logos_app/app/build/packages/

set -e  # Exit on error

echo "Building app plugins..."

# Make sure the plugins output directory exists
mkdir -p logos_app/logos_dapps/build/plugins

# List of plugins to build
plugins=("counter" "colorchanger" "waku_ui" "main_ui" "chat_ui")

# Build each plugin individually
for plugin_name in "${plugins[@]}"; do
    plugin_dir="logos_app/logos_dapps/$plugin_name"

    if [ ! -d "$plugin_dir" ]; then
        echo "Warning: Plugin directory $plugin_dir does not exist. Skipping."
        continue
    fi

    echo "Building plugin: $plugin_name"

    # Create build directory for this plugin
    mkdir -p "$plugin_dir/build"

    # Build the plugin
    (
        cd "$plugin_dir/build"
        cmake .. -DCMAKE_BUILD_TYPE=Release
        make -j$(sysctl -n hw.ncpu)
    )
done

# Clean up existing plugins if any
rm -f logos_app/logos_dapps/build/plugins/*.dylib logos_app/logos_dapps/build/plugins/*.so

# Create the packages directory if it doesn't exist
mkdir -p logos_app/app/build/packages

# Copy the built plugins to the central plugins directory
echo "Copying plugins to appropriate directories..."

# Find all built plugin libraries and copy them
if [[ "$(uname)" == "Darwin" ]]; then
    # macOS: .dylib files
    for plugin_name in "${plugins[@]}"; do
        plugin_dir="logos_app/logos_dapps/$plugin_name"
        if [ -d "$plugin_dir/build" ]; then
            find "$plugin_dir/build" -name "*.dylib" -type f | while read plugin; do
                # Get the filename
                filename=$(basename "$plugin")

                # Copy main_ui to modules directory, others to packages directory
                if [ "$plugin_name" == "main_ui" ]; then
                    echo "Copying $filename to logos_app/app/build/plugins/"
                    cp "$plugin" logos_app/app/build/plugins/
                    # Also copy to logos_app/logos_dapps/build/plugins/ for compatibility
                    cp "$plugin" logos_app/logos_dapps/build/plugins/
                else
                    echo "Copying $filename to logos_app/app/build/packages/"
                    cp "$plugin" logos_app/app/build/packages/
                    # Also copy to logos_app/logos_dapps/build/plugins/ for compatibility
                    cp "$plugin" logos_app/logos_dapps/build/plugins/
                fi
            done
        fi
    done

    # Set correct permissions
    find logos_app/app/build/plugins -name "*.dylib" -type f -exec chmod 755 {} \;
    find logos_app/app/build/packages -name "*.dylib" -type f -exec chmod 755 {} \;
    find logos_app/logos_dapps/build/plugins -name "*.dylib" -type f -exec chmod 755 {} \;
else
    # Linux/other: .so files
    for plugin_name in "${plugins[@]}"; do
        plugin_dir="logos_app/logos_dapps/$plugin_name"
        if [ -d "$plugin_dir/build" ]; then
            find "$plugin_dir/build" -name "*.so" -type f | while read plugin; do
                # Get the filename
                filename=$(basename "$plugin")

                # Copy main_ui to modules directory, others to packages directory
                if [ "$plugin_name" == "main_ui" ]; then
                    echo "Copying $filename to logos_app/app/build/plugins/"
                    cp "$plugin" logos_app/app/build/plugins/
                    # Also copy to logos_app/logos_dapps/build/plugins/ for compatibility
                    cp "$plugin" logos_app/logos_dapps/build/plugins/
                else
                    echo "Copying $filename to logos_app/app/build/packages/"
                    cp "$plugin" logos_app/app/build/packages/
                    # Also copy to logos_app/logos_dapps/build/plugins/ for compatibility
                    cp "$plugin" logos_app/logos_dapps/build/plugins/
                fi
            done
        fi
    done

    # Set correct permissions
    find logos_app/app/build/plugins -name "*.so" -type f -exec chmod 755 {} \;
    find logos_app/app/build/packages -name "*.so" -type f -exec chmod 755 {} \;
    find logos_app/logos_dapps/build/plugins -name "*.so" -type f -exec chmod 755 {} \;
fi

echo "App plugins built successfully!"
echo "main_ui plugin is available in: logos_app/app/build/plugins/"
echo "Other plugins are available in: logos_app/app/build/packages/"
echo "All plugins are also available in: logos_app/logos_dapps/build/plugins/ for compatibility" 