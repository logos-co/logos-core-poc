#include "chat_plugin.h"
#include <QTimer>
#include <QDateTime>

ChatPlugin::ChatPlugin() : currentRelayTopic("/waku/2/rs/16/32"), logosAPI(nullptr) {
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

        logosAPI->getClient("waku_module")->onEventResponse(this, "chatMessage", data);
    };

    void* result = ::initAndStart(logosAPI->getClient("waku_module"), currentRelayTopic, actualCallback);

    return (result != nullptr);
}

bool ChatPlugin::joinChannel(const QString& channelName) {
    return ::joinChannel(logosAPI->getClient("waku_module"), channelName.toStdString(), currentRelayTopic);
}

void ChatPlugin::sendMessage(const QString& channelName, const QString& username, const QString& message) {
    // print method arguments
    std::cout << "ChatPlugin::sendMessage called with channelName: " << channelName.toStdString() << ", username: " << username.toStdString() << ", message: " << message.toStdString() << std::endl;
    ::sendMessage(logosAPI->getClient("waku_module"), channelName.toStdString(), username.toStdString(), message.toStdString());
}

bool ChatPlugin::retrieveHistory(const std::string& channelName) {
    MessageCallback actualCallback = [this](const std::string& timestamp, const std::string& nick, const std::string& message) {
        QVariantList data;
        data << QString::fromStdString(timestamp) << QString::fromStdString(nick) << QString::fromStdString(message);

        logosAPI->getClient("waku_module")->onEventResponse(this, "historyMessage", data);
    };

    ::retrieveHistory(logosAPI->getClient("waku_module"), channelName, actualCallback);
    return true; // Assume success for now
}

bool ChatPlugin::retrieveHistory(const QString& channelName) {
    return retrieveHistory(channelName.toStdString());
}

void ChatPlugin::initLogos(LogosAPI* logosAPIInstance) {
    if (logosAPI) {
        delete logosAPI;
    }
    logosAPI = logosAPIInstance;
}
