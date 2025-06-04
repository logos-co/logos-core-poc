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

bool ChatPlugin::initialize() {
    MessageCallback actualCallback = [this](const std::string& timestamp, const std::string& nick, const std::string& message) {
        QVariantList data;
        data << QString::fromStdString(timestamp) << QString::fromStdString(nick) << QString::fromStdString(message);

        logosAPI->onEventResponse(this, "chatMessage", data);
    };

    // Initialize and start Waku with the callback
    void* result = ::initAndStart(currentRelayTopic, actualCallback);

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

bool ChatPlugin::retrieveHistory(const std::string& channelName) {
    MessageCallback actualCallback = [this](const std::string& timestamp, const std::string& nick, const std::string& message) {
        QVariantList data;
        data << QString::fromStdString(timestamp) << QString::fromStdString(nick) << QString::fromStdString(message);

        logosAPI->onEventResponse(this, "historyMessage", data);
    };

    ::retrieveHistory(channelName, actualCallback);
    return true; // Assume success for now
}

bool ChatPlugin::retrieveHistory(const QString& channelName) {
    // Convert QString to std::string and call the interface implementation
    return retrieveHistory(channelName.toStdString());
}
