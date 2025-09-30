const ffi = require('ffi-napi');
const ref = require('ref-napi');
const path = require('path');
const fs = require('fs');

/**
 * LogosAPI - A JavaScript SDK for interacting with liblogos_core
 * 
 * This class abstracts the FFI functionality needed to use liblogos including:
 * - Library loading and initialization
 * - Plugin management (processing, loading, unloading)
 * - Async method calls and event handling
 * - Event processing and lifecycle management
 */
class LogosAPI {
  constructor(options = {}) {
    this.options = {
      libPath: options.libPath || null,
      pluginsDir: options.pluginsDir || null,
      autoInit: options.autoInit !== false, // Default to true
      ...options
    };
    
    this.LogosCore = null;
    this.isInitialized = false;
    this.isStarted = false;
    this.eventProcessingInterval = null;
    this.callbacks = new Map();
    this.eventListeners = new Map();
    
    // Define pointer types for C string arrays
    this.StringArrayPtr = ref.refType(ref.types.CString);
    
    // Define callback type for async operations
    this.AsyncCallback = ffi.Function('void', ['int', 'string', 'pointer']);
    
    if (this.options.autoInit) {
      this.init();
    }

    // Cache for reflective plugin proxies
    this._pluginProxies = new Map();

    // Return a proxy to enable reflective access like logos.chat.joinChannel(...)
    return new Proxy(this, {
      get: (target, prop, receiver) => {
        // Preserve access to existing properties/methods
        if (prop in target) {
          return Reflect.get(target, prop, receiver);
        }
        // Support Symbol utilities without treating them as plugins
        if (typeof prop !== 'string') {
          return undefined;
        }
        // Dynamically create and return a plugin proxy
        return target._getPluginProxy(prop);
      }
    });
  }
  
  /**
   * Initialize the LogosCore library
   */
  init() {
    if (this.isInitialized) {
      throw new Error('LogosAPI is already initialized');
    }
    
    try {
      this._loadLibrary();
      this._initializeCore();
      this.isInitialized = true;
      return true;
    } catch (error) {
      throw new Error(`Failed to initialize LogosAPI: ${error.message}`);
    }
  }
  
  /**
   * Load the liblogos_core library
   * @private
   */
  _loadLibrary() {
    // Determine library path
    let libPath = this.options.libPath;
    if (!libPath) {
      const libExtension = this._getLibraryExtension();
      // First try to use the SDK's own lib directory
      libPath = path.resolve(__dirname, 'lib', `liblogos_core${libExtension}`);
      
      // Fall back to the original location if not found
      if (!fs.existsSync(libPath)) {
        libPath = path.resolve(__dirname, '../../../logos-liblogos/build/lib', `liblogos_core${libExtension}`);
      }
    }
    
    // Check if the library file exists
    if (!fs.existsSync(libPath)) {
      throw new Error(`Library file not found at: ${libPath}. Please build the core library first.`);
    }
    
    // Define the interface to liblogos_core
    this.LogosCore = ffi.Library(libPath, {
      // Core initialization and lifecycle
      'logos_core_init': ['void', ['int', 'pointer']],
      'logos_core_set_plugins_dir': ['void', ['string']],
      'logos_core_start': ['void', []],
      'logos_core_exec': ['int', []],
      'logos_core_cleanup': ['void', []],
      
      // Plugin management
      'logos_core_get_loaded_plugins': [this.StringArrayPtr, []],
      'logos_core_get_known_plugins': [this.StringArrayPtr, []],
      'logos_core_load_plugin': ['int', ['string']],
      'logos_core_unload_plugin': ['int', ['string']],
      'logos_core_process_plugin': ['string', ['string']],
      
      // Async callback functions
      'logos_core_async_operation': ['void', ['string', this.AsyncCallback, 'pointer']],
      'logos_core_load_plugin_async': ['void', ['string', this.AsyncCallback, 'pointer']],
      'logos_core_call_plugin_method_async': ['void', ['string', 'string', 'string', this.AsyncCallback, 'pointer']],
      
      // Event listener registration
      'logos_core_register_event_listener': ['void', ['string', 'string', this.AsyncCallback, 'pointer']],
      
      // Qt event processing (non-blocking)
      'logos_core_process_events': ['void', []]
    });
  }
  
