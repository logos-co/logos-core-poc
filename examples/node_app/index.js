const ffi = require('ffi-napi');
const ref = require('ref-napi');
const StructType = require('ref-struct-di')(ref);
const ArrayType = require('ref-array-di')(ref);
const path = require('path');

// Simple hello world example that uses liblogos_core
console.log('Hello from Node.js!');

// Assuming the .so/.dylib is already built and available
const libExtension = process.platform === 'darwin' ? '.dylib' : '.so';
const libPath = path.resolve(__dirname, '../../core/build/lib', `liblogos_core${libExtension}`);

// Define the callback types
const WakuCallbackType = ffi.Function('void', ['bool', 'string']);

// Define the interface to liblogos_core
const LogosCore = ffi.Library(libPath, {
  'logos_core_init': ['void', ['int', 'pointer']],
  'logos_core_set_plugins_dir': ['void', ['string']],
  'logos_core_start': ['void', []],
  'logos_core_cleanup': ['void', []],
  'logos_core_load_plugin': ['int', ['string']],
  'logos_core_get_loaded_plugins': ['pointer', []],
  'logos_core_get_plugin_methods': ['pointer', ['string']],
  'logos_core_call_plugin_method': ['pointer', ['string', 'string', 'string', 'string', WakuCallbackType]],
  'logos_core_get_plugin': ['pointer', ['string']],
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
// Alternative approach using the getPlugin function - advanced FFI for Qt objects
console.log("\n\n\n\n=== Using getPlugin with advanced FFI setup ===");

try {
  // Get the calculator plugin pointer
  const calculatorPtr = LogosCore.logos_core_get_plugin("calculator");
  
  // Define the CalculatorInterface virtual method table structure (vtable)
  // In C++ classes with virtual methods, the first bytes of the object contain a pointer to the vtable
  // We need to create bindings for the methods we want to call

  // Create a function pointer type for the add method
  // Virtual method layout: first entry is destructor, then virtual methods in order
  // CalculatorInterface inherits from PluginInterface, so we need to account for those virtual methods too
  
  // Assuming PluginInterface has some virtual methods before CalculatorInterface methods
  // This is an approximation, you might need to adjust offsets based on actual class hierarchy
  
  // Get the actual vtable pointer (first field in the object)
  const vtablePtr = ref.readPointer(calculatorPtr, 0);
  
  // Define the add method function pointer (offset in vtable depends on inheritance hierarchy)
  // This is an educated guess - you may need to adjust based on actual vtable layout
  // Usually for a simple inheritance: destructor (0), then PluginInterface methods, then CalculatorInterface methods
  const calculatorAddFn = ffi.ForeignFunction(
    ref.readPointer(vtablePtr, 14 * ref.sizeof.pointer), // Offset: position in vtable (adjust based on actual class)
    'int',  // Return type
    ['pointer', 'int', 'int'] // Arguments: this pointer, a, b
  );
  
  // Call the add method with values 2 and 3
  const result = calculatorAddFn(calculatorPtr, 2, 3);
  console.log(`Calculator add(2, 3) result: ${result}`);
  
} catch (err) {
  console.error('Error in direct plugin method calling:', err);
}

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