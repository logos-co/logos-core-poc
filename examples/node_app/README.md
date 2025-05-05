# Logos Core - Node.js Example

This example demonstrates how to use the `liblogos_core` library from Node.js using the FFI (Foreign Function Interface) approach.

## Prerequisites

- Node.js (v14 or newer recommended)
- npm
- Built liblogos_core library (must be built first)

## Setup

1. Make sure you've built the Logos Core library first:
   ```
   cd ../../
   ./run_core.sh build
   ```

2. Install the Node.js dependencies:
   ```
   npm install
   ```

## Running the Example

To run the example:

```
npm start
```

## How it Works

This example uses [ffi-napi](https://github.com/node-ffi-napi/node-ffi-napi) to create bindings to the C functions exported by the `liblogos_core` library. It demonstrates:

1. Loading the library
2. Initializing the Logos Core
3. Setting the plugins directory
4. Starting the Logos Core
5. Cleaning up resources

## Troubleshooting

If you encounter an error like "Library not found", make sure:
- You've built the Logos Core library successfully
- The path to the library is correct (currently set to `../../core/build/lib/liblogos_core.[so|dylib|dll]`)

For other FFI-related issues, you might need to install additional dependencies:
- On Ubuntu/Debian: `sudo apt-get install libffi-dev`
- On macOS: The required libraries should be included with macOS
- On Windows: Make sure you have the appropriate Visual Studio build tools installed 