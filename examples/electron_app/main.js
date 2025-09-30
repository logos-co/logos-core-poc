const { app, BrowserWindow, ipcMain } = require('electron');
const path = require('path');
const fs = require('fs');
const LogosAPI = require('logos-api');

// Keep a global reference of the window object
let mainWindow;
let logos = null;

// Chat state
let chatInitialized = false;
let currentChannel = 'baixa-chiado'; // Default channel
let username = `LogosUser_${Math.floor(Math.random() * 100).toString().padStart(2, '0')}`;

// Helper function to print loaded plugins via LogosAPI
function getPluginStatus() {
  try {
    if (!logos) return { loaded: [], known: [] };
    return logos.getPluginStatus();
  } catch (error) {
    console.error('Error getting plugin status:', error.message);
    return { loaded: [], known: [], error: error.message };
  }
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
    try {
      const message = await logos.chat.initialize();
      const timestamp = new Date().toISOString();
      if (mainWindow && !mainWindow.isDestroyed()) {
        mainWindow.webContents.send('chat-method-result', { success: true, message, timestamp });
      }
    } catch (message) {
      const timestamp = new Date().toISOString();
      if (mainWindow && !mainWindow.isDestroyed()) {
        mainWindow.webContents.send('chat-method-result', { success: false, message, timestamp });
      }
    }
    
    // Wait a bit for initialization
    await new Promise(resolve => setTimeout(resolve, 1000));
    
    // Register event listeners
    console.log('📡 Registering event listeners for chat...');
    const forwardEvent = (eventObj) => {
      const timestamp = new Date().toISOString();
      if (!eventObj || typeof eventObj !== 'object') {
        if (mainWindow && !mainWindow.isDestroyed()) {
          mainWindow.webContents.send('chat-event', { success: false, message: eventObj, timestamp });
        }
        return;
      }
      const eventName = eventObj.event || 'raw';
      const data = eventObj.data;
      if (mainWindow && !mainWindow.isDestroyed()) {
        mainWindow.webContents.send('chat-event', { eventName, data, timestamp, rawMessage: JSON.stringify(eventObj) });
      }
    };
    logos.chat.onChatMessage(forwardEvent);
    logos.chat.onHistoryMessage(forwardEvent);
    
    // Wait a bit for event registration
    await new Promise(resolve => setTimeout(resolve, 500));
    
    // Join default channel
    console.log(`🚪 Joining default channel: ${currentChannel}`);
    try {
      const message = await logos.chat.joinChannel(currentChannel);
      const timestamp = new Date().toISOString();
      if (mainWindow && !mainWindow.isDestroyed()) {
        mainWindow.webContents.send('chat-method-result', { success: true, message, timestamp });
      }
    } catch (message) {
      const timestamp = new Date().toISOString();
      if (mainWindow && !mainWindow.isDestroyed()) {
        mainWindow.webContents.send('chat-method-result', { success: false, message, timestamp });
      }
    }
    
    // Wait a bit for channel join
    await new Promise(resolve => setTimeout(resolve, 1000));
    
    // Retrieve history
    console.log(`📜 Retrieving history for channel: ${currentChannel}`);
    try {
      const message = await logos.chat.retrieveHistory(currentChannel);
      const timestamp = new Date().toISOString();
      if (mainWindow && !mainWindow.isDestroyed()) {
        mainWindow.webContents.send('chat-method-result', { success: true, message, timestamp });
      }
    } catch (message) {
      const timestamp = new Date().toISOString();
      if (mainWindow && !mainWindow.isDestroyed()) {
        mainWindow.webContents.send('chat-method-result', { success: false, message, timestamp });
      }
    }
    
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

  // DevTools can be opened manually with Ctrl+Shift+I or Cmd+Option+I
  // mainWindow.webContents.openDevTools();

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
  if (logos) {
    console.log('Cleaning up LogosAPI...');
    try {
      logos.cleanup();
      console.log('LogosAPI cleanup completed!');
    } catch (error) {
      console.error('Error during LogosAPI cleanup:', error.message);
    }
  }
});

// Start Qt event processing
function startEventProcessing() {
  if (logos) {
    console.log('🔄 Starting Qt event processing...');
    logos.startEventProcessing(50);
  }
}

function stopEventProcessing() {
  if (logos) {
    logos.stopEventProcessing();
    console.log('🛑 Stopped Qt event processing');
  }
}

