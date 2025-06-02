#include "chat_plugin.h"
#include "../../core/plugin_registry.h"

ChatPlugin::ChatPlugin() : currentRelayTopic("/waku/2/rs/16/32"), wakuPlugin(nullptr), logosAPI(nullptr) {
    // Get the waku plugin from the PluginRegistry
    wakuPlugin = PluginRegistry::getPlugin<WakuInterface>("waku");
    
    // Initialize the Logos API
    logosAPI = new LogosAPI("local:logoscore_registry", this);
}

ChatPlugin::~ChatPlugin() {
    // Clean up any resources if needed
    if (logosAPI) {
        delete logosAPI;
        logosAPI = nullptr;
    }
}

bool ChatPlugin::initialize(MessageCallback messageCallback) {
    // Create a message callback that emits the signal
    MessageCallback actualCallback = [this](const std::string& timestamp, const std::string& nick, const std::string& message) {
        // TODO: this later will be LogosAPI.emit...
        // Emit the eventResponse signal with QVariantList
        QVariantList data;
        data << QString::fromStdString(timestamp) << QString::fromStdString(nick) << QString::fromStdString(message);
        emit eventResponse(data);
    };
    
    // Use the provided callback if given, otherwise use our signal-emitting callback
    MessageCallback callbackToUse = messageCallback ? messageCallback : actualCallback;
    
    // Initialize and start Waku with the callback
    void* result = ::initAndStart(currentRelayTopic, callbackToUse);
    
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