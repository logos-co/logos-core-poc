---
title: Interacting with the Library
description: How clients communicate with liblogos at runtime.
---

# Interacting with liblogos

The Logos Core library (`liblogos_core`) exposes a small C API for initializing the core, managing plugins, invoking plugin methods asynchronously, and subscribing to plugin events.

## C API

Include header: `logos-liblogos/src/logos_core.h`

Lifecycle

```c
void logos_core_init(int argc, char *argv[]);
void logos_core_set_plugins_dir(const char* plugins_dir);
void logos_core_start();
int  logos_core_exec();                // Run Qt event loop (blocking)
void logos_core_cleanup();
```

Descriptions

- `logos_core_init` — Creates the internal `QCoreApplication` and prepares the core. Call this first (you can pass your process `argc/argv` or `0,NULL`).
- `logos_core_set_plugins_dir` — Overrides the directory scanned for modules before `start`. If not set, defaults to `../modules` relative to the binary.
- `logos_core_start` — Initializes the Qt Remote Objects registry, discovers and processes plugins in the plugins directory, and bootstraps internal modules (e.g., core manager). Does not block.
- `logos_core_exec` — Enters the Qt event loop and blocks until quit. Use either this or periodic `logos_core_process_events()`—not both.
- `logos_core_cleanup` — Stops all plugin processes, tears down internal state and the registry host, and destroys the application instance.

Plugin management

```c
char** logos_core_get_loaded_plugins(); // Null-terminated array; caller frees
char** logos_core_get_known_plugins();  // Null-terminated array; caller frees
int    logos_core_load_plugin(const char* plugin_name);   // 1 success, 0 fail
int    logos_core_unload_plugin(const char* plugin_name); // 1 success, 0 fail
char*  logos_core_process_plugin(const char* plugin_path); // Returns plugin name or NULL; caller frees
char*  logos_core_get_token(const char* key); // Experimental; caller frees
```

Descriptions

- `logos_core_get_loaded_plugins` — Returns names of currently running plugins. The result is a null-terminated `char**` allocated with `new[]`; delete each entry and the array with `delete[]`.
- `logos_core_get_known_plugins` — Returns names discovered/processed in the plugins directory, whether loaded or not. Same allocation/ownership as above.
- `logos_core_load_plugin` — Starts the plugin process by name (must be in known plugins). Returns `1` on success, `0` on failure.
- `logos_core_unload_plugin` — Terminates the plugin process and removes it from the loaded list. Returns `1` on success, `0` on failure.
- `logos_core_process_plugin` — Parses a plugin file at `plugin_path`, adds it to the known list, and returns the discovered plugin name, or `NULL` on error. The returned `char*` is allocated with `new[]`; free with `delete[]`.
- `logos_core_get_token` — Experimental helper to fetch a token by key from the core token manager. Returns a `char*` allocated with `new[]` or `NULL` if not found; free with `delete[]`.

Async operations and events

```c
typedef void (*AsyncCallback)(int result, const char* message, void* user_data);

void logos_core_async_operation(const char* data,
                                AsyncCallback cb,
                                void* user_data);

// params_json: JSON array of {name, value, type}
void logos_core_call_plugin_method_async(const char* plugin_name,
                                         const char* method_name,
                                         const char* params_json,
                                         AsyncCallback cb,
                                         void* user_data);

void logos_core_register_event_listener(const char* plugin_name,
                                        const char* event_name,
                                        AsyncCallback cb,
                                        void* user_data);

// Non-blocking event pump (useful if you manage your own loop)
void logos_core_process_events();
```

Descriptions

- `AsyncCallback` — Callback signature used by all async operations. `result` is `1` (success) or `0` (error). `message` contains the payload or error text. Treat `message` as read‑only and copy it if you need it after the callback returns.
- `logos_core_async_operation` — Simple test/utility async op; invokes the callback after a short delay with a success message. Useful for wiring checks.
- `logos_core_call_plugin_method_async` — Invokes a method on a loaded plugin. `params_json` is a JSON array of `{name, value, type}`. Supported `type` values: `string`, `int|integer`, `bool|boolean`, `double|float` (unknown types default to string). On completion, calls the callback with result or error text.
- `logos_core_register_event_listener` — Subscribes to a plugin event. When the event fires, the callback receives a JSON string like `{"event":"<name>","data":[...]}"` in `message`.
- `logos_core_process_events` — Processes Qt events without blocking; call this regularly if you are not using `logos_core_exec()`.

Notes

- The async callbacks receive `(result, message, user_data)`. For method calls, `message` is either the result payload or an error description. For events, `message` is a JSON string like `{ "event": "name", "data": [...] }`.
- Use `logos_core_exec()` for a blocking Qt event loop, or call `logos_core_process_events()` periodically from your own loop.
- Memory ownership: arrays and strings returned by the API are allocated with `new[]`. Free with `delete[]` (delete each string, then the array).

