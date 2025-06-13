const ffi = require('ffi-napi');
const ref = require('ref-napi');
const path = require('path');

// Simple hello world example that uses liblogos_core
console.log('Hello from Node.js!');

// Assuming the .so/.dylib is already built and available
const libExtension = process.platform === 'darwin' ? '.dylib' : '.so';
const libPath = path.resolve(__dirname, '../../core/build/lib', `liblogos_core${libExtension}`);

console.log(`Looking for library at: ${libPath}`);

// Check if the library file exists
const fs = require('fs');
if (!fs.existsSync(libPath)) {
    console.error(`Library file not found at: ${libPath}`);
    console.error('Please build the core library first by running: ./scripts/run_core.sh build');
    process.exit(1);
}

// Define pointer types for C string arrays
const StringArrayPtr = ref.refType(ref.types.CString);

// Define callback type for async operations
const AsyncCallback = ffi.Function('void', ['int', 'string', 'pointer']);

// Define the interface to liblogos_core based on the actual C API
const LogosCore = ffi.Library(libPath, {
  // Core initialization and lifecycle
  'logos_core_init': ['void', ['int', 'pointer']],
  'logos_core_set_plugins_dir': ['void', ['string']],
  'logos_core_start': ['void', []],
  'logos_core_exec': ['int', []],
  'logos_core_cleanup': ['void', []],
  
  // Plugin management
  'logos_core_get_loaded_plugins': [StringArrayPtr, []],
  'logos_core_get_known_plugins': [StringArrayPtr, []],
  'logos_core_load_plugin': ['int', ['string']],
  'logos_core_unload_plugin': ['int', ['string']],
  'logos_core_process_plugin': ['string', ['string']],
  
  // NEW: Async callback functions
  'logos_core_async_operation': ['void', ['string', AsyncCallback, 'pointer']],
  'logos_core_load_plugin_async': ['void', ['string', AsyncCallback, 'pointer']],
  'logos_core_call_plugin_method_async': ['void', ['string', 'string', 'string', AsyncCallback, 'pointer']],
  
  // Event listener registration
  'logos_core_register_event_listener': ['void', ['string', 'string', AsyncCallback, 'pointer']],
  
  // Qt event processing (non-blocking)
  'logos_core_process_events': ['void', []]
});

// Helper function to convert C string array to JavaScript array
function convertCStringArrayToJS(cStringArray) {
    const result = [];
    if (cStringArray.isNull()) {
        return result;
    }
    
    let i = 0;
    while (true) {
        const stringPtr = cStringArray.readPointer(i * ref.sizeof.pointer);
        if (stringPtr.isNull()) {
            break;
        }
        result.push(stringPtr.readCString());
        i++;
    }
    
    return result;
}

// Helper function to print loaded plugins
function printLoadedPlugins() {
    console.log('\n--- Current Plugin Status ---');
    
    // Get loaded plugins
    const loadedPluginsPtr = LogosCore.logos_core_get_loaded_plugins();
    const loadedPlugins = convertCStringArrayToJS(loadedPluginsPtr);
    console.log(`Loaded plugins (${loadedPlugins.length}):`, loadedPlugins);
    
    // Get known plugins
    const knownPluginsPtr = LogosCore.logos_core_get_known_plugins();
    const knownPlugins = convertCStringArrayToJS(knownPluginsPtr);
    console.log(`Known plugins (${knownPlugins.length}):`, knownPlugins);
    
    console.log('--- End Plugin Status ---\n');
}

// Initialize logos_core
console.log('Initializing logos_core...');
LogosCore.logos_core_init(0, null);

// Set plugins directory
const pluginsDir = path.resolve(__dirname, '../../core/build/modules');
console.log(`Setting plugins directory to: ${pluginsDir}`);
LogosCore.logos_core_set_plugins_dir(pluginsDir);

// Start logos_core
console.log('Starting logos_core...');
LogosCore.logos_core_start();

console.log('Logos Core initialized successfully!');
console.log('Hello World from Logos Core Node.js example!');

