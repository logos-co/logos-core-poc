const { app, BrowserWindow, ipcMain } = require('electron');
const path = require('path');
const ffi = require('ffi-napi');
const ref = require('ref-napi');
const fs = require('fs');

// Keep a global reference of the window object
let mainWindow;
let LogosCore = null;

// Define callback type for async operations
const AsyncCallback = ffi.Function('void', ['int', 'string', 'pointer']);

// Chat state
let chatInitialized = false;
let currentChannel = 'baixa-chiado'; // Default channel
let username = `LogosUser_${Math.floor(Math.random() * 100).toString().padStart(2, '0')}`;

// Initialize LogosCore FFI
function initializeLogosCore() {
  try {
    // Determine library extension based on platform
    const libExtension = process.platform === 'darwin' ? '.dylib' : '.so';
    const libPath = path.resolve(__dirname, '../../core/build/lib', `liblogos_core${libExtension}`);
    
    console.log(`Looking for library at: ${libPath}`);
    
    // Check if the library file exists
    if (!fs.existsSync(libPath)) {
      throw new Error(`Library file not found at: ${libPath}. Please build the core library first by running: ./scripts/run_core.sh build`);
    }
    
    // Define the interface to liblogos_core
    LogosCore = ffi.Library(libPath, {
      // Core initialization and lifecycle
      'logos_core_init': ['void', ['int', 'pointer']],
      'logos_core_set_plugins_dir': ['void', ['string']],
      'logos_core_start': ['void', []],
      'logos_core_cleanup': ['void', []],
      
      // Plugin management
      'logos_core_get_loaded_plugins': [ref.refType(ref.types.CString), []],
      'logos_core_get_known_plugins': [ref.refType(ref.types.CString), []],
      'logos_core_process_plugin': ['string', ['string']],
      'logos_core_load_plugin': ['int', ['string']],
      
      // Async operations and event handling
      'logos_core_call_plugin_method_async': ['void', ['string', 'string', 'string', AsyncCallback, 'pointer']],
      'logos_core_register_event_listener': ['void', ['string', 'string', AsyncCallback, 'pointer']],
      'logos_core_process_events': ['void', []]
    });
    
    return true;
  } catch (error) {
    console.error('Failed to initialize LogosCore:', error.message);
    return false;
  }
}

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
  if (!LogosCore) return { loaded: [], known: [] };
  
  try {
    // Get loaded plugins
    const loadedPluginsPtr = LogosCore.logos_core_get_loaded_plugins();
    const loadedPlugins = convertCStringArrayToJS(loadedPluginsPtr);
    
    // Get known plugins
    const knownPluginsPtr = LogosCore.logos_core_get_known_plugins();
    const knownPlugins = convertCStringArrayToJS(knownPluginsPtr);
    
    return { loaded: loadedPlugins, known: knownPlugins };
  } catch (error) {
    console.error('Error getting plugin status:', error.message);
    return { loaded: [], known: [], error: error.message };
  }
}

// Create callback functions for async operations
let methodCallCallback = null;
let eventListenerCallback = null;

