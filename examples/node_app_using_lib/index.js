const LogosAPI = require('logos-api');

// Example using the Logos API SDK
console.log('🚀 Starting Logos API SDK Example');

async function runExample() {
  let logos;
  
  try {
    // Initialize LogosAPI (now self-contained with its own libraries)
    console.log('📦 Initializing LogosAPI...');
    logos = new LogosAPI({
      autoInit: true // Let it auto-initialize using its own libraries
    });
    
    // Start the system
    console.log('▶️  Starting LogosCore...');
    logos.start();
    
    console.log('✅ Logos Core initialized successfully!');
    console.log('🎉 Hello World from Logos Core Node.js SDK example!');
    
    // Check initial plugin status
    console.log('\n📊 Initial Plugin Status:');
    const initialStatus = logos.getPluginStatus();
    console.log(`  Loaded plugins (${initialStatus.loaded.length}):`, initialStatus.loaded);
    console.log(`  Known plugins (${initialStatus.known.length}):`, initialStatus.known);
    
    // Load some plugins using the SDK
    console.log('\n🔌 Loading plugins using SDK...');
    const pluginsToLoad = ['waku_module', 'chat', 'template_module'];
    const results = logos.processAndLoadPlugins(pluginsToLoad);
    
    // Display results
    console.log('\nPlugin loading results:');
    for (const [pluginName, result] of Object.entries(results)) {
      if (result.loaded) {
        console.log(`  ✅ ${pluginName}: Successfully loaded`);
      } else {
        console.log(`  ❌ ${pluginName}: Failed - ${result.error || 'Unknown error'}`);
      }
    }
    
    // Check final plugin status
    console.log('\n📊 Final Plugin Status:');
    const finalStatus = logos.getPluginStatus();
    console.log(`  Loaded plugins (${finalStatus.loaded.length}):`, finalStatus.loaded);
    console.log(`  Known plugins (${finalStatus.known.length}):`, finalStatus.known);
    
    // ============================================
    // ASYNC CALLBACK TESTING using SDK
    // ============================================
    
    console.log('\n🚀 Testing Async Callbacks via SDK');
    console.log('===================================');
    
    // Test: Register event listener for fooTriggered event using SDK
    console.log('\n🧪 TEST: Register Event Listener via SDK');
    console.log('Registering event listener for template_module::fooTriggered');
    
    logos.registerEventListener('template_module', 'fooTriggered', (success, message, meta) => {
      const timestamp = meta.timestamp;
      if (success) {
        console.log(`🎉 [${timestamp}] EVENT RECEIVED via SDK:`, message);
        
        // Try to parse the message as JSON to see event data
        try {
          if (typeof message === 'object' && message.event) {
            console.log(`🎊 [${timestamp}] EVENT NAME: ${message.event}`);
            console.log(`📊 [${timestamp}] EVENT DATA:`, message.data);
          } else {
            console.log(`📝 [${timestamp}] RAW EVENT MESSAGE:`, message);
          }
        } catch (e) {
          console.log(`📝 [${timestamp}] RAW EVENT MESSAGE:`, message);
        }
      } else {
        console.log(`🚫 [${timestamp}] EVENT LISTENER FAILED:`, message);
      }
    });
    
    // Test: Call plugin method using SDK
    setTimeout(() => {
      if (finalStatus.loaded.includes('template_module')) {
        console.log('\n🧪 TEST: Plugin Method Call via SDK');
        console.log('Calling template_module::foo() with string parameter via SDK');

        const stringParams = JSON.stringify([
          {
            name: "bar",
            value: "Hello from Logos API SDK!",
            type: "string"
          }
        ]);

        console.log('Making method call via SDK...');
        logos.callPluginMethodAsync('template_module', 'foo', stringParams, (success, message, meta) => {
          const timestamp = meta.timestamp;
          if (success) {
            console.log(`🎯 [${timestamp}] METHOD SUCCESS via SDK:`, message);
          } else {
            console.log(`🚫 [${timestamp}] METHOD FAILED via SDK:`, message);
          }
        });
      } else {
        console.log('\n⚠️  template_module not loaded, skipping method call test');
      }
    }, 5000); // Wait 5 seconds for the event listener to be ready
    
    // Start event processing using SDK
    console.log('\n⚡ Starting event processing via SDK...');
    logos.startEventProcessing(50); // Process every 50ms for responsive event handling
    
    // Keep the process alive for a while to see all callbacks
    console.log('\n⏳ Waiting for async operations to complete...');
    console.log('   (Method call should complete soon)');
    
    // Keep the process alive longer to see the async callbacks
    setTimeout(() => {
      console.log('\n⏱️  Still waiting for callbacks...');
    }, 8000);
    
    setTimeout(() => {
      console.log('\n🎉 Async test period completed via SDK');
      console.log('Note: Process will continue running. Press Ctrl+C to exit.');
    }, 15000);
    
  } catch (error) {
    console.error('💥 Error in SDK example:', error.message);
    console.error('Stack trace:', error.stack);
    
    if (logos) {
      logos.cleanup();
    }
    
    process.exit(1);
  }
}

// Handle process termination gracefully
process.on('SIGINT', () => {
  console.log('\n🛑 Interrupted! Cleaning up via SDK...');
  // The LogosAPI will handle cleanup automatically
  console.log('✨ Cleanup completed via SDK!');
  process.exit(0);
});

process.on('SIGTERM', () => {
  console.log('\n🛑 Terminated! Cleaning up via SDK...');
  process.exit(0);
});

// Run the example
runExample();