// Print initial plugin status
printLoadedPlugins();

// Try to load some plugins
console.log('Attempting to load plugins...');

const pluginsToLoad = ['waku_module', 'chat', 'template_module'];

// first process these plugins

pluginsToLoad.forEach(pluginName => {
    console.log(`\nProcessing plugin file: ${pluginName}`);
    
    // Determine plugin extension based on platform
    let pluginExtension;
    if (process.platform === 'darwin') {
        pluginExtension = '.dylib';
    } else if (process.platform === 'win32') {
        pluginExtension = '.dll'; 
    } else {
        pluginExtension = '.so';
    }

    // Construct full plugin path
    const pluginPath = path.join(pluginsDir, `${pluginName}_plugin${pluginExtension}`);
    const result = LogosCore.logos_core_process_plugin(pluginPath);
    
    if (result) {
        console.log(`✓ Processed plugin: ${pluginName}`);
    } else {
        console.log(`✗ Failed to process plugin file: ${pluginName}`);
    }
});


pluginsToLoad.forEach(pluginName => {
    console.log(`\nLoading plugin: ${pluginName}`);
    const result = LogosCore.logos_core_load_plugin(pluginName);
    if (result === 1) {
        console.log(`✓ Successfully loaded ${pluginName} plugin`);
    } else {
        console.log(`✗ Failed to load ${pluginName} plugin`);
    }
});

// Print final plugin status
printLoadedPlugins();

// ============================================
// ASYNC CALLBACK TESTING
// ============================================

console.log('\n🚀 Testing Async Callbacks');
console.log('==========================');

// Create callback functions
const asyncOperationCallback = ffi.Callback('void', ['int', 'string', 'pointer'], 
    (result, message, userData) => {
        const timestamp = new Date().toISOString();
        if (result === 1) {
            console.log(`✅ [${timestamp}] ASYNC SUCCESS: ${message}`);
        } else {
            console.log(`❌ [${timestamp}] ASYNC FAILED: ${message}`);
        }
    }
);

const pluginLoadCallback = ffi.Callback('void', ['int', 'string', 'pointer'], 
    (result, message, userData) => {
        const timestamp = new Date().toISOString();
        if (result === 1) {
            console.log(`🔌 [${timestamp}] PLUGIN LOADED: ${message}`);
        } else {
            console.log(`💥 [${timestamp}] PLUGIN FAILED: ${message}`);
        }
    }
);

const methodCallCallback = ffi.Callback('void', ['int', 'string', 'pointer'], 
    (result, message, userData) => {
        const timestamp = new Date().toISOString();
        if (result === 1) {
            console.log(`🎯 [${timestamp}] METHOD SUCCESS: ${message}`);
            console.log(`📝 [${timestamp}] RESPONSE MESSAGE: ${message}`);
        } else {
            console.log(`🚫 [${timestamp}] METHOD FAILED: ${message}`);
        }
    }
);

const eventListenerCallback = ffi.Callback('void', ['int', 'string', 'pointer'], 
    (result, message, userData) => {
        const timestamp = new Date().toISOString();
        if (result === 1) {
            console.log(`🎉 [${timestamp}] EVENT RECEIVED: ${message}`);
            
            // Try to parse the message as JSON to see event data
            try {
                const parsedMessage = JSON.parse(message);
                console.log(`🎊 [${timestamp}] EVENT NAME: ${parsedMessage.event}`);
                console.log(`📊 [${timestamp}] EVENT DATA:`, parsedMessage.data);
            } catch (e) {
                // Message is not JSON, just print it as is
                console.log(`📝 [${timestamp}] RAW EVENT MESSAGE: ${message}`);
            }
        } else {
            console.log(`🚫 [${timestamp}] EVENT LISTENER FAILED: ${message}`);
        }
    }
);

// Test 1: Simple async operation
// console.log('\n🧪 TEST 1: Simple Async Operation');
// console.log('Calling logos_core_async_operation with "Hello Async World"...');
// LogosCore.logos_core_async_operation('Hello Async World', asyncOperationCallback, null);

