#include "chat_plugin.h"
#include "../../core/plugin_registry.h"

ChatPlugin::ChatPlugin() : currentRelayTopic("/waku/2/rs/16/32"), wakuPlugin(nullptr) {
    // Get the waku plugin from the PluginRegistry
    wakuPlugin = PluginRegistry::getPlugin<WakuInterface>("waku");
}

ChatPlugin::~ChatPlugin() {
    // Clean up any resources if needed
}

bool ChatPlugin::initialize(MessageCallback messageCallback) {
    // Initialize and start Waku
    void* result = ::initAndStart(currentRelayTopic, messageCallback);
    
    // Return success/failure
    return (result != nullptr);
}

bool ChatPlugin::joinChannel(const QString& channelName) {
    // Convert QString to std::string for the underlying API
    return ::joinChannel(channelName.toStdString(), currentRelayTopic);
}

void ChatPlugin::sendMessage(const QString& channelName, const QString& username, const QString& message) {
    // Convert QString to std::string for the underlying API
    ::sendMessage(channelName.toStdString(), username.toStdString(), message.toStdString());
}

void ChatPlugin::retrieveHistory(const std::string& channelName, MessageCallback callback) {
    ::retrieveHistory(channelName, callback);
} 