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
  'logos_core_cleanup': ['void', []]
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