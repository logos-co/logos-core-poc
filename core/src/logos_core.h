#ifndef LOGOS_CORE_H
#define LOGOS_CORE_H

// Define export macro for the library
#if defined(LOGOS_CORE_LIBRARY)
#  define LOGOS_CORE_EXPORT __attribute__((visibility("default")))
#else
#  define LOGOS_CORE_EXPORT
#endif

#ifdef __cplusplus
extern "C" {
#endif

// Initialize the logos core library
LOGOS_CORE_EXPORT void logos_core_init(int argc, char *argv[]);

// Set a custom plugins directory
LOGOS_CORE_EXPORT void logos_core_set_plugins_dir(const char* plugins_dir);

// Start the logos core functionality
LOGOS_CORE_EXPORT void logos_core_start();

// Run the event loop
LOGOS_CORE_EXPORT int logos_core_exec();

// Clean up resources
LOGOS_CORE_EXPORT void logos_core_cleanup();

// Get the list of loaded plugins
// Returns a null-terminated array of plugin names that must be freed by the caller
LOGOS_CORE_EXPORT char** logos_core_get_loaded_plugins();

// Get the list of known plugins
// Returns a null-terminated array of plugin names that must be freed by the caller
LOGOS_CORE_EXPORT char** logos_core_get_known_plugins();

// Load a specific plugin by name
// Returns 1 if successful, 0 if failed
LOGOS_CORE_EXPORT int logos_core_load_plugin(const char* plugin_name);

// Unload a specific plugin by name
// Returns 1 if successful, 0 if failed
LOGOS_CORE_EXPORT int logos_core_unload_plugin(const char* plugin_name);

// Process a plugin file and add it to known plugins
// Returns the plugin name if successful, NULL if failed
LOGOS_CORE_EXPORT char* logos_core_process_plugin(const char* plugin_path);

// Get a token by key from the core token manager
// Returns the token value if found, NULL if not found
// The returned string must be freed by the caller
LOGOS_CORE_EXPORT char* logos_core_get_token(const char* key);

// === Async Callback API ===

// Define the callback function type for async operations
typedef void (*AsyncCallback)(int result, const char* message, void* user_data);

// Simple async operation example that uses a callback
LOGOS_CORE_EXPORT void logos_core_async_operation(const char* data, AsyncCallback callback, void* user_data);

// Async plugin loading with callback
LOGOS_CORE_EXPORT void logos_core_load_plugin_async(const char* plugin_name, AsyncCallback callback, void* user_data);

// Proxy method to call plugin methods remotely with async callback
// params_json: JSON string containing array of {name, value, type} objects
LOGOS_CORE_EXPORT void logos_core_call_plugin_method_async(
    const char* plugin_name, 
    const char* method_name, 
    const char* params_json, 
    AsyncCallback callback, 
    void* user_data
);

// Register an event listener for a specific event from a specific plugin
// The callback will be triggered whenever the specified event is emitted by the plugin
LOGOS_CORE_EXPORT void logos_core_register_event_listener(
    const char* plugin_name,
    const char* event_name, 
    AsyncCallback callback,
    void* user_data
);

// Process Qt events without blocking (for integration with other event loops)
LOGOS_CORE_EXPORT void logos_core_process_events();

#ifdef __cplusplus
}
#endif

#endif // LOGOS_CORE_H 