  /**
   * Get the appropriate library extension for the current platform
   * @private
   */
  _getLibraryExtension() {
    switch (process.platform) {
      case 'darwin':
        return '.dylib';
      case 'win32':
        return '.dll';
      default:
        return '.so';
    }
  }
  
  /**
   * Initialize the core system
   * @private
   */
  _initializeCore() {
    // Initialize logos_core
    this.LogosCore.logos_core_init(0, null);
    
    // Set plugins directory
    let pluginsDir = this.options.pluginsDir;
    if (!pluginsDir) {
      // Default to plugins directory in the calling application
      pluginsDir = path.resolve(process.cwd(), 'plugins');
      
      // If not found, try other common locations
      if (!fs.existsSync(pluginsDir)) {
        pluginsDir = path.resolve(process.cwd(), 'modules');
      }
      
      // Fall back to the original core location if not found
      if (!fs.existsSync(pluginsDir)) {
        pluginsDir = path.resolve(__dirname, '../../../logos-liblogos/build/modules');
      }
    }
    
    if (!fs.existsSync(pluginsDir)) {
      throw new Error(`Plugins directory not found at: ${pluginsDir}. Please ensure plugins are available in your application directory.`);
    }
    
    this.LogosCore.logos_core_set_plugins_dir(pluginsDir);
  }
  
  /**
   * Start the LogosCore system
   */
  start() {
    if (!this.isInitialized) {
      throw new Error('LogosAPI must be initialized before starting');
    }
    
    if (this.isStarted) {
      throw new Error('LogosAPI is already started');
    }
    
    this.LogosCore.logos_core_start();
    this.isStarted = true;
    return true;
  }
  
  /**
   * Get the list of loaded plugins
   */
  getLoadedPlugins() {
    if (!this.isInitialized) {
      throw new Error('LogosAPI must be initialized first');
    }
    
    const loadedPluginsPtr = this.LogosCore.logos_core_get_loaded_plugins();
    return this._convertCStringArrayToJS(loadedPluginsPtr);
  }
  
  /**
   * Get the list of known plugins
   */
  getKnownPlugins() {
    if (!this.isInitialized) {
      throw new Error('LogosAPI must be initialized first');
    }
    
    const knownPluginsPtr = this.LogosCore.logos_core_get_known_plugins();
    return this._convertCStringArrayToJS(knownPluginsPtr);
  }
  
  /**
   * Get plugin status information
   */
  getPluginStatus() {
    return {
      loaded: this.getLoadedPlugins(),
      known: this.getKnownPlugins()
    };
  }
  
  /**
   * Process a plugin file
   * @param {string} pluginName - Name of the plugin (without extension)
   */
  processPlugin(pluginName) {
    if (!this.isInitialized) {
      throw new Error('LogosAPI must be initialized first');
    }
    
    const pluginExtension = this._getLibraryExtension();
    let pluginsDir = this.options.pluginsDir;
    if (!pluginsDir) {
      // Default to plugins directory in the calling application
      pluginsDir = path.resolve(process.cwd(), 'plugins');
      
      // If not found, try other common locations
      if (!fs.existsSync(pluginsDir)) {
        pluginsDir = path.resolve(process.cwd(), 'modules');
      }
      
      // Fall back to the original core location if not found
      if (!fs.existsSync(pluginsDir)) {
        pluginsDir = path.resolve(__dirname, '../../../logos-liblogos/build/modules');
      }
    }
    const pluginPath = path.join(pluginsDir, `${pluginName}_plugin${pluginExtension}`);
    
    const result = this.LogosCore.logos_core_process_plugin(pluginPath);
    return !!result;
  }
  
  /**
   * Load a plugin
   * @param {string} pluginName - Name of the plugin
   */
  loadPlugin(pluginName) {
    if (!this.isInitialized) {
      throw new Error('LogosAPI must be initialized first');
    }
    
    const result = this.LogosCore.logos_core_load_plugin(pluginName);
    return result === 1;
  }
  
