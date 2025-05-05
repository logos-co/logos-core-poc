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
  'logos_core_call_plugin_method': ['pointer', ['string', 'string', 'string', 'string']],
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
      console.log(methodsString);
      
      try {
        // Create an entry in the logoscore object for this plugin
        logoscore[pluginName] = {};
        
        // Parse the methods JSON
        const methods = JSON.parse(methodsString);
        
        // Filter out duplicate methods, keeping the one with most parameters
        const methodMap = new Map();
        methods.forEach(method => {
            const existingMethod = methodMap.get(method.name);
            if (!existingMethod || 
                (method.parameters?.length || 0) > (existingMethod.parameters?.length || 0)) {
                methodMap.set(method.name, method);
            }
        });
        // Replace methods array with filtered unique methods
        methods.splice(0, methods.length, ...methodMap.values());

        console.log(JSON.stringify(methods));

        // Create wrapper functions for each method
        methods.forEach(method => {
          console.log(`Creating wrapper for ${pluginName}.${method.name}`);
          
          // Skip methods that aren't invokable
          if (!method.isInvokable) {
            console.log(`Skipping non-invokable method: ${method.name}`);
            return;
          }
          
          // Create a wrapper function for this method
          logoscore[pluginName][method.name] = function() {
            // Check if we have the right number of arguments
            if (arguments.length !== (method.parameters ? method.parameters.length : 0)) {
              throw new Error(`Method ${method.name} expects ${method.parameters ? method.parameters.length : 0} arguments, but got ${arguments.length}`);
            }
            
            // Create the params array for the method call
            const params = [];
            if (method.parameters) {
              for (let i = 0; i < method.parameters.length; i++) {
                params.push({
                  name: method.parameters[i].name,
                  type: method.parameters[i].type,
                  value: arguments[i]
                });
              }
            }
            
            // Convert params to JSON
            const paramsJson = JSON.stringify(params);

            console.log("paramsJson:", paramsJson);
            
            // Determine return type (use the method's return type)
            const returnType = method.returnType || 'void';
            
            // Call the method
            console.log(`Calling ${pluginName}.${method.name}...`);
            const resultPtr = LogosCore.logos_core_call_plugin_method(pluginName, method.name, paramsJson, returnType);
            
            if (!resultPtr.isNull()) {
              try {
                // Convert the returned pointer to a string and parse it as JSON
                const resultJsonStr = ref.readCString(resultPtr);
                const resultObj = JSON.parse(resultJsonStr);
                
                // Free the memory allocated by C++
                LogosCore.free(resultPtr);
                
                // Check if the call was successful
                if (!resultObj.success) {
                  throw new Error(`Error calling ${pluginName}.${method.name}: ${resultObj.message}`);
                }
                
                // Return the result value if available
                return resultObj.hasOwnProperty('returnValue') ? resultObj.returnValue : undefined;
              } catch (e) {
                LogosCore.free(resultPtr);
                throw e;
              }
            } else {
              throw new Error(`Error calling ${pluginName}.${method.name}: null pointer returned`);
            }
          };
        });
        
        // Free the memory allocated by C++
        LogosCore.free(methodsJSON);
      } catch (e) {
        console.error(`Error processing methods for ${pluginName}:`, e);
        // Even on error, free the memory
        LogosCore.free(methodsJSON);
      }
    }

    // Move to the next pointer
    offset += ref.sizeof.pointer;
  }
}

console.log('Logos library created with these plugins:', Object.keys(logoscore));

// Call calculator plugin's add method using the wrapper function
console.log('\n\nCalling calculator plugin add method using wrapper function:');
try {
  const result = logoscore.calculator.add(5, 7);
  console.log(`Result of calculator.add(5, 7) = ${result}`);
} catch (error) {
  console.error('Error calling calculator.add:', error.message);
}

// For comparison, here's the old way of calling the method directly
console.log('\n\nCalling calculator plugin add method using direct call (old way):');
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
const resultPtr = LogosCore.logos_core_call_plugin_method('calculator', 'add', calculatorParams, 'int');
if (!resultPtr.isNull()) {
  // Convert the returned pointer to a string and parse it as JSON
  const resultJsonStr = ref.readCString(resultPtr);
  const resultObj = JSON.parse(resultJsonStr);
  
  console.log('Call successful:', resultObj.success);
  console.log('Message:', resultObj.message);
  
  if (resultObj.success && resultObj.hasOwnProperty('returnValue')) {
    console.log(`Result of calculator.add(5, 7) = ${resultObj.returnValue}`);
  }
  
  // Free the memory allocated by C++
  LogosCore.free(resultPtr);
} else {
  console.log('Call failed: null pointer returned');
}

// Examples of calling other plugin methods if they exist
console.log('\n\nTrying other plugin methods using wrapper functions:');

// Try to call a method on the chat plugin if it exists
if (logoscore.chat && logoscore.chat.sendMessage) {
  try {
    console.log('Sending a test message to general-chat channel...');
    const result = logoscore.chat.sendMessage('general-chat', 'Hello from Node.js!');
    console.log('Message sent result:', result);
  } catch (error) {
    console.error('Error calling chat.sendMessage:', error.message);
  }
}

// Try to call a method on the waku plugin if it exists
if (logoscore.waku && logoscore.waku.getVersion) {
  try {
    console.log('Getting Waku version...');
    const version = logoscore.waku.getVersion();
    console.log('Waku version:', version);
  } catch (error) {
    console.error('Error calling waku.getVersion:', error.message);
  }
}

// Try some other calculator methods if they exist
if (logoscore.calculator) {
  // Try multiply method
  if (logoscore.calculator.multiply) {
    try {
      console.log('Calling calculator.multiply(6, 7)...');
      const result = logoscore.calculator.multiply(6, 7);
      console.log(`Result of calculator.multiply(6, 7) = ${result}`);
    } catch (error) {
      console.error('Error calling calculator.multiply:', error.message);
    }
  }
  
  // Try subtract method
  if (logoscore.calculator.subtract) {
    try {
      console.log('Calling calculator.subtract(20, 7)...');
      const result = logoscore.calculator.subtract(20, 7);
      console.log(`Result of calculator.subtract(20, 7) = ${result}`);
    } catch (error) {
      console.error('Error calling calculator.subtract:', error.message);
    }
  }
}

console.log('logoscore:', logoscore);

// Keep the process alive for demonstration purposes
//const intervalId = setInterval(() => {
//  console.log('Application running...');
//}, 2000);
//
//// Clean up on Ctrl+C
//process.on('SIGINT', () => {
//  clearInterval(intervalId);
//  console.log('\nCleaning up...');
//  LogosCore.logos_core_cleanup();
//  process.exit(0);
//});