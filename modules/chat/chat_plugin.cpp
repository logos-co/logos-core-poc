#include "chat_plugin.h"
#include <QTimer>
#include <QDateTime>

ChatPlugin::ChatPlugin() : currentRelayTopic("/waku/2/rs/16/32"), logosAPI(nullptr) {
    logosAPI = new LogosAPI("waku_module", this);
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

    void* result = ::initAndStart(logosAPI, currentRelayTopic, actualCallback);

    return (result != nullptr);
}

bool ChatPlugin::joinChannel(const QString& channelName) {
    return ::joinChannel(logosAPI, channelName.toStdString(), currentRelayTopic);
}

void ChatPlugin::sendMessage(const QString& channelName, const QString& username, const QString& message) {
    ::sendMessage(logosAPI, channelName.toStdString(), username.toStdString(), message.toStdString());
}

bool ChatPlugin::retrieveHistory(const std::string& channelName) {
    MessageCallback actualCallback = [this](const std::string& timestamp, const std::string& nick, const std::string& message) {
        QVariantList data;
        data << QString::fromStdString(timestamp) << QString::fromStdString(nick) << QString::fromStdString(message);

        logosAPI->onEventResponse(this, "historyMessage", data);
    };

    ::retrieveHistory(logosAPI, channelName, actualCallback);
    return true; // Assume success for now
}

bool ChatPlugin::retrieveHistory(const QString& channelName) {
    return retrieveHistory(channelName.toStdString());
}