  /**
   * Unload a plugin
   * @param {string} pluginName - Name of the plugin
   */
  unloadPlugin(pluginName) {
    if (!this.isInitialized) {
      throw new Error('LogosAPI must be initialized first');
    }
    
    const result = this.LogosCore.logos_core_unload_plugin(pluginName);
    return result === 1;
  }
  
  /**
   * Process and load a plugin in one step
   * @param {string} pluginName - Name of the plugin
   */
  processAndLoadPlugin(pluginName) {
    const processed = this.processPlugin(pluginName);
    if (!processed) {
      throw new Error(`Failed to process plugin: ${pluginName}`);
    }
    
    const loaded = this.loadPlugin(pluginName);
    if (!loaded) {
      throw new Error(`Failed to load plugin: ${pluginName}`);
    }
    
    return true;
  }
  
  /**
   * Process and load multiple plugins
   * @param {string[]} pluginNames - Array of plugin names
   */
  processAndLoadPlugins(pluginNames) {
    const results = {};
    
    // First process all plugins
    for (const pluginName of pluginNames) {
      try {
        results[pluginName] = { processed: this.processPlugin(pluginName) };
      } catch (error) {
        results[pluginName] = { processed: false, error: error.message };
      }
    }
    
    // Then load all plugins
    for (const pluginName of pluginNames) {
      if (results[pluginName].processed) {
        try {
          results[pluginName].loaded = this.loadPlugin(pluginName);
        } catch (error) {
          results[pluginName].loaded = false;
          results[pluginName].error = error.message;
        }
      }
    }
    
    return results;
  }
  
  /**
   * Call a plugin method asynchronously
   * @param {string} pluginName - Name of the plugin
   * @param {string} methodName - Name of the method
   * @param {string} params - JSON string of parameters
   * @param {Function} callback - Callback function (result, message) => {}
   */
  callPluginMethodAsync(pluginName, methodName, params, callback) {
    if (!this.isInitialized) {
      throw new Error('LogosAPI must be initialized first');
    }
    
    const callbackId = this._generateCallbackId();
    const ffiCallback = this._createFFICallback(callback, callbackId);
    
    this.callbacks.set(callbackId, { callback, ffiCallback });
    
    this.LogosCore.logos_core_call_plugin_method_async(
      pluginName,
      methodName,
      params,
      ffiCallback,
      null
    );
    
    return callbackId;
  }
  
  /**
   * Register an event listener
   * @param {string} pluginName - Name of the plugin
   * @param {string} eventName - Name of the event
   * @param {Function} callback - Callback function (result, message) => {}
   */
  registerEventListener(pluginName, eventName, callback) {
    if (!this.isInitialized) {
      throw new Error('LogosAPI must be initialized first');
    }
    
    const listenerId = this._generateCallbackId();
    const ffiCallback = this._createFFICallback(callback, listenerId);
    
    this.eventListeners.set(listenerId, { 
      pluginName, 
      eventName, 
      callback, 
      ffiCallback 
    });
    
    this.LogosCore.logos_core_register_event_listener(
      pluginName,
      eventName,
      ffiCallback,
      null
    );
    
    return listenerId;
  }
  
  /**
   * Start event processing loop
   * @param {number} interval - Processing interval in milliseconds (default: 100)
   */
  startEventProcessing(interval = 100) {
    if (this.eventProcessingInterval) {
      throw new Error('Event processing is already running');
    }
    
    this.eventProcessingInterval = setInterval(() => {
      if (this.LogosCore) {
        this.LogosCore.logos_core_process_events();
      }
    }, interval);
    
    return true;
  }
  
  /**
   * Stop event processing loop
   */
  stopEventProcessing() {
    if (this.eventProcessingInterval) {
      clearInterval(this.eventProcessingInterval);
      this.eventProcessingInterval = null;
      return true;
    }
    return false;
  }
  
  /**
   * Execute the core event loop (blocking)
   */
  exec() {
    if (!this.isInitialized) {
      throw new Error('LogosAPI must be initialized first');
    }
    
    return this.LogosCore.logos_core_exec();
  }
  
