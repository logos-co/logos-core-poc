---
title: Nim SDK
description: Using the Logos Nim bindings to integrate with the core.
---

# Nim SDK

The Nim SDK provides a thin wrapper over the experimental C API exposed by `liblogos_core`, enabling Nim applications to initialize the core, load modules, invoke plugin methods, and subscribe to events using a simple, callback-based API. It offers an interface for Nim developers to interact with the LogosCore system.

## Installation

The Nim SDK is located at `logos-nim-sdk/logos_api.nim`. To use it in your Nim application, simply import the module:

```nim
import logos_api
```

## Quick Start

### Basic Usage

```nim
import os, strformat
import logos_api

# Initialize the API
var api = newLogosAPI(autoInit = true)

# Start the core system
discard api.start()

# Load required modules
discard api.processAndLoadPlugins(["waku_module", "chat"])

# Start event processing (manual pumping required)
while true:
  api.processEventsTick()
  sleep(50)
```

### Using the Plugin API

The SDK provides a plugin-based API for method calls and event registration:

```nim
# Register event listener
api.plugin("chat").on("chatMessage") do (success: bool, message: string):
  if success:
    echo "New message received: ", message

# Call plugin method asynchronously
api.plugin("chat").call("joinChannel", "baixa-chiado") do (success: bool, message: string):
  if success:
    echo "Successfully joined channel: ", message
```

## Core Concepts

### Initialization Options

The LogosAPI constructor accepts several configuration options:

```nim
var api = newLogosAPI(
  libPath = "/custom/path/to/liblogos_core.so",  # Custom library path
  pluginsDir = "/custom/plugins/directory",       # Custom plugins directory
  autoInit = true                                 # Auto-initialize (default: true)
)
```

### Plugin Management

The SDK provides comprehensive plugin management capabilities:

```nim
# Get plugin status
let status = api.getPluginStatus()
echo "Loaded plugins: ", status.loaded
echo "Known plugins: ", status.known

# Process and load a single plugin
discard api.processPlugin("chat")
discard api.loadPlugin("chat")

# Process and load multiple plugins
let results = api.processAndLoadPlugins(["waku_module", "chat", "wallet_module"])
for r in results:
  echo &"Plugin {r.name}: processed={r.processed}, loaded={r.loaded}"

# Unload a plugin
discard api.unloadPlugin("chat")
```

### Event Processing

The SDK requires manual event processing since Nim applications don't run the Qt event loop automatically:

```nim
# Manual event processing (required for Nim)
while true:
  api.processEventsTick()
  sleep(50)  # Process every 50ms

# Or integrate with your application's event loop
proc myAppEventLoop() =
  while appRunning:
    handleAppEvents()      # Your app's events
    api.processEventsTick() # Logos events
    sleep(16)              # ~60 FPS
```

## API Reference

### Constructor Options

| Option | Type | Default | Description |
|--------|------|---------|-------------|
| `libPath` | string | "" | Custom path to liblogos_core library |
| `pluginsDir` | string | "" | Custom plugins directory path |
| `autoInit` | bool | true | Automatically initialize on construction |

### Core Methods

#### `start(): bool`
Starts the LogosCore system. Must be called after initialization.

#### `cleanup()`
Cleans up resources and shuts down the system.

#### `getPluginStatus(): tuple[loaded: seq[string], known: seq[string]]`
Returns a tuple with `loaded` and `known` plugin arrays.

#### `processAndLoadPlugin(pluginName: string): bool`
Processes and loads a single plugin.

#### `processAndLoadPlugins(pluginNames: openArray[string]): seq[tuple[name: string, processed: bool, loaded: bool]]`
Processes and loads multiple plugins. Returns a results sequence.

### Asynchronous Operations

#### `callPluginMethodAsync(pluginName, methodName, paramsJson: string, callback: LogosCallback)`
Calls a plugin method asynchronously using the traditional callback approach.