function createCallbacks() {
  // Method call callback
  methodCallCallback = ffi.Callback('void', ['int', 'string', 'pointer'], 
    (result, message, userData) => {
      const timestamp = new Date().toISOString();
      console.log(`\n🔔 [${timestamp}] METHOD CALLBACK RECEIVED:`);
      console.log(`   Result: ${result} (${result === 1 ? 'SUCCESS' : 'FAILURE'})`);
      console.log(`   Message: "${message}"`);
      console.log(`   UserData: ${userData}`);

      if (result === 1) {
        console.log(`🎯 [${timestamp}] METHOD SUCCESS: ${message}`);
        // Send to renderer process
        if (mainWindow && !mainWindow.isDestroyed()) {
          mainWindow.webContents.send('chat-method-result', { success: true, message, timestamp });
        }
      } else {
        console.log(`🚫 [${timestamp}] METHOD FAILED: ${message}`);
        // Send to renderer process
        if (mainWindow && !mainWindow.isDestroyed()) {
          mainWindow.webContents.send('chat-method-result', { success: false, message, timestamp });
        }
      }
      console.log(`🔔 [${timestamp}] METHOD CALLBACK COMPLETED\n`);
    }
  );

  // Event listener callback
  eventListenerCallback = ffi.Callback('void', ['int', 'string', 'pointer'], 
    (result, message, userData) => {
      const timestamp = new Date().toISOString();
      console.log(`\n\n\n\n\n\n🎉 [${timestamp}] EVENT RECEIVED: ${message}`);
      if (result === 1) {
        console.log(`🎉 [${timestamp}] EVENT RECEIVED: ${message}`);
        
        // Try to parse the message as JSON to see event data
        try {
          const parsedMessage = JSON.parse(message);
          console.log(`\n\n\n\n\n\n🎊 [${timestamp}] EVENT NAME: ${parsedMessage.event}`);
          console.log(`\n\n\n\n\n\n📊 [${timestamp}] EVENT DATA:`, parsedMessage.data);
          
          // Special logging for chatMessage events
          if (parsedMessage.event === 'chatMessage' && Array.isArray(parsedMessage.data) && parsedMessage.data.length >= 3) {
            console.log(`\n\n\n\n\n\n💬 [${timestamp}] CHAT MESSAGE - Timestamp: ${parsedMessage.data[0]}, Nick: ${parsedMessage.data[1]}, Message: ${parsedMessage.data[2]}`);
          }
          
          // Send to renderer process
          if (mainWindow && !mainWindow.isDestroyed()) {
            mainWindow.webContents.send('chat-event', { 
              eventName: parsedMessage.event, 
              data: parsedMessage.data, 
              timestamp,
              rawMessage: message
            });
          }
        } catch (e) {
          // Message is not JSON, just print it as is
          console.log(`📝 [${timestamp}] RAW EVENT MESSAGE: ${message}`);
          
          // Send to renderer process
          if (mainWindow && !mainWindow.isDestroyed()) {
            mainWindow.webContents.send('chat-event', { 
              eventName: 'raw', 
              data: message, 
              timestamp,
              rawMessage: message
            });
          }
        }
      } else {
        console.log(`🚫 [${timestamp}] EVENT LISTENER FAILED: ${message}`);
        // Send to renderer process
        if (mainWindow && !mainWindow.isDestroyed()) {
          mainWindow.webContents.send('chat-event', { 
            success: false, 
            message, 
            timestamp 
          });
        }
      }
    }
  );
}

// Auto-initialize chat flow similar to ChatWidget
async function autoInitializeChat() {
  try {
    console.log('🚀 Starting auto-initialization...');
    
    // Send status update to renderer
    if (mainWindow && !mainWindow.isDestroyed()) {
      mainWindow.webContents.send('chat-status', { status: 'Initializing chat...', username });
    }
    
    // Initialize chat
    console.log('🔧 Calling chat initialize() method...');
    const noParams = JSON.stringify([]);
    LogosCore.logos_core_call_plugin_method_async(
      'chat', 
      'initialize', 
      noParams, 
      methodCallCallback, 
      null
    );
    
    // Wait a bit for initialization
    await new Promise(resolve => setTimeout(resolve, 1000));
    
    // Register event listeners
    console.log('📡 Registering event listeners for chat...');
    LogosCore.logos_core_register_event_listener('chat', 'chatMessage', eventListenerCallback, null);
    LogosCore.logos_core_register_event_listener('chat', 'historyMessage', eventListenerCallback, null);
    
    // Wait a bit for event registration
    await new Promise(resolve => setTimeout(resolve, 500));
    
    // Join default channel
    console.log(`🚪 Joining default channel: ${currentChannel}`);
    const params = JSON.stringify([
      {
        name: "channel",
        value: currentChannel,
        type: "string"
      }
    ]);
    
    LogosCore.logos_core_call_plugin_method_async(
      'chat', 
      'joinChannel', 
      params, 
      methodCallCallback, 
      null
    );
    
    // Wait a bit for channel join
    await new Promise(resolve => setTimeout(resolve, 1000));
    
    // Retrieve history
    console.log(`📜 Retrieving history for channel: ${currentChannel}`);
    LogosCore.logos_core_call_plugin_method_async(
      'chat', 
      'retrieveHistory', 
      params, 
      methodCallCallback, 
      null
    );
    
    chatInitialized = true;
    
    // Send ready status to renderer
    if (mainWindow && !mainWindow.isDestroyed()) {
      mainWindow.webContents.send('chat-status', { 
        status: `Connected to channel: ${currentChannel}`, 
        username,
        ready: true,
        currentChannel
      });
    }
    
    console.log('✅ Auto-initialization completed!');
    
  } catch (error) {
    console.error('❌ Auto-initialization failed:', error);
    if (mainWindow && !mainWindow.isDestroyed()) {
      mainWindow.webContents.send('chat-status', { 
        status: `Error: ${error.message}`, 
        username,
        error: true
      });
    }
  }
}

