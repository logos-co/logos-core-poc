# Qt Remote Objects Registry Example

This is a simple example demonstrating how to use Qt Remote Objects with a Registry.

## Overview

The project consists of three applications:

1. **Registry** - Central server that manages available remote objects
2. **Source** - Server that provides a Counter object
3. **Replica** - Client that uses the Counter object remotely through the registry

## Building

```bash
# Create a build directory
mkdir build && cd build

# Configure with CMake
cmake ..

# Build
cmake --build .
```

## Running

You need to run the applications in the following order:

1. First, start the registry:
   ```bash
   ./registry/registry_app
   ```

2. Next, start the source (server):
   ```bash
   ./source/source_app
   ```

3. Finally, start the replica (client):
   ```bash
   ./replica/replica_app
   ```

## How It Works

1. The registry starts on port 45000 and waits for sources to register
2. The source connects to the registry and registers a Counter object
3. The replica connects to the registry, discovers the Counter object, and connects to it
4. The replica's UI displays the counter value and provides buttons to increment/decrement it

## Implementation Details

- The `.rep` file defines the interface for the Counter
- Qt's code generator creates the source and replica classes from the `.rep` file
- The source implements the Counter functionality
- The replica provides a proxy to the remote Counter object 