  /**
   * Clean up and shutdown
   */
  cleanup() {
    this.stopEventProcessing();
    
    if (this.LogosCore) {
      this.LogosCore.logos_core_cleanup();
    }
    
    // Clear all callbacks
    this.callbacks.clear();
    this.eventListeners.clear();
    
    this.isInitialized = false;
    this.isStarted = false;
  }
  
  /**
   * Convert C string array to JavaScript array
   * @private
   */
  _convertCStringArrayToJS(cStringArray) {
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
  
  /**
   * Generate a unique callback ID
   * @private
   */
  _generateCallbackId() {
    return `callback_${Date.now()}_${Math.random().toString(36).substr(2, 9)}`;
  }
  
  /**
   * Create an FFI callback wrapper
   * @private
   */
  _createFFICallback(userCallback, callbackId) {
    return ffi.Callback('void', ['int', 'string', 'pointer'], 
      (result, message, userData) => {
        try {
          const success = result === 1;
          
          // Try to parse message as JSON if possible
          let parsedMessage;
          try {
            parsedMessage = JSON.parse(message);
          } catch (e) {
            parsedMessage = message;
          }
          
          // Call user callback
          userCallback(success, parsedMessage, {
            callbackId,
            timestamp: new Date().toISOString(),
            rawMessage: message
          });
        } catch (error) {
          console.error(`Error in callback ${callbackId}:`, error);
        }
      }
    );
  }

  // ===== Reflective API helpers =====

  _getPluginProxy(pluginName) {
    if (this._pluginProxies.has(pluginName)) {
      return this._pluginProxies.get(pluginName);
    }
    const proxy = this._createReflectivePluginProxy(pluginName);
    this._pluginProxies.set(pluginName, proxy);
    return proxy;
  }

  _createReflectivePluginProxy(pluginName) {
    const decapitalize = (s) => s.length ? s.charAt(0).toLowerCase() + s.slice(1) : s;
    const makeParamsJson = (args) => {
      const toParam = (arg, index) => {
        const inferred = this._inferTypeAndValue(arg);
        return { name: `arg${index}`, value: inferred.value, type: inferred.type };
      };
      return JSON.stringify(Array.from(args).map(toParam));
    };

    const api = this; // capture

    return new Proxy({}, {
      get(_t, property) {
        if (property === 'pluginName') return pluginName;
        if (property === 'toString') return () => `[LogosPluginProxy ${pluginName}]`;
        // Avoid being mistaken for a thenable/Promise
        if (property === 'then') return undefined;

        if (typeof property !== 'string') {
          return undefined;
        }

        // Event subscription: on<EventName>(callback)
        if (property.startsWith('on') && property.length > 2) {
          const eventName = decapitalize(property.slice(2));
          return (callback) => {
            if (typeof callback !== 'function') {
              throw new Error(`Callback must be a function for ${pluginName}.${property}`);
            }
            return api.registerEventListener(pluginName, eventName, (success, message /* parsed or string */, meta) => {
              // Forward parsed event payload if available
              if (success) {
                // message is already parsed in _createFFICallback if JSON
                callback(message);
              } else {
                callback({ error: true, message });
              }
            });
          };
        }

        // Method invocation: returns a Promise
        return (...args) => new Promise((resolve, reject) => {
          try {
            const params = makeParamsJson(args);
            api.callPluginMethodAsync(pluginName, property, params, (success, message /* parsed or string */, meta) => {
              if (success) {
                resolve(message);
              } else {
                reject(message);
              }
            });
          } catch (err) {
            reject(err);
          }
        });
      }
    });
  }

  _inferTypeAndValue(value) {
    if (value === null || value === undefined) {
      return { type: 'string', value: '' };
    }
    const t = typeof value;
    if (t === 'boolean') {
      return { type: 'bool', value: value ? 'true' : 'false' };
    }
    if (t === 'number') {
      if (Number.isInteger(value)) {
        return { type: 'int', value: String(value) };
      }
      return { type: 'double', value: String(value) };
    }
    if (t === 'string') {
      return { type: 'string', value };
    }
    // Fallback: stringify complex values
    try {
      return { type: 'string', value: JSON.stringify(value) };
    } catch (_e) {
      return { type: 'string', value: String(value) };
    }
  }
}

module.exports = LogosAPI; 