const { contextBridge, ipcRenderer } = require('electron');

contextBridge.exposeInMainWorld('walletAPI', {
  initialize: () => ipcRenderer.invoke('initialize-wallet-app'),
  initWallet: (config) => ipcRenderer.invoke('wallet-init', config),
  chainId: (rpcUrl) => ipcRenderer.invoke('wallet-chain-id', rpcUrl),
  ethBalance: (rpcUrl, address) => ipcRenderer.invoke('wallet-eth-balance', rpcUrl, address)
});