**Parameters:**
- `pluginName`: Name of the target plugin
- `methodName`: Name of the method to call
- `paramsJson`: JSON string of parameters in format `[{name, value, type}]`
- `callback`: Function called with `(success: bool, message: string)`

#### `registerEventListener(pluginName, eventName: string, callback: LogosCallback)`
Registers an event listener for plugin events.

**Parameters:**
- `pluginName`: Name of the plugin emitting events
- `eventName`: Name of the event to listen for
- `callback`: Function called with `(success: bool, message: string)`

### Plugin API

The plugin API provides a more ergonomic interface:

#### Method Calls
```nim
# Automatically handles parameter conversion
api.plugin("pluginName").call("methodName", "singleParam", callback)
api.plugin("pluginName").call("methodName", ["param1", "param2"], callback)
```

#### Event Listeners
```nim
# Register event listeners with clean syntax
api.plugin("pluginName").on("eventName") do (success: bool, message: string):
  # Handle event
```

### Type Inference

The plugin API automatically infers parameter types and converts them to the expected JSON format:

- Single string values → wrapped as `[{name: "arg0", value: "...", type: "string"}]`
- Multiple string values → wrapped as `[{name: "arg0", ...}, {name: "arg1", ...}]`
- JSON arrays → passed through directly

## Examples

### Chat Application

```nim
import os, strformat
import logos_api

proc createChatApp() =
  var api = newLogosAPI(autoInit = true)
  discard api.start()
  
  # Load required modules
  discard api.processAndLoadPlugins(["waku_module", "chat"])
  
  # Listen for incoming messages
  api.plugin("chat").on("chatMessage") do (success: bool, message: string):
    if success:
      echo &"[chat] New message: {message}"
  
  # Initialize and join channel
  api.plugin("chat").call("initialize", "[]") do (success: bool, message: string):
    echo &"Chat initialized: {success}"
  
  # Allow time for initialization
  for i in 0..20:
    api.processEventsTick()
    sleep(50)
  
  api.plugin("chat").call("joinChannel", "general") do (success: bool, message: string):
    echo &"Joined channel: {success}"
  
  # Send a message
  api.plugin("chat").call("sendMessage", ["general", "myNick", "Hello, world!"]) do (success: bool, message: string):
    echo &"Message sent: {success}"
  
  # Keep the event loop running
  while true:
    api.processEventsTick()
    sleep(50)

when isMainModule:
  createChatApp()
```

### Plugin Method with Parameters

```nim
# Using traditional API with explicit JSON
let params = toJson([
  Param(name: "channelName", value: "baixa-chiado", ptype: "string"),
  Param(name: "nickname", value: "alice", ptype: "string")
])

api.callPluginMethodAsync("chat", "joinChannel", params) do (success: bool, message: string):
  if success:
    echo "Successfully joined channel: ", message

# Using plugin API (equivalent)
api.plugin("chat").call("joinChannel", ["baixa-chiado", "alice"]) do (success: bool, message: string):
  if success:
    echo "Successfully joined channel: ", message
```

### Error Handling

```nim
# Handle method call errors
api.plugin("chat").call("sendMessage", "nonexistent-channel") do (success: bool, message: string):
  if not success:
    echo "Failed to send message: ", message

# Check plugin loading results
let results = api.processAndLoadPlugins(["chat", "nonexistent_plugin"])
for r in results:
  if not r.processed:
    echo &"Failed to process plugin: {r.name}"
  elif not r.loaded:
    echo &"Failed to load plugin: {r.name}"
```

## Troubleshooting

### Library Not Found
```
Failed to load library: /path/to/liblogos_core.so
```
**Solution**: Build the core library first by running `./scripts/run_core.sh build`.

### Plugin Loading Failures
```
plugin=chat processed=false loaded=false
```
**Solution**: Ensure plugins are built and available in the plugins directory. Check that the plugin was processed successfully before loading.

### Events Not Firing
**Solution**: Make sure you're calling `processEventsTick()` regularly in your main loop. This is critical for Nim applications since they don't run the Qt event loop automatically.
