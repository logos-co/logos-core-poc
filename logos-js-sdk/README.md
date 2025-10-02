NOTE: This is very WIP

# Logos API - JavaScript SDK

A JavaScript SDK for interacting with liblogos_core, providing a clean abstraction over FFI functionality for plugin management, async operations, and event handling.

## Basic Usage

### LogosAPI - Core Functionality

```javascript
const LogosAPI = require('logos-api');

// Initialize with default settings
const logos = new LogosAPI();

// Or with custom options
const logos = new LogosAPI({
  libPath: '/custom/path/to/liblogos_core.dylib',
  pluginsDir: '/custom/plugins/directory',
  autoInit: true
});

// Start the system
logos.start();

// Load plugins
const results = logos.processAndLoadPlugins(['waku_module', 'chat', 'template_module']);
console.log('Plugin loading results:', results);

// Check plugin status
const status = logos.getPluginStatus();
console.log('Loaded plugins:', status.loaded);
console.log('Known plugins:', status.known);

// Call a plugin method asynchronously
logos.callPluginMethodAsync('chat', 'initialize', JSON.stringify([]), (success, message, meta) => {
  if (success) {
    console.log('Chat initialized:', message);
  } else {
    console.error('Failed to initialize chat:', message);
  }
});

// Register event listener
logos.registerEventListener('chat', 'chatMessage', (success, message, meta) => {
  if (success && message.event === 'chatMessage') {
    const [timestamp, username, text] = message.data;
    console.log(`Message from ${username}: ${text}`);
  }
});

// Start event processing
logos.startEventProcessing();

// Cleanup when done
process.on('SIGINT', () => {
  logos.cleanup();
  process.exit(0);
});
```

## API Reference

### LogosAPI

#### Constructor Options

```javascript
const options = {
  libPath: string,           // Custom path to liblogos_core library
  pluginsDir: string,        // Custom plugins directory
  autoInit: boolean          // Auto-initialize on construction (default: true)
};
```

#### Core Methods

- `init()` - Initialize the library
- `start()` - Start the LogosCore system
- `cleanup()` - Clean up and shutdown

#### Plugin Management

- `getLoadedPlugins()` - Get array of loaded plugin names
- `getKnownPlugins()` - Get array of known plugin names
- `getPluginStatus()` - Get object with loaded and known plugins
- `processPlugin(pluginName)` - Process a plugin file
- `loadPlugin(pluginName)` - Load a plugin
- `unloadPlugin(pluginName)` - Unload a plugin
- `processAndLoadPlugin(pluginName)` - Process and load in one step
- `processAndLoadPlugins(pluginNames[])` - Process and load multiple plugins

#### Async Operations

- `callPluginMethodAsync(pluginName, methodName, params, callback)` - Call plugin method
- `registerEventListener(pluginName, eventName, callback)` - Register event listener

#### Event Processing

- `startEventProcessing(interval)` - Start event processing loop
- `stopEventProcessing()` - Stop event processing loop
- `exec()` - Execute blocking event loop


## Requirements

- Node.js 18+
- liblogos_core built and available
- ffi-napi and ref-napi dependencies

## Building liblogos_core

Before using this SDK, ensure liblogos_core is built:

```bash
./scripts/run_core.sh build
```

The library should be available at `logos-liblogos/build/lib/liblogos_core.{dylib|so|dll}` and plugins at `logos-liblogos/build/modules/`.
