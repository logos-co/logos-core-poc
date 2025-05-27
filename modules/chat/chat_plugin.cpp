#include "chat_plugin.h"
#include "../../core/plugin_registry.h"

ChatPlugin::ChatPlugin() : wakuCtx(nullptr), currentRelayTopic("/waku/2/rs/16/32"), wakuPlugin(nullptr) {
    // Get the waku plugin from the PluginRegistry
    wakuPlugin = PluginRegistry::getPlugin<WakuInterface>("waku");
}

ChatPlugin::~ChatPlugin() {
    // Clean up any resources if needed
    if (wakuCtx != nullptr) {
        // Cleanup code could go here if needed
        wakuCtx = nullptr;
    }
}

bool ChatPlugin::initialize(MessageCallback messageCallback) {
    // Initialize and start Waku
    wakuCtx = ::initAndStart(currentRelayTopic, messageCallback);
    
    // Return success/failure
    return (wakuCtx != nullptr);
}

bool ChatPlugin::joinChannel(const QString& channelName) {
    if (wakuCtx == nullptr) {
        return false;
    }
    
    // Convert QString to std::string for the underlying API
    return ::joinChannel(wakuCtx, channelName.toStdString(), currentRelayTopic);
}

void ChatPlugin::sendMessage(const QString& channelName, const QString& username, const QString& message) {
    if (wakuCtx == nullptr) {
        return;
    }
    
    // Convert QString to std::string for the underlying API
    ::sendMessage(wakuCtx, channelName.toStdString(), username.toStdString(), message.toStdString());
}

void ChatPlugin::retrieveHistory(const std::string& channelName, MessageCallback callback) {
    if (wakuCtx == nullptr) {
        return;
    }
    
    ::retrieveHistory(wakuCtx, channelName, callback);
} 