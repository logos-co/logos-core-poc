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

function main() {
  console.log('🔌 Copying plugins to application...');
  
  const appDir = path.resolve(__dirname, '..');
  const coreDir = path.resolve(appDir, '../../logos-liblogos/build');
  
  // Define paths
  const pluginsSrc = path.join(coreDir, 'modules');
  const pluginsDest = path.join(appDir, 'plugins');
  
  // Copy plugins
  console.log(`📂 Copying plugins from ${pluginsSrc}...`);
  if (fs.existsSync(pluginsSrc)) {
    if (copyDirectorySync(pluginsSrc, pluginsDest)) {
      console.log(`✅ Plugins copied to ${pluginsDest}`);
      
      // List copied plugins
      try {
        const plugins = fs.readdirSync(pluginsDest);
        const pluginList = plugins.filter(p => p.endsWith('.dylib') || p.endsWith('.so') || p.endsWith('.dll'));
        console.log(`📋 Copied plugins: ${pluginList.join(', ')}`);
      } catch (error) {
        // Silent fail for listing
      }
      
      console.log('✨ Application plugins are ready!');
    } else {
      console.warn('⚠️  Failed to copy some plugins.');
    }
  } else {
    console.warn(`⚠️  Plugins directory not found at ${pluginsSrc}`);
    console.warn('   Please build the core library first: ./scripts/run_core.sh build');
    console.warn('   The application will fall back to alternative plugin locations.');
  }
}

if (require.main === module) {
  main();
}

module.exports = { copyFileSync, copyDirectorySync }; 