## Example (C)

Minimal C program that initializes the core, loads plugins, subscribes to an event, and calls a plugin method asynchronously.

```c
#include <stdio.h>
#include <string.h>
#include "logos_core.h"

static void on_async(int result, const char* message, void* user_data) {
    const char* tag = (const char*)user_data;
    printf("[%s] result=%d message=%s\n", tag ? tag : "cb", result, message ? message : "");
}

int main() {
    // 1) Init and start core
    logos_core_init(0, NULL);
    logos_core_set_plugins_dir("./build/plugins"); // Adjust to your plugins output dir
    logos_core_start();

    // 2) Discover and load plugins
    // If you have plugin files, you can process them directly:
    // logos_core_process_plugin("./build/plugins/chat.plugin");
    // Otherwise, load by name if the core already knows them:
    if (!logos_core_load_plugin("waku_module")) {
        printf("Failed to load waku_module\n");
    }
    if (!logos_core_load_plugin("chat")) {
        printf("Failed to load chat\n");
    }

    // 3) Register an event listener (e.g., chatMessage from chat plugin)
    logos_core_register_event_listener("chat", "chatMessage", on_async, (void*)"event");

    // 4) Call a plugin method asynchronously
    // Example: joinChannel(channelName: string)
    const char* params = "[{\"name\":\"channelName\",\"value\":\"general\",\"type\":\"string\"}]";
    logos_core_call_plugin_method_async("chat", "joinChannel", params, on_async, (void*)"call");

    // 5) Pump events for a bit (or call logos_core_exec() to block)
    for (int i = 0; i < 200; i++) {
        logos_core_process_events();
        // Simple sleep; replace with your loop's timing
        #ifdef _WIN32
            Sleep(50);
        #else
            usleep(50 * 1000);
        #endif
    }

    // 6) Cleanup
    logos_core_cleanup();
    return 0;
}
```

Build notes

- Link against the shared library: `-llogos_core` (Linux), `-llogos_core` with proper `-L` path (macOS), or `logos_core.lib` (Windows).
- Ensure the runtime loader can find the library (`LD_LIBRARY_PATH`, `DYLD_LIBRARY_PATH`, or system path settings).

## Example (JavaScript via FFI)

You can use Node.js FFI to call the C API directly. The snippet below uses `ffi-napi` and `ref-napi`.

Install dependencies

```bash
npm install ffi-napi ref-napi
```

Bind and call the library

```javascript
const ffi = require('ffi-napi');
const ref = require('ref-napi');

// Resolve the library file for your OS
// Linux:   ./build/liblogos_core.so
// macOS:   ./build/liblogos_core.dylib
// Windows: .\\build\\logos_core.dll
const libPath = process.env.LOGOS_CORE_LIB || './build/liblogos_core.dylib';

// Define the callback type: void (*)(int, const char*, void*)
const AsyncCallback = ffi.Function('void', ['int', 'string', 'void*']);

const logos = ffi.Library(libPath, {
  // lifecycle
  logos_core_init: ['void', ['int', 'pointer']],
  logos_core_set_plugins_dir: ['void', ['string']],
  logos_core_start: ['void', []],
  logos_core_cleanup: ['void', []],

  // async ops
  logos_core_call_plugin_method_async: ['void', ['string', 'string', 'string', AsyncCallback, 'void*']],
  logos_core_register_event_listener: ['void', ['string', 'string', AsyncCallback, 'void*']],
  logos_core_process_events: ['void', []],
});

// Create callbacks
const onEvent = ffi.Callback('void', ['int', 'string', 'void*'], (result, message, userData) => {
  console.log('[event]', { result, message });
});

const onCall = ffi.Callback('void', ['int', 'string', 'void*'], (result, message, userData) => {
  console.log('[call]', { result, message });
});

// Init and start core
logos.logos_core_init(0, ref.NULL);
logos.logos_core_set_plugins_dir('./build/plugins');
logos.logos_core_start();

// Register event listener and call a method
logos.logos_core_register_event_listener('chat', 'chatMessage', onEvent, ref.NULL);

const params = JSON.stringify([
  { name: 'channelName', value: 'general', type: 'string' },
]);
logos.logos_core_call_plugin_method_async('chat', 'joinChannel', params, onCall, ref.NULL);

// Pump events periodically (Qt event loop integration)
const timer = setInterval(() => {
  logos.logos_core_process_events();
}, 50);

// Later, when shutting down
process.on('SIGINT', () => {
  clearInterval(timer);
  logos.logos_core_cleanup();
  process.exit(0);
});
```

Tips

- If you prefer a higher-level interface, see the JavaScript SDK at `website/docs/sdks/javascript-sdk.md`, which wraps these FFI calls and adds a reflective API for plugin methods and events.
- Always call `logos_core_process_events()` on an interval (or use the SDK’s helper) so callbacks and events are delivered.