function createWindow() {
  // Create the browser window
  mainWindow = new BrowserWindow({
    width: 1000,
    height: 800,
    webPreferences: {
      nodeIntegration: false, // Security best practice
      contextIsolation: true, // Security best practice
      preload: path.join(__dirname, 'preload.js')
    }
  });

  // Load the app
  mainWindow.loadFile('index.html');

  // Open DevTools for development
  mainWindow.webContents.openDevTools();

  // Emitted when the window is closed
  mainWindow.on('closed', function () {
    mainWindow = null;
  });
}

// This method will be called when Electron has finished initialization
app.whenReady().then(createWindow);

// Quit when all windows are closed
app.on('window-all-closed', function () {
  if (process.platform !== 'darwin') app.quit();
});

app.on('activate', function () {
  if (mainWindow === null) createWindow();
});

// Handle process termination gracefully
app.on('before-quit', () => {
  if (LogosCore) {
    console.log('Cleaning up LogosCore...');
    try {
      LogosCore.logos_core_cleanup();
      console.log('LogosCore cleanup completed!');
    } catch (error) {
      console.error('Error during LogosCore cleanup:', error.message);
    }
  }
});

// Start Qt event processing
let eventProcessingInterval = null;

function startEventProcessing() {
  if (LogosCore && !eventProcessingInterval) {
    console.log('🔄 Starting Qt event processing...');
    eventProcessingInterval = setInterval(() => {
      try {
        LogosCore.logos_core_process_events();
      } catch (error) {
        console.error('Error processing events:', error.message);
      }
    }, 50); // Process every 50ms for responsive event handling
  }
}

function stopEventProcessing() {
  if (eventProcessingInterval) {
    clearInterval(eventProcessingInterval);
    eventProcessingInterval = null;
    console.log('🛑 Stopped Qt event processing');
  }
}

// IPC handler for LogosCore initialization and auto-chat setup
ipcMain.handle('initialize-chat-app', async () => {
  try {
    // Initialize LogosCore FFI
    if (!initializeLogosCore()) {
      return { success: false, error: 'Failed to initialize LogosCore FFI' };
    }
    
    // Create callbacks
    createCallbacks();
    
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

    // Process and load plugins
    const pluginsToLoad = ['waku_module', 'chat'];
    const pluginResults = [];

    // Determine plugin extension based on platform
    let pluginExtension;
    if (process.platform === 'darwin') {
      pluginExtension = '.dylib';
    } else if (process.platform === 'win32') {
      pluginExtension = '.dll'; 
    } else {
      pluginExtension = '.so';
    }

    // First process the plugins
    console.log('Processing plugins...');
    for (const pluginName of pluginsToLoad) {
      console.log(`Processing plugin file: ${pluginName}`);
      
      // Construct full plugin path
      const pluginPath = path.join(pluginsDir, `${pluginName}_plugin${pluginExtension}`);
      const result = LogosCore.logos_core_process_plugin(pluginPath);
      
      if (result) {
        console.log(`✓ Processed plugin: ${pluginName}`);
        pluginResults.push({ plugin: pluginName, processed: true, processResult: result });
      } else {
        console.log(`✗ Failed to process plugin file: ${pluginName}`);
        pluginResults.push({ plugin: pluginName, processed: false, processResult: null });
      }
    }

    // Then load the plugins
    console.log('Loading plugins...');
    for (const pluginName of pluginsToLoad) {
      console.log(`Loading plugin: ${pluginName}`);
      const result = LogosCore.logos_core_load_plugin(pluginName);
      
      // Find the corresponding plugin result to update it
      const pluginResult = pluginResults.find(p => p.plugin === pluginName);
      if (pluginResult) {
        if (result === 1) {
          console.log(`✓ Successfully loaded ${pluginName} plugin`);
          pluginResult.loaded = true;
          pluginResult.loadResult = result;
        } else {
          console.log(`✗ Failed to load ${pluginName} plugin`);
          pluginResult.loaded = false;
          pluginResult.loadResult = result;
        }
      }
    }

    // Get final plugin status
    const finalPluginStatus = printLoadedPlugins();
    console.log('Final plugin status:', finalPluginStatus);
    
    // Start event processing
    startEventProcessing();
    
    // Auto-initialize chat
    setTimeout(() => autoInitializeChat(), 1000);
    
    return { 
      success: true, 
      message: 'Chat app initialized successfully!',
      pluginStatus: finalPluginStatus,
      pluginResults: pluginResults,
      username: username
    };
  } catch (error) {
    return { success: false, error: error.message };
  }
});

