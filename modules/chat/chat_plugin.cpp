#include "chat_plugin.h"
#include <QTimer>
#include <QDateTime>

ChatPlugin::ChatPlugin() : currentRelayTopic("/waku/2/rs/16/32"), logosAPIClient(nullptr), logosAPIProvider(nullptr) {
    logosAPIClient = new LogosAPIClient("waku_module", "app", this);
    logosAPIProvider = new LogosAPIProvider("waku_module", this);
}

ChatPlugin::~ChatPlugin() {
    // Clean up any resources if needed
    if (logosAPIClient) {
        delete logosAPIClient;
        logosAPIClient = nullptr;
    }
    if (logosAPIProvider) {
        delete logosAPIProvider;
        logosAPIProvider = nullptr;
    }
}

bool ChatPlugin::initialize() {
    MessageCallback actualCallback = [this](const std::string& timestamp, const std::string& nick, const std::string& message) {
        QVariantList data;
        data << QString::fromStdString(timestamp) << QString::fromStdString(nick) << QString::fromStdString(message);

        // logosAPIProvider->onEventResponse(this, "chatMessage", data);
    };

    void* result = ::initAndStart(logosAPIClient, currentRelayTopic, actualCallback);

    return (result != nullptr);
}

bool ChatPlugin::joinChannel(const QString& channelName) {
    return ::joinChannel(logosAPIClient, channelName.toStdString(), currentRelayTopic);
}

void ChatPlugin::sendMessage(const QString& channelName, const QString& username, const QString& message) {
    // print method arguments
    std::cout << "ChatPlugin::sendMessage called with channelName: " << channelName.toStdString() << ", username: " << username.toStdString() << ", message: " << message.toStdString() << std::endl;
    ::sendMessage(logosAPIClient, channelName.toStdString(), username.toStdString(), message.toStdString());
}

bool ChatPlugin::retrieveHistory(const std::string& channelName) {
    MessageCallback actualCallback = [this](const std::string& timestamp, const std::string& nick, const std::string& message) {
        QVariantList data;
        data << QString::fromStdString(timestamp) << QString::fromStdString(nick) << QString::fromStdString(message);

        // logosAPIProvider->onEventResponse(this, "historyMessage", data);
    };

    ::retrieveHistory(logosAPIClient, channelName, actualCallback);
    return true; // Assume success for now
}

bool ChatPlugin::retrieveHistory(const QString& channelName) {
    return retrieveHistory(channelName.toStdString());
}
