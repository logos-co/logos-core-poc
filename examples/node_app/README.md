# Logos Core - Node.js Example

A simple Node.js hello world example that uses the liblogos_core library.

## Prerequisites

- Node.js
- liblogos_core library (must be already built)

## Setup

1. Install dependencies:
   ```
   npm install
   ```

## Running the Example

Run the example with:

```
npm start
```

Or use the provided script:

```
./run_example.sh
```

## How it Works

This example uses ffi-napi to create bindings to the liblogos_core library. It performs these basic steps:

1. Load the liblogos_core library
2. Initialize the Logos Core
3. Set the plugins directory 
4. Start the Logos Core
5. Clean up when the application is terminated

The example assumes that the liblogos_core library has already been built and is available in the `../../core/build/lib` directory. 