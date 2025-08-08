// This file is required by the index.html file and will
// be executed in the renderer process for that window.

// Chat application renderer
document.addEventListener('DOMContentLoaded', () => {
    // UI Elements
    const loadingScreen = document.getElementById('loading-screen');
    const chatInterface = document.getElementById('chat-interface');
    const statusBar = document.getElementById('status-bar');
    const userInfo = document.getElementById('user-info');
    const channelInput = document.getElementById('channel-input');
    const joinBtn = document.getElementById('join-btn');
    const chatMessages = document.getElementById('chat-messages');
    const messageInput = document.getElementById('message-input');
    const sendBtn = document.getElementById('send-btn');

    // Chat state
    let currentUsername = '';
    let currentChannel = '';
    let chatReady = false;
    const pendingMessages = [];
    const seenHistory = new Set();

    // Initialize the chat application
    async function initializeChatApp() {
        try {
            console.log('Initializing chat application...');
            const result = await window.electronAPI.initializeChatApp();
            
            if (result.success) {
                console.log('Chat app initialized successfully');
                currentUsername = result.username;
                updateUserInfo(currentUsername);
            } else {
                console.error('Failed to initialize chat app:', result.error);
                updateStatus(`Error: ${result.error}`, 'error');
            }
        } catch (error) {
            console.error('Error initializing chat app:', error);
            updateStatus(`Error: ${error.message}`, 'error');
        }
    }

    // Update status bar
    function updateStatus(message, type = 'info') {
        statusBar.textContent = message;
        statusBar.className = 'status-bar';
        if (type === 'error') {
            statusBar.classList.add('error');
        } else if (type === 'ready') {
            statusBar.classList.add('ready');
        }
    }

    // Update user info
    function updateUserInfo(username) {
        userInfo.textContent = `User: ${username}`;
    }

    // Add message to chat display
    // options: { pending: boolean, pendingId: string }
    function addMessage(sender, content, type = 'other', timestamp = null, options = {}) {
        const messageDiv = document.createElement('div');
        const isPending = !!options.pending;
        const classes = ['message', type];
        if (isPending) classes.push('pending');
        messageDiv.className = classes.join(' ');
        if (isPending) {
            messageDiv.dataset.pending = 'true';
            messageDiv.dataset.messageText = content;
            if (options.pendingId) messageDiv.dataset.pendingId = options.pendingId;
        }
        
        const now = timestamp ? new Date(timestamp) : new Date();
        const timeStr = now.toLocaleTimeString();
        
        let messageHTML = '';
        
        if (type === 'system') {
            messageHTML = `<div class="message-content">${content}</div>`;
        } else {
            if (type === 'history') {
                messageHTML = `
                    <div class="message-header">[HISTORY] ${timeStr} - ${sender}</div>
                    <div class="message-content">${content}</div>
                `;
            } else {
                const pendingBadge = isPending ? ' <span class="badge">(sending...)</span>' : '';
                messageHTML = `
                    <div class="message-header">${timeStr} - ${sender}${pendingBadge}</div>
                    <div class="message-content">${content}</div>
                `;
            }
        }
        
        messageDiv.innerHTML = messageHTML;
        const shouldStickToBottom = chatMessages.scrollTop + chatMessages.clientHeight >= chatMessages.scrollHeight - 20;
        chatMessages.appendChild(messageDiv);
        if (shouldStickToBottom || type !== 'history') {
            chatMessages.scrollTop = chatMessages.scrollHeight;
        }
        return messageDiv;
    }

    // Clear chat messages
    function clearMessages() {
        chatMessages.innerHTML = '';
        chatMessages.scrollTop = 0;
    }

    // Enable/disable UI elements
    function setUIEnabled(enabled) {
        channelInput.disabled = !enabled;
        joinBtn.disabled = !enabled;
        messageInput.disabled = !enabled;
        sendBtn.disabled = !enabled;
    }

    // Show chat interface
    function showChatInterface() {
        loadingScreen.classList.add('hidden');
        chatInterface.classList.remove('hidden');
    }

    // Send message
    async function sendMessage() {
        const message = messageInput.value.trim();
        if (!message || !chatReady) return;

        try {
            messageInput.disabled = true;
            sendBtn.disabled = true;
            updateStatus('Sending message...', 'info');

            // Show a pending message bubble immediately
            const pendingId = `p_${Date.now()}_${Math.random().toString(36).slice(2)}`;
            const pendingEl = addMessage(currentUsername, message, 'own', null, { pending: true, pendingId });
            pendingMessages.push({ id: pendingId, text: message, el: pendingEl, createdAt: Date.now() });
            
            const result = await window.electronAPI.sendMessage(message);
            
            if (result.success) {
                // Clear input
                messageInput.value = '';
                // Note: The actual message will appear via the chat event
                // Keep status as sending until event arrives; fallback clear later
            } else {
                console.error('Failed to send message:', result.error);
                addMessage('System', `Failed to send message: ${result.error}`, 'system');
                // Mark last pending (matching text) as failed
                const idx = pendingMessages.findIndex(p => p.text === message);
                if (idx !== -1) {
                    const el = pendingMessages[idx].el;
                    const header = el.querySelector('.message-header');
                    if (header) header.innerHTML += ' <span class="badge error">(failed)</span>';
                    el.classList.remove('pending');
                    el.classList.add('failed');
                    pendingMessages.splice(idx, 1);
                }
                updateStatus('Failed to send message', 'error');
            }
        } catch (error) {
            console.error('Error sending message:', error);
            addMessage('System', `Error sending message: ${error.message}`, 'system');
            updateStatus('Error sending message', 'error');
        } finally {
            if (chatReady) {
                messageInput.disabled = false;
                sendBtn.disabled = false;
                messageInput.focus();
            }
        }
    }

    // Join channel
    async function joinChannel() {
        const channelName = channelInput.value.trim();
        if (!channelName || !chatReady) return;

        try {
            joinBtn.disabled = true;
            updateStatus(`Joining channel: ${channelName}...`);
            
            // Clear messages when joining new channel
            clearMessages();
            addMessage('System', `Joining channel: ${channelName}`, 'system');
            
            const result = await window.electronAPI.joinChannel(channelName);
            
            if (result.success) {
                currentChannel = result.currentChannel;
                updateStatus(`Connected to channel: ${currentChannel}`, 'ready');
                addMessage('System', `You have joined channel: ${currentChannel}`, 'system');
                addMessage('System', '--- Message History ---', 'system');
                // Reset history duplicate tracker for new channel
                seenHistory.clear();
            } else {
                console.error('Failed to join channel:', result.error);
                updateStatus(`Failed to join channel: ${result.error}`, 'error');
                addMessage('System', `Failed to join channel: ${result.error}`, 'system');
            }
        } catch (error) {
            console.error('Error joining channel:', error);
            updateStatus(`Error: ${error.message}`, 'error');
            addMessage('System', `Error joining channel: ${error.message}`, 'system');
        } finally {
            joinBtn.disabled = false;
        }
    }

    // Event Listeners
    sendBtn.addEventListener('click', sendMessage);
    joinBtn.addEventListener('click', joinChannel);
    
    messageInput.addEventListener('keypress', (e) => {
        if (e.key === 'Enter') {
            sendMessage();
        }
    });
    
    channelInput.addEventListener('keypress', (e) => {
        if (e.key === 'Enter') {
            joinChannel();
        }
    });

    // Set up IPC event listeners
    window.electronAPI.onChatStatus((event, data) => {
        console.log('Chat status update:', data);
        
        if (data.username) {
            currentUsername = data.username;
            updateUserInfo(currentUsername);
        }
        
        if (data.status) {
            updateStatus(data.status, data.error ? 'error' : (data.ready ? 'ready' : 'info'));
        }
        
        if (data.ready) {
            chatReady = true;
            currentChannel = data.currentChannel || '';
            channelInput.value = currentChannel;
            setUIEnabled(true);
            showChatInterface();
            messageInput.focus();
            
            // Add welcome message
            addMessage('System', `Welcome to Logos Chat! You are connected to channel: ${currentChannel}`, 'system');
        }
        
        if (data.error) {
            chatReady = false;
            setUIEnabled(false);
        }
    });

    window.electronAPI.onChatEvent((event, data) => {
        console.log('Chat event received:', data);
        
        if (data.success === false) {
            addMessage('System', `Event error: ${data.message}`, 'system');
            return;
        }

        if (data.eventName === 'chatMessage') {
            const eventData = Array.isArray(data.data) ? data.data : [data.data];
            if (eventData.length >= 3) {
                const timestamp = eventData[0];
                const nick = eventData[1];
                const message = eventData[2];
                
                // Check if this is your own message or from another user
                const isOwn = (nick === currentUsername);
                if (isOwn) {
                    // Try to resolve a pending bubble instead of duplicating
                    const idx = pendingMessages.findIndex(p => p.text === message);
                    if (idx !== -1) {
                        const el = pendingMessages[idx].el;
                        const header = el.querySelector('.message-header');
                        if (header) {
                            const date = new Date(timestamp);
                            const timeStr = date.toLocaleTimeString();
                            // Strip any existing badge and update timestamp
                            header.innerHTML = `${timeStr} - ${nick}`;
                        }
                        el.classList.remove('pending');
                        pendingMessages.splice(idx, 1);
                        updateStatus('Message sent', 'ready');
                        return;
                    }
                }
                const messageType = isOwn ? 'own' : 'other';
                addMessage(nick, message, messageType, timestamp);
            }
        } else if (data.eventName === 'historyMessage') {
            const eventData = Array.isArray(data.data) ? data.data : [data.data];
            if (eventData.length >= 3) {
                const timestamp = eventData[0];
                const nick = eventData[1];
                const message = eventData[2];
                // Deduplicate history items (some backends may emit them multiple times)
                const key = `${timestamp}|${nick}|${message}`;
                if (seenHistory.has(key)) {
                    return; // skip duplicates
                }
                seenHistory.add(key);
                addMessage(nick, message, 'history', timestamp);
            }
        }
    });

    window.electronAPI.onChatMethodResult((event, data) => {
        console.log('Chat method result:', data);
        
        if (data.success) {
            console.log('Method success:', data.message);
        } else {
            console.error('Method failed:', data.message);
            addMessage('System', `Operation failed: ${data.message}`, 'system');
            // Mark any outstanding pending message as failed for visibility
            if (pendingMessages.length > 0) {
                const { el } = pendingMessages.shift();
                const header = el.querySelector('.message-header');
                if (header) header.innerHTML += ' <span class="badge error">(failed)</span>';
                el.classList.remove('pending');
                el.classList.add('failed');
            }
            updateStatus('Operation failed', 'error');
        }
    });

    // Initialize the application
    initializeChatApp();
}); 