// IPC handler for sending messages
ipcMain.handle('send-message', async (event, message) => {
  try {
    console.log(`\n📨 SEND MESSAGE REQUEST RECEIVED:`);
    console.log(`   Message: "${message}"`);
    console.log(`   Current Channel: "${currentChannel}"`);
    console.log(`   Username: "${username}"`);
    console.log(`   Chat Initialized: ${chatInitialized}`);
    console.log(`   LogosCore Available: ${!!LogosCore}`);
    console.log(`   Method Callback Available: ${!!methodCallCallback}`);
    
    if (!LogosCore || !methodCallCallback || !chatInitialized) {
      console.log('❌ Send message failed: Chat not initialized');
      return { success: false, error: 'Chat not initialized' };
    }
    
    if (!message || message.trim() === '') {
      console.log('❌ Send message failed: Empty message');
      return { success: false, error: 'Message cannot be empty' };
    }
    
    console.log(`💬 Sending message to ${currentChannel}: "${message}" from user: ${username}`);
    
    const params = JSON.stringify([
      {
        name: "channel",
        value: currentChannel,
        type: "string"
      },
      {
        name: "nick",
        value: username,
        type: "string"
      },
      {
        name: "message",
        value: message.trim(),
        type: "string"
      }
    ]);
    
    console.log(`📤 Calling sendMessage with params: ${params}`);
    console.log(`📤 Parameter breakdown:`);
    console.log(`   - Plugin: "chat"`);
    console.log(`   - Method: "sendMessage"`);
    console.log(`   - Params: ${params}`);
    console.log(`   - Callback: ${!!methodCallCallback}`);
    
    console.log(`🚀 Invoking logos_core_call_plugin_method_async...`);
    
    LogosCore.logos_core_call_plugin_method_async(
      'chat', 
      'sendMessage', 
      params, 
      methodCallCallback, 
      null
    );
    
    console.log('✅ Send message call completed, waiting for callback...');
    console.log(`📨 SEND MESSAGE REQUEST COMPLETED\n`);
    
    return { success: true, message: 'Message sent' };
  } catch (error) {
    console.error('❌ Error in send-message handler:', error);
    console.error('❌ Stack trace:', error.stack);
    return { success: false, error: error.message };
  }
});

// IPC handler for joining a new channel
ipcMain.handle('join-channel', async (event, channelName) => {
  try {
    if (!LogosCore || !methodCallCallback || !chatInitialized) {
      return { success: false, error: 'Chat not initialized' };
    }
    
    if (!channelName || channelName.trim() === '') {
      return { success: false, error: 'Channel name cannot be empty' };
    }
    
    currentChannel = channelName.trim();
    console.log(`🚪 Joining channel: ${currentChannel}`);
    
    const params = JSON.stringify([
      {
        name: "channel",
        value: currentChannel,
        type: "string"
      }
    ]);
    
    LogosCore.logos_core_call_plugin_method_async(
      'chat', 
      'joinChannel', 
      params, 
      methodCallCallback, 
      null
    );
    
    // Retrieve history after joining
    setTimeout(() => {
      LogosCore.logos_core_call_plugin_method_async(
        'chat', 
        'retrieveHistory', 
        params, 
        methodCallCallback, 
        null
      );
    }, 500);
    
    return { success: true, message: `Joining channel: ${currentChannel}`, currentChannel };
  } catch (error) {
    return { success: false, error: error.message };
  }
}); 