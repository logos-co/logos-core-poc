const { contextBridge, ipcRenderer } = require('electron');

// Expose protected methods that allow the renderer process to use
// the ipcRenderer without exposing the entire object
contextBridge.exposeInMainWorld('electronAPI', {
  // Chat app initialization
  initializeChatApp: () => ipcRenderer.invoke('initialize-chat-app'),
  
  // Chat methods
  sendMessage: (message) => ipcRenderer.invoke('send-message', message),
  joinChannel: (channelName) => ipcRenderer.invoke('join-channel', channelName),
  
  // Event listeners for chat events
  onChatMethodResult: (callback) => ipcRenderer.on('chat-method-result', callback),
  onChatEvent: (callback) => ipcRenderer.on('chat-event', callback),
  onChatStatus: (callback) => ipcRenderer.on('chat-status', callback),
  
  // Remove event listeners
  removeChatMethodResultListener: (callback) => ipcRenderer.removeListener('chat-method-result', callback),
  removeChatEventListener: (callback) => ipcRenderer.removeListener('chat-event', callback),
  removeChatStatusListener: (callback) => ipcRenderer.removeListener('chat-status', callback)
}); 