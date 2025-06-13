#!/usr/bin/env node

const fs = require('fs');
const path = require('path');

function copyFileSync(src, dest) {
  try {
    // Ensure destination directory exists
    const destDir = path.dirname(dest);
    if (!fs.existsSync(destDir)) {
      fs.mkdirSync(destDir, { recursive: true });
    }
    
    // Copy file
    fs.copyFileSync(src, dest);
    return true;
  } catch (error) {
    console.warn(`Failed to copy ${src} to ${dest}:`, error.message);
    return false;
  }
}

function copyDirectorySync(src, dest) {
  try {
    // Ensure destination directory exists
    if (!fs.existsSync(dest)) {
      fs.mkdirSync(dest, { recursive: true });
    }
    
    // Read source directory
    const items = fs.readdirSync(src);
    
    for (const item of items) {
      const srcPath = path.join(src, item);
      const destPath = path.join(dest, item);
      
      const stat = fs.statSync(srcPath);
      if (stat.isDirectory()) {
        copyDirectorySync(srcPath, destPath);
      } else {
        copyFileSync(srcPath, destPath);
      }
    }
    
    return true;
  } catch (error) {
    console.warn(`Failed to copy directory ${src} to ${dest}:`, error.message);
    return false;
  }
}

function getLibraryExtension() {
  switch (process.platform) {
    case 'darwin':
      return '.dylib';
    case 'win32':
      return '.dll';
    default:
      return '.so';
  }
}

function main() {
  console.log('📦 Copying liblogos library to SDK...');
  
  const sdkDir = path.resolve(__dirname, '..');
  const coreDir = path.resolve(sdkDir, '../../../core/build');
  
  // Define paths
  const libExtension = getLibraryExtension();
  const libSrc = path.join(coreDir, 'lib', `liblogos_core${libExtension}`);
  const libDest = path.join(sdkDir, 'lib', `liblogos_core${libExtension}`);
  
  let copySuccess = true;
  
  // Copy library
  console.log(`📚 Copying library from ${libSrc}...`);
  if (fs.existsSync(libSrc)) {
    if (copyFileSync(libSrc, libDest)) {
      console.log(`✅ Library copied to ${libDest}`);
    } else {
      copySuccess = false;
    }
  } else {
    console.warn(`⚠️  Library not found at ${libSrc}`);
    console.warn('   Please build the core library first: ./scripts/run_core.sh build');
    copySuccess = false;
  }
  
  if (copySuccess) {
    console.log('✨ SDK now has its own copy of liblogos_core!');
    console.log('📝 Note: Applications should have their own plugins directory.');
  } else {
    console.log('⚠️  Library could not be copied. The SDK will fall back to the original location.');
  }
}

if (require.main === module) {
  main();
}

module.exports = { copyFileSync, copyDirectorySync, getLibraryExtension }; 