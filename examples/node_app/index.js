const ffi = require('ffi-napi');
const path = require('path');

// Simple hello world example that uses liblogos_core
console.log('Hello from Node.js!');

// Assuming the .so/.dylib is already built and available
const libExtension = process.platform === 'darwin' ? '.dylib' : '.so';
const libPath = path.resolve(__dirname, '../../core/build/lib', `liblogos_core${libExtension}`);

// Define the interface to liblogos_core
const LogosCore = ffi.Library(libPath, {
  'logos_core_init': ['void', ['int', 'pointer']],
  'logos_core_set_plugins_dir': ['void', ['string']],
  'logos_core_start': ['void', []],
  'logos_core_cleanup': ['void', []],
  'logos_core_load_plugin': ['int', ['string']],
  'logos_core_get_loaded_plugins': ['pointer', []],
  'logos_core_get_plugin_methods': ['pointer', ['string']],
  'logos_core_call_plugin_method': ['int', ['string', 'string', 'string']],
  'free': ['void', ['pointer']]
});

// Initialize logos_core
console.log('Initializing logos_core...');
LogosCore.logos_core_init(0, null);

// Set plugins directory
const pluginsDir = path.resolve(__dirname, '../../core/build/plugins');
console.log(`Setting plugins directory to: ${pluginsDir}`);
LogosCore.logos_core_set_plugins_dir(pluginsDir);

// Start logos_core
console.log('Starting logos_core...');
LogosCore.logos_core_start();

console.log('Logos Core initialized successfully!');
console.log('Hello World from Logos Core Node.js example!');

LogosCore.logos_core_load_plugin('waku');
LogosCore.logos_core_load_plugin('chat');
LogosCore.logos_core_load_plugin('calculator');

console.log("\n\n\n\n\n\n\n")

// Create the logoscore library
const logoscore = {};

// Get the loaded plugins
const ref = require('ref-napi');
const pluginsPtr = LogosCore.logos_core_get_loaded_plugins();

// Check if we got a valid pointer
if (!pluginsPtr.isNull()) {
  // This part is tricky with FFI - we're dealing with a char** (array of strings)
  // We need to read each string until we hit a NULL pointer
  let currentPtr = pluginsPtr;
  let offset = 0;
  
  // Keep reading pointers until we hit NULL
  while (true) {
    // Read the pointer at the current position
    const strPtr = ref.readPointer(currentPtr, offset);
    
    // If we hit a NULL pointer, we're done
    if (strPtr.isNull()) {
      break;
    }

    // Convert the pointer to a string
    const pluginName = ref.readCString(strPtr);
    console.log(`\n\n\nFound loaded plugin: ${pluginName}`);

    // Get plugin methods
    const methodsJSON = LogosCore.logos_core_get_plugin_methods(pluginName);
    if (!methodsJSON.isNull()) {
      const methodsString = ref.readCString(methodsJSON);
      console.log(methodsString)
      //try {
      //  const methodsString = ref.readCString(methodsJSON);
      //  const methods = JSON.parse(methodsString);
      //  console.log(`Plugin methods for ${pluginName}:`);
      //  methods.forEach(method => {
      //    console.log(`  - ${method.name}: ${method.signature}`);
      //    if (method.parameters && method.parameters.length > 0) {
      //      console.log('    Parameters:');
      //      method.parameters.forEach(param => {
      //        console.log(`      * ${param.name} (${param.type})`);
      //      });
      //    }
      //  });
      //  // Free the memory allocated by C++
      //  LogosCore.free(methodsJSON);
      //} catch (e) {
      //  console.error(`Error parsing methods for ${pluginName}:`, e);
      //  // Even on error, free the memory
      //  LogosCore.free(methodsJSON);
      //}
    }

    // Create an entry in the logoscore object for this plugin
    logoscore[pluginName] = {};

    // Move to the next pointer
    offset += ref.sizeof.pointer;
  }
}

console.log('Logos library created with these plugins:', Object.keys(logoscore));

// Call calculator plugin's add method
console.log('\n\nCalling calculator plugin add method:');
const calculatorParams = JSON.stringify([
  {
    "name": "a",
    "type": "int",
    "value": 5
  },
  {
    "name": "b",
    "type": "int",
    "value": 7
  }
]);
const result = LogosCore.logos_core_call_plugin_method('Simple Calculator Plugin', 'add', calculatorParams);
console.log(`Result of calculator.add(5, 7): ${result === 1 ? 'Call successful' : 'Call failed'}`);
// Note: The actual return value isn't accessible directly without additional FFI work

// Keep the process alive for demonstration purposes
const intervalId = setInterval(() => {
  console.log('Application running...');
}, 2000);

// Clean up on Ctrl+C
process.on('SIGINT', () => {
  clearInterval(intervalId);
  console.log('\nCleaning up...');
  LogosCore.logos_core_cleanup();
  process.exit(0);
});