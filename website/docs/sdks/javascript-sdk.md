---
title: JavaScript SDK
slug: /sdks/javascript-sdk
description: JavaScript SDK to interact with LogosCore and its modules
---

# JavaScript SDK

The Logos JavaScript SDK provides a convenient way to interact with the LogosCore system and its modules from Node.js applications. It wraps the native liblogos_core library using FFI (Foreign Function Interface) and offers both low-level and high-level APIs for plugin management, method calls, and event handling.

## Installation

The experimental JavaScript SDK is located at `./logos-js-sdk`. After installing the package via NPM, you can start using it immediately:

```bash
npm install logos-api
```

## Quick Start

### Basic Usage

```javascript
const LogosAPI = require('logos-api');

// Initialize the API
const logos = new LogosAPI();

// Start the core system
logos.start();

// Load required modules
logos.processAndLoadPlugins(["waku_module", "chat"]);

// Start event processing
logos.startEventProcessing();
```

### Using the Traditional API

The traditional API provides explicit method calls and event registration:

```javascript
// Register event listener
logos.registerEventListener('chat', 'chatMessage', (success, message, meta) => {
    if (success) {
        console.log("New message received:", message);
    }
});

// Call plugin method asynchronously
const result = await logos.callPluginMethodAsync(
    'chat', 
    'joinChannel', 
    JSON.stringify([{
        name: "channelName", 
        value: "baixa-chiado", 
        type: "string"
    }])
);
```

### Using the Reflective API

The SDK also supports a more ergonomic reflective API that maps plugins and their methods directly to JavaScript objects:

```javascript
// Method calls return Promises
await logos.chat.initialize();
await logos.chat.joinChannel("baixa-chiado");
await logos.chat.sendMessage("baixa-chiado", "nick", "hello!");

// Event listeners with cleaner syntax
const listenerId = logos.chat.onChatMessage((evt) => {
    console.log('Chat event received:', evt);
});
```

## Core Concepts

### Initialization Options

The LogosAPI constructor accepts several configuration options:

```javascript
const logos = new LogosAPI({
    libPath: '/custom/path/to/liblogos_core.dylib',  // Custom library path
    pluginsDir: '/custom/plugins/directory',          // Custom plugins directory
    autoInit: true                                    // Auto-initialize (default: true)
});
```

### Plugin Management

The SDK provides comprehensive plugin management capabilities:

```javascript
// Get plugin status
const status = logos.getPluginStatus();
console.log('Loaded plugins:', status.loaded);
console.log('Known plugins:', status.known);

// Process and load a single plugin
logos.processAndLoadPlugin('chat');

// Process and load multiple plugins
const results = logos.processAndLoadPlugins(['waku_module', 'chat', 'wallet_module']);
console.log('Plugin loading results:', results);

// Unload a plugin
logos.unloadPlugin('chat');
```

### Event Processing

The SDK handles Qt events automatically when you start event processing:

```javascript
// Start automatic event processing (recommended)
logos.startEventProcessing(100); // Process every 100ms

// Or manually process events
setInterval(() => {
    logos.processEvents();
}, 50);

// Stop automatic event processing
logos.stopEventProcessing();
```

## API Reference

### Constructor Options

| Option | Type | Default | Description |
|--------|------|---------|-------------|
| `libPath` | string | null | Custom path to liblogos_core library |
| `pluginsDir` | string | null | Custom plugins directory path |
| `autoInit` | boolean | true | Automatically initialize on construction |

### Core Methods

#### `start()`
Starts the LogosCore system. Must be called after initialization.

#### `cleanup()`
Cleans up resources and shuts down the system.

#### `getPluginStatus()`
Returns an object with `loaded` and `known` plugin arrays.

#### `processAndLoadPlugin(pluginName)`
Processes and loads a single plugin.

#### `processAndLoadPlugins(pluginNames)`
Processes and loads multiple plugins. Returns a results object.

### Asynchronous Operations

#### `callPluginMethodAsync(pluginName, methodName, params, callback)`
Calls a plugin method asynchronously using the traditional callback approach.

**Parameters:**
- `pluginName`: Name of the target plugin
- `methodName`: Name of the method to call
- `params`: JSON string of parameters in format `[{name, value, type}]`
- `callback`: Function called with `(success, message, meta)`

#### `registerEventListener(pluginName, eventName, callback)`
Registers an event listener for plugin events.

**Parameters:**
- `pluginName`: Name of the plugin emitting events
- `eventName`: Name of the event to listen for
- `callback`: Function called with `(success, message, meta)`

### Reflective API

The reflective API provides a more JavaScript-friendly interface:

#### Method Calls
```javascript
// Automatically converts arguments to the expected format
await logos.pluginName.methodName(arg1, arg2, ...);
```

#### Event Listeners
```javascript
// Register event listeners with camelCase naming
const listenerId = logos.pluginName.onEventName((eventData) => {
    // Handle event
});
```

### Type Inference

The reflective API automatically infers parameter types:

- `boolean` → `bool` type
- `number` (integer) → `int` type  
- `number` (float) → `double` type
- `string` → `string` type
- Objects → JSON stringified as `string` type

## Examples

### Chat Application

```javascript
const LogosAPI = require('logos-api');

async function createChatApp() {
    const logos = new LogosAPI();
    logos.start();
    
    // Load required modules
    logos.processAndLoadPlugins(["waku_module", "chat"]);
    logos.startEventProcessing();
    
    // Listen for incoming messages
    logos.chat.onChatMessage((message) => {
        console.log(`[${message.channel}] ${message.nick}: ${message.content}`);
    });
    
    // Initialize and join channel
    await logos.chat.initialize();
    await logos.chat.joinChannel("general");
    
    // Send a message
    await logos.chat.sendMessage("general", "myNick", "Hello, world!");
}

createChatApp().catch(console.error);
```

### Plugin Method with Parameters

```javascript
// Using traditional API
const params = JSON.stringify([
    { name: "channelName", value: "baixa-chiado", type: "string" },
    { name: "nickname", value: "alice", type: "string" }
]);

logos.callPluginMethodAsync('chat', 'joinChannel', params, (success, result) => {
    if (success) {
        console.log('Successfully joined channel:', result);
    }
});

// Using reflective API (equivalent)
const result = await logos.chat.joinChannel("baixa-chiado", "alice");
console.log('Successfully joined channel:', result);
```

### Error Handling

```javascript
try {
    await logos.chat.sendMessage("nonexistent-channel", "nick", "message");
} catch (error) {
    console.error('Failed to send message:', error);
}

// With traditional API
logos.callPluginMethodAsync('chat', 'sendMessage', params, (success, message) => {
    if (!success) {
        console.error('Method call failed:', message);
    }
});
```

## Troubleshooting

### Library Not Found
```
Library file not found at: /path/to/liblogos_core.dylib
```
**Solution**: Build the core library first by running `./scripts/run_core.sh build`.

### Plugin Loading Failures
```
Failed to load plugin: chat
```
**Solution**: Ensure plugins are built and available in the plugins directory. Check that the plugin was processed successfully before loading.

### Event Processing Issues
**Solution**: Make sure to call `startEventProcessing()` or manually process events with `setInterval(() => logos.processEvents(), 50)`.