// // Test 2: Multiple async operations
// console.log('\n🧪 TEST 2: Multiple Async Operations');
// ['Operation A', 'Operation B', 'Operation C'].forEach((data, index) => {
//     setTimeout(() => {
//         console.log(`Calling async operation ${index + 1}: ${data}`);
//         LogosCore.logos_core_async_operation(data, asyncOperationCallback, null);
//     }, index * 500); // Stagger the calls
// });
// 
// // Test 3: Async plugin loading
// console.log('\n🧪 TEST 3: Async Plugin Loading');
// const asyncPluginsToLoad = ['package_manager'];  // Start with just one for testing
// 
// asyncPluginsToLoad.forEach((pluginName, index) => {
//     setTimeout(() => {
//         console.log(`Attempting to load plugin asynchronously: ${pluginName}`);
//         LogosCore.logos_core_load_plugin_async(pluginName, pluginLoadCallback, null);
//     }, 3000 + (index * 1500)); // Start after 3 seconds
// });
// 
// // Test 4: Non-existent plugin (should fail)
// setTimeout(() => {
//     console.log('\n🧪 TEST 4: Loading Non-existent Plugin (should fail)');
//     console.log('Attempting to load non-existent plugin: "fake_plugin"');
//     LogosCore.logos_core_load_plugin_async('fake_plugin', pluginLoadCallback, null);
// }, 5000);
// 
// // Test 5: Proxy method calls - Simple method with no parameters
// setTimeout(() => {
//     console.log('\n🧪 TEST 5: Proxy Method Call - No Parameters');
//     console.log('Calling template_module::helloWorld() with no parameters');
//     
//     const noParams = JSON.stringify([]);
//     LogosCore.logos_core_call_plugin_method_async(
//         'template_module', 
//         'helloWorld', 
//         noParams, 
//         methodCallCallback, 
//         null
//     );
// }, 6000);

// Test 6: Register event listener for fooTriggered event
console.log('\n🧪 TEST 6: Register Event Listener');
console.log('Registering event listener for template_module::fooTriggered');
LogosCore.logos_core_register_event_listener(
    'template_module',
    'fooTriggered',
    eventListenerCallback,
    null
);

// Test 7: Proxy method calls - Method with string parameter  
// Wait a bit for the event listener to be fully set up before calling the method
setTimeout(() => {
    console.log('\n🧪 TEST 7: Proxy Method Call - String Parameter');
    console.log('Calling template_module::foo() with string parameter');

    const stringParams = JSON.stringify([
        {
            name: "bar",
            value: "Hello from Node.js!",
            type: "string"
        }
    ]);

    console.log('Scheduling method call...');
    LogosCore.logos_core_call_plugin_method_async(
        'template_module', 
        'foo', 
        stringParams, 
        methodCallCallback, 
        null
    );
}, 5000); // Wait 5 seconds for the event listener to be ready

// Keep the process alive for a while to see all callbacks
console.log('\n⏳ Waiting for async operations to complete...');
console.log('   (Method call should complete soon)');

// Keep the process alive longer to see the async callbacks
setTimeout(() => {
    console.log('\n⏱️  Still waiting for callbacks...');
}, 8000);

setTimeout(() => {
    console.log('\n🎉 Async test period completed');
    console.log('Note: Process will continue running Qt event loop. Press Ctrl+C to exit.');
}, 15000);

// Handle process termination gracefully
process.on('SIGINT', () => {
    console.log('\n🛑 Interrupted! Cleaning up...');
    LogosCore.logos_core_cleanup();
    console.log('✨ Cleanup completed!');
    process.exit(0);
});

// Run the Qt event loop (this will block)
console.log('\n🔄 Processing Qt events periodically...');
// Instead of blocking with logos_core_exec(), process Qt events periodically
// This allows Node.js and Qt event loops to coexist
setInterval(() => {
    LogosCore.logos_core_process_events();
}, 50); // Process every 50ms for responsive event handling 
