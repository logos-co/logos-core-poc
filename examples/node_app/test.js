const ffi = require('ffi-napi');
const path = require('path');
const fs = require('fs');

console.log('Testing FFI setup for Logos Core...\n');

// Check if the library exists
const libExtension = process.platform === 'darwin' ? '.dylib' : '.so';
const libPath = path.resolve(__dirname, '../../logos-liblogos/build/lib', `liblogos_core${libExtension}`);

console.log(`Platform: ${process.platform}`);
console.log(`Library extension: ${libExtension}`);
console.log(`Looking for library at: ${libPath}`);

if (!fs.existsSync(libPath)) {
    console.error('❌ Library file not found!');
    console.error('Please build the core library first by running: ./scripts/run_core.sh build');
    process.exit(1);
}

console.log('✅ Library file found');

// Try to load the library with minimal functions
try {
    console.log('\nTesting library loading...');
    
    const LogosCore = ffi.Library(libPath, {
        'logos_core_init': ['void', ['int', 'pointer']],
        'logos_core_cleanup': ['void', []]
    });
    
    console.log('✅ Library loaded successfully');
    
    // Test basic initialization
    console.log('\nTesting basic initialization...');
    LogosCore.logos_core_init(0, null);
    console.log('✅ Core initialization successful');
    
    // Test cleanup
    console.log('\nTesting cleanup...');
    LogosCore.logos_core_cleanup();
    console.log('✅ Core cleanup successful');
    
    console.log('\n🎉 All tests passed! The FFI setup is working correctly.');
    console.log('You can now run the full example with: npm start');
    
} catch (error) {
    console.error('❌ Error loading or testing library:', error.message);
    console.error('\nThis could be due to:');
    console.error('1. Missing dependencies in the library');
    console.error('2. Architecture mismatch');
    console.error('3. Missing Qt libraries');
    console.error('\nTry rebuilding the core library and ensure all dependencies are installed.');
    process.exit(1);
} 