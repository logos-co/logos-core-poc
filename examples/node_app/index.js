const ffi = require('ffi-napi');
const path = require('path');
const fs = require('fs');

// Determine library file extension based on platform
const libExtension = process.platform === 'darwin' ? '.dylib' : 
                    (process.platform === 'win32' ? '.dll' : '.so');

// Construct path to the liblogos_core library
// This assumes the library is in ../../core/build/lib relative to this script
const libPath = path.resolve(__dirname, '../../core/build/lib', `liblogos_core${libExtension}`);

console.log(`Looking for liblogos_core at: ${libPath}`);

// Check if the library exists
if (!fs.existsSync(libPath)) {
  console.error(`Error: Library not found at ${libPath}`);
  console.error('Please make sure to build the logos_core library first.');
  process.exit(1);
}

// Define the logos_core library interface
const LogosCore = ffi.Library(libPath, {
  'logos_core_init': ['void', ['int', 'pointer']],
  'logos_core_set_plugins_dir': ['void', ['string']],
  'logos_core_start': ['void', []],
  'logos_core_cleanup': ['void', []],
  'logos_core_get_loaded_plugins': ['pointer', []]
});

console.log('Hello from Node.js!');
console.log('Initializing logos_core...');

// Initialize logos_core
// Since we can't directly pass argv, we'll pass null for now
LogosCore.logos_core_init(0, null);

// Set plugins directory
const pluginsDir = path.resolve(__dirname, '../../core/build/plugins');
console.log(`Setting plugins directory to: ${pluginsDir}`);
LogosCore.logos_core_set_plugins_dir(pluginsDir);

// Start logos_core
console.log('Starting logos_core...');
LogosCore.logos_core_start();

console.log('Logos Core initialized successfully!');

// Note: In a real application, you would keep the process running
// and call logos_core_cleanup() when done

// For this simple example, we'll just wait a bit and then clean up
//setTimeout(() => {
//  console.log('Cleaning up logos_core...');
//  LogosCore.logos_core_cleanup();
//  console.log('Done!');
//}, 2000); 