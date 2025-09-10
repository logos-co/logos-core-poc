const { app, BrowserWindow, ipcMain } = require('electron');
const path = require('path');
const fs = require('fs');
const LogosAPI = require('logos-api');

let mainWindow;
let logos = null;

function createWindow() {
  mainWindow = new BrowserWindow({
    width: 1000,
    height: 800,
    webPreferences: {
      nodeIntegration: false,
      contextIsolation: true,
      preload: path.join(__dirname, 'preload.js')
    }
  });
  mainWindow.loadFile('index.html');
  mainWindow.on('closed', function () {
    mainWindow = null;
  });
}

app.whenReady().then(createWindow);

app.on('window-all-closed', function () {
  if (process.platform !== 'darwin') app.quit();
});

app.on('activate', function () {
  if (mainWindow === null) createWindow();
});

app.on('before-quit', () => {
  if (logos) {
    try { logos.cleanup(); } catch (_) {}
  }
});

function startEventProcessing() {
  if (logos) {
    logos.startEventProcessing(50);
  }
}

ipcMain.handle('initialize-wallet-app', async () => {
  try {
    const logosHostPath = path.resolve(
      __dirname,
      '../../core/build/bin',
      process.platform === 'win32' ? 'logos_host.exe' : 'logos_host'
    );
    process.env.LOGOS_HOST_PATH = logosHostPath;

    const libExtension = process.platform === 'darwin' ? '.dylib' : (process.platform === 'win32' ? '.dll' : '.so');
    const libPath = path.resolve(__dirname, '../../core/build/lib', `liblogos_core${libExtension}`);
    const pluginsDir = path.resolve(__dirname, '../../core/build/modules');

    if (!fs.existsSync(libPath)) {
      throw new Error(`Library file not found at: ${libPath}. Please build the core library first by running: ./scripts/run_core.sh build`);
    }

    logos = new LogosAPI({ libPath, pluginsDir, autoInit: true });
    logos.start();

    const pluginsToLoad = ['capability_module', 'wallet_module'];

    const results = logos.processAndLoadPlugins(pluginsToLoad);

    startEventProcessing();

    return {
      success: true,
      message: 'Wallet app initialized successfully!',
      pluginStatus: logos.getPluginStatus(),
      pluginResults: results
    };
  } catch (error) {
    return { success: false, error: error.message };
  }
});

// Wallet operations
ipcMain.handle('wallet-init', async (event, config) => {
  try {
    const resp = await logos.wallet_module.initWallet(JSON.stringify(config || {}));
    return { success: true, message: resp };
  } catch (e) {
    return { success: false, error: e };
  }
});

ipcMain.handle('wallet-chain-id', async (event, rpcUrl) => {
  try {
    const resp = await logos.wallet_module.chainId(rpcUrl);
    return { success: true, message: resp };
  } catch (e) {
    return { success: false, error: e };
  }
});

ipcMain.handle('wallet-eth-balance', async (event, rpcUrl, address) => {
  try {
    const resp = await logos.wallet_module.getEthBalance(rpcUrl, address);
    return { success: true, message: resp };
  } catch (e) {
    return { success: false, error: e };
  }
});

