const ffi = require('ffi-napi');
const ref = require('ref-napi');
const path = require('path');

// Chat application using liblogos_core
console.log('🚀 Chat App Starting...');

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
  
  // Async callback functions
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

// Print initial plugin status
printLoadedPlugins();

// Process and load required plugins for chat
console.log('🔌 Loading required plugins for chat...');

const requiredPlugins = ['waku_module', 'chat'];

// First process the plugin files
requiredPlugins.forEach(pluginName => {
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

// Then load the plugins
requiredPlugins.forEach(pluginName => {
    console.log(`\nLoading plugin: ${pluginName}`);
    const result = LogosCore.logos_core_load_plugin(pluginName);
    if (result === 1) {
        console.log(`✓ Successfully loaded ${pluginName} plugin`);
    } else {
        console.log(`✗ Failed to load ${pluginName} plugin`);
        console.error(`CRITICAL: Failed to load ${pluginName} plugin`);
        LogosCore.logos_core_cleanup();
        process.exit(1);
    }
});

// Print final plugin status
printLoadedPlugins();

// ============================================
// CHAT FUNCTIONALITY
// ============================================

console.log('\n💬 Setting up Chat Functionality');
console.log('==================================');

// Create callback functions for chat events and method calls
const chatEventCallback = ffi.Callback('void', ['int', 'string', 'pointer'], 
    (result, message, userData) => {
        const timestamp = new Date().toISOString();
        if (result === 1) {
            console.log(`\n🎉 [${timestamp}] CHAT EVENT RECEIVED:`);
            
            // Try to parse the message as JSON to see event data
            try {
                const parsedMessage = JSON.parse(message);
                console.log(`   Event Name: ${parsedMessage.event}`);
                console.log(`   Event Data:`, parsedMessage.data);
                
                // Handle specific chat events
                if (parsedMessage.event === 'chatMessage' && parsedMessage.data && parsedMessage.data.length >= 3) {
                    console.log(`   📝 Message from ${parsedMessage.data[1]}: ${parsedMessage.data[2]}`);
                } else if (parsedMessage.event === 'historyMessage') {
                    console.log(`   📚 History message:`, parsedMessage.data);
                }
            } catch (e) {
                // Message is not JSON, just print it as is
                console.log(`   📝 RAW EVENT MESSAGE: ${message}`);
            }
        } else {
            console.log(`🚫 [${timestamp}] CHAT EVENT FAILED: ${message}`);
        }
    }
);

const methodCallCallback = ffi.Callback('void', ['int', 'string', 'pointer'], 
    (result, message, userData) => {
        const timestamp = new Date().toISOString();
        if (result === 1) {
            console.log(`✅ [${timestamp}] METHOD SUCCESS: ${message}`);
        } else {
            console.log(`❌ [${timestamp}] METHOD FAILED: ${message}`);
        }
    }
);

// Register event listeners for chat events
console.log('📡 Registering chat event listeners...');

// Listen for chat messages
console.log('Registering listener for chatMessage events...');
LogosCore.logos_core_register_event_listener(
    'chat',
    'chatMessage',
    chatEventCallback,
    null
);

// Listen for history messages
console.log('Registering listener for historyMessage events...');
LogosCore.logos_core_register_event_listener(
    'chat',
    'historyMessage',
    chatEventCallback,
    null
);

console.log('✓ Event listeners registered');

// Initialize chat module
console.log('\n🚀 Initializing chat module...');
setTimeout(() => {
    console.log('Calling chat initialize() method...');
    const initParams = JSON.stringify([]);
    LogosCore.logos_core_call_plugin_method_async(
        'chat',
        'initialize',
        initParams,
        methodCallCallback,
        null
    );
}, 1000);

// Join a chat channel
const CHANNEL_NAME = 'baixa-chiado';
setTimeout(() => {
    console.log(`\n🏠 Joining channel: ${CHANNEL_NAME}`);
    const joinParams = JSON.stringify([
        {
            name: "channel",
            value: CHANNEL_NAME,
            type: "string"
        }
    ]);
    
    LogosCore.logos_core_call_plugin_method_async(
        'chat',
        'joinChannel',
        joinParams,
        methodCallCallback,
        null
    );
}, 2000);

// Retrieve chat history
setTimeout(() => {
    console.log(`\n📚 Retrieving chat history for channel: ${CHANNEL_NAME}`);
    const historyParams = JSON.stringify([
        {
            name: "channel",
            value: CHANNEL_NAME,
            type: "string"
        }
    ]);
    
    LogosCore.logos_core_call_plugin_method_async(
        'chat',
        'retrieveHistory',
        historyParams,
        methodCallCallback,
        null
    );
}, 3000);

// Send periodic messages every 3 seconds
let messageCounter = 0;
const NICK = 'NodeChatBot';

console.log('\n⏰ Setting up periodic message sending (every 3 seconds)...');

const sendPeriodicMessage = () => {
    messageCounter++;
    const timestamp = new Date().toISOString();
    const testMessage = `Hello from Node.js chat bot! Message #${messageCounter} at ${timestamp}`;
    
    console.log(`\n📤 Sending message #${messageCounter}: ${testMessage}`);
    
    const messageParams = JSON.stringify([
        {
            name: "channel",
            value: CHANNEL_NAME,
            type: "string"
        },
        {
            name: "nick",
            value: NICK,
            type: "string"
        },
        {
            name: "message",
            value: testMessage,
            type: "string"
        }
    ]);
    
    LogosCore.logos_core_call_plugin_method_async(
        'chat',
        'sendMessage',
        messageParams,
        methodCallCallback,
        null
    );
};

// Start sending messages after initial setup is complete
setTimeout(() => {
    console.log('✓ Starting periodic message sending...');
    sendPeriodicMessage(); // Send first message immediately
    
    // Then send every 3 seconds
    setInterval(sendPeriodicMessage, 3000);
}, 5000);

// Handle process termination gracefully
process.on('SIGINT', () => {
    console.log('\n🛑 Interrupted! Cleaning up...');
    LogosCore.logos_core_cleanup();
    console.log('✨ Cleanup completed!');
    process.exit(0);
});

// Run the Qt event loop periodically to handle events
console.log('\n🔄 Starting Qt event processing...');
console.log('💬 Chat application is now running!');
console.log(`📢 Sending messages to channel: ${CHANNEL_NAME}`);
console.log('🎯 Press Ctrl+C to exit');

// Process Qt events periodically to handle callbacks and events
setInterval(() => {
    LogosCore.logos_core_process_events();
}, 50); // Process every 50ms for responsive event handling 