// IPC handler for LogosCore initialization and auto-chat setup
ipcMain.handle('initialize-chat-app', async () => {
  try {
    // Ensure logos_host can be located by the core
    const logosHostPath = path.resolve(
      __dirname,
      '../../logos-liblogos/build/bin',
      process.platform === 'win32' ? 'logos_host.exe' : 'logos_host'
    );
    process.env.LOGOS_HOST_PATH = logosHostPath;
    if (!fs.existsSync(logosHostPath)) {
      console.warn(`logos_host not found at: ${logosHostPath}`);
    } else {
      console.log(`Using logos_host at: ${logosHostPath}`);
    }

    // Determine library extension based on platform
    const libExtension = process.platform === 'darwin' ? '.dylib' : (process.platform === 'win32' ? '.dll' : '.so');
    const libPath = path.resolve(__dirname, '../../logos-liblogos/build/lib', `liblogos_core${libExtension}`);
    const pluginsDir = path.resolve(__dirname, '../../logos-liblogos/build/modules');

    console.log(`Looking for library at: ${libPath}`);
    if (!fs.existsSync(libPath)) {
      throw new Error(`Library file not found at: ${libPath}. Please build the core library first by running: ./scripts/run_core.sh build`);
    }

    // Initialize LogosAPI
    logos = new LogosAPI({ libPath, pluginsDir, autoInit: true });
    logos.start();
    console.log('Logos Core initialized successfully via LogosAPI!');

    // Process and load plugins
    const pluginsToLoad = ['capability_module', 'waku_module', 'chat'];
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

    // First process the plugins (validate file exists first)
    console.log('Processing plugins...');
    for (const pluginName of pluginsToLoad) {
      console.log(`Processing plugin file: ${pluginName}`);
      const candidatePath = path.join(pluginsDir, `${pluginName}_plugin${pluginExtension}`);
      if (!fs.existsSync(candidatePath)) {
        console.warn(`Plugin file not found: ${candidatePath}`);
        pluginResults.push({ plugin: pluginName, processed: false, processResult: null, missing: true, path: candidatePath });
        continue;
      }

      const processed = logos.processPlugin(pluginName);
      if (processed) {
        console.log(`✓ Processed plugin: ${pluginName}`);
        pluginResults.push({ plugin: pluginName, processed: true, processResult: true });
      } else {
        console.log(`✗ Failed to process plugin file: ${pluginName}`);
        pluginResults.push({ plugin: pluginName, processed: false, processResult: null, path: candidatePath });
      }
    }

    // Then load the plugins
    console.log('Loading plugins...');
    for (const pluginName of pluginsToLoad) {
      console.log(`Loading plugin: ${pluginName}`);
      const loaded = logos.loadPlugin(pluginName);
      
      // Find the corresponding plugin result to update it
      const pluginResult = pluginResults.find(p => p.plugin === pluginName);
      if (pluginResult) {
        if (loaded) {
          console.log(`✓ Successfully loaded ${pluginName} plugin`);
          pluginResult.loaded = true;
          pluginResult.loadResult = 1;
        } else {
          console.log(`✗ Failed to load ${pluginName} plugin`);
          pluginResult.loaded = false;
          pluginResult.loadResult = 0;
        }
      }
    }

    // Get final plugin status
    const finalPluginStatus = getPluginStatus();
    console.log('Final plugin status:', finalPluginStatus);
    console.log('Plugin processing results:', pluginResults);
    
    // Start event processing
    startEventProcessing();

    // Give plugins extra time to boot their processes and establish connections
    setTimeout(() => autoInitializeChat(), 3000);
    
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
    console.log(`   LogosAPI Available: ${!!logos}`);
    
    if (!logos || !chatInitialized) {
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
    
  console.log(`🚀 Invoking logos.chat.sendMessage...`);
    
    try {
      const resp = await logos.chat.sendMessage(currentChannel, username, message.trim());
      const timestamp = new Date().toISOString();
      if (mainWindow && !mainWindow.isDestroyed()) {
        mainWindow.webContents.send('chat-method-result', { success: true, message: resp, timestamp });
      }
    } catch (resp) {
      const timestamp = new Date().toISOString();
      if (mainWindow && !mainWindow.isDestroyed()) {
        mainWindow.webContents.send('chat-method-result', { success: false, message: resp, timestamp });
      }
    }
    
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
    if (!logos || !chatInitialized) {
      return { success: false, error: 'Chat not initialized' };
    }
    
    if (!channelName || channelName.trim() === '') {
      return { success: false, error: 'Channel name cannot be empty' };
    }
    
    currentChannel = channelName.trim();
    console.log(`🚪 Joining channel: ${currentChannel}`);
    
    try {
      const resp = await logos.chat.joinChannel(currentChannel);
      const timestamp = new Date().toISOString();
      if (mainWindow && !mainWindow.isDestroyed()) {
        mainWindow.webContents.send('chat-method-result', { success: true, message: resp, timestamp });
      }
    } catch (resp) {
      const timestamp = new Date().toISOString();
      if (mainWindow && !mainWindow.isDestroyed()) {
        mainWindow.webContents.send('chat-method-result', { success: false, message: resp, timestamp });
      }
    }
    
    // Retrieve history after joining
    setTimeout(async () => {
      try {
        const resp = await logos.chat.retrieveHistory(currentChannel);
        const timestamp = new Date().toISOString();
        if (mainWindow && !mainWindow.isDestroyed()) {
          mainWindow.webContents.send('chat-method-result', { success: true, message: resp, timestamp });
        }
      } catch (resp) {
        const timestamp = new Date().toISOString();
        if (mainWindow && !mainWindow.isDestroyed()) {
          mainWindow.webContents.send('chat-method-result', { success: false, message: resp, timestamp });
        }
      }
    }, 500);
    
    return { success: true, message: `Joining channel: ${currentChannel}`, currentChannel };
  } catch (error) {
    return { success: false, error: error.message };
  }
}); 