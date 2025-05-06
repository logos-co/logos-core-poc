const ffi = require('ffi-napi');
const ref = require('ref-napi');
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

// Call the waku plugin's initWaku method
console.log('Calling waku plugin initWaku method...');

// Create a JavaScript callback function
const wakuInitCallback = WakuCallbackType.toPointer((success, message) => {
  console.log('Waku initialization callback from JS:');
  console.log('  Success:', success);
  console.log('  Message:', message);
});

// Create the configuration string
const wakuConfig = {
  host: "0.0.0.0",
  tcpPort: 60010,
  key: null,
  clusterId: 16,
  relay: true,
  relayTopics: ["/waku/2/rs/16/32"],
  shards: [1, 32, 64, 128, 256],
  maxMessageSize: "1024KiB",
  dnsDiscovery: true,
  dnsDiscoveryUrl: "enrtree://AMOJVZX4V6EXP7NTJPMAYJYST2QP6AJXYW76IU6VGJS7UVSNDYZG4@boot.prod.status.nodes.status.im",
  discv5Discovery: false,
  discv5EnrAutoUpdate: false,
  logLevel: "INFO",
  keepAlive: true
};

// Create the parameters array for the method call
const initWakuParams = [
  {
    name: "cfg",
    type: "QString",
    value: JSON.stringify(wakuConfig)
  },
  {
    name: "callback",
    type: "WakuInitCallback",
    value: null  // This won't be used directly, but is needed to match the method signature
  }
];

// Call the method with our JavaScript callback
const resultPtr = LogosCore.logos_core_call_plugin_method(
  "waku",                            // plugin name
  "initWaku",                        // method name
  JSON.stringify(initWakuParams),    // parameters as JSON
  "void",                            // return type hint
  wakuInitCallback                   // JavaScript callback
);

// Check if the call was successful
if (resultPtr) {
  const resultString = resultPtr.readCString();
  const result = JSON.parse(resultString);
  
  console.log('Result of initWaku call:', result);
  
  // Free the memory allocated for the result
  LogosCore.free(resultPtr);
} else {
  console.error('Failed to call initWaku method');
}

// Wait a few seconds for the callback to execute
console.log('Waiting for callback to execute...');

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