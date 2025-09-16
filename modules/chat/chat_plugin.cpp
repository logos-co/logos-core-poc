#include "chat_plugin.h"
#include <QTimer>
#include <QDateTime>
#include <QDebug>
#include <QString>
#include "../../SDK/cpp/logos_api_client.h"

ChatPlugin::ChatPlugin() : currentRelayTopic("/waku/2/rs/16/32") {
}

ChatPlugin::~ChatPlugin() {
    if (logos) {
        delete logos;
        logos = nullptr;
    }
    if (logosAPI) {
        delete logosAPI;
        logosAPI = nullptr;
    }
}

bool ChatPlugin::ensureLogosContext(const char* caller) const {
    const QString context = QString::fromLatin1(caller ? caller : "unknown");

    if (!logosAPI) {
        qWarning() << "ChatPlugin:" << context << "- LogosAPI not initialized";
        return false;
    }

    if (!logos) {
        qWarning() << "ChatPlugin:" << context << "- LogosModules not initialized";
        return false;
    }

    return true;
}

bool ChatPlugin::initialize() {
    if (!ensureLogosContext("initialize")) {
        return false;
    }

    MessageCallback actualCallback = [this](const std::string& timestamp, const std::string& nick, const std::string& message) {
        QVariantList data;
        data << QString::fromStdString(timestamp) << QString::fromStdString(nick) << QString::fromStdString(message);

        emitEvent(QStringLiteral("chatMessage"), data);
    };

    void* result = ::initAndStart(logosAPI, logos, currentRelayTopic, actualCallback);

    return (result != nullptr);
}

bool ChatPlugin::joinChannel(const QString& channelName) {
    if (!ensureLogosContext("joinChannel")) {
        return false;
    }
    return ::joinChannel(logosAPI, logos, channelName.toStdString(), currentRelayTopic);
}

void ChatPlugin::sendMessage(const QString& channelName, const QString& username, const QString& message) {
    if (!ensureLogosContext("sendMessage")) {
        return;
    }
    std::cout << "ChatPlugin::sendMessage called with channelName: " << channelName.toStdString()
              << ", username: " << username.toStdString()
              << ", message: " << message.toStdString() << std::endl;
    ::sendMessage(logosAPI, logos, channelName.toStdString(), username.toStdString(), message.toStdString());
}

bool ChatPlugin::retrieveHistory(const std::string& channelName) {
    if (!ensureLogosContext("retrieveHistory")) {
        return false;
    }

    MessageCallback actualCallback = [this](const std::string& timestamp, const std::string& nick, const std::string& message) {
        QVariantList data;
        data << QString::fromStdString(timestamp) << QString::fromStdString(nick) << QString::fromStdString(message);

        emitEvent(QStringLiteral("historyMessage"), data);
    };

    ::retrieveHistory(logosAPI, logos, channelName, actualCallback);
    return true;
}

bool ChatPlugin::retrieveHistory(const QString& channelName) {
    return retrieveHistory(channelName.toStdString());
}

void ChatPlugin::initLogos(LogosAPI* logosAPIInstance) {
    if (logos) {
        delete logos;
        logos = nullptr;
    }
    if (logosAPI) {
        delete logosAPI;
        logosAPI = nullptr;
    }
    logosAPI = logosAPIInstance;
    if (logosAPI) {
        logos = new LogosModules(logosAPI);
    }
}

void ChatPlugin::emitEvent(const QString& eventName, const QVariantList& data) {
    if (!logosAPI) {
        qWarning() << "ChatPlugin: LogosAPI not available, cannot emit" << eventName;
        return;
    }

    LogosAPIClient* client = logosAPI->getClient("chat");
    if (!client) {
        qWarning() << "ChatPlugin: Failed to get chat client for event" << eventName;
        return;
    }

    client->onEventResponse(this, eventName, data);
}
