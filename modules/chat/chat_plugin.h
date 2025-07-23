#pragma once

#include <QtCore/QObject>
#include <functional>
#include "chat_interface.h"
#include "src/chat_api.h"
#include "../../SDK/cpp/logos_api.h"
#include "../../SDK/cpp/logos_api_client.h"

class ChatPlugin : public QObject, public ChatInterface {
    Q_OBJECT
    Q_PLUGIN_METADATA(IID ChatInterface_iid FILE "metadata.json")
    Q_INTERFACES(ChatInterface PluginInterface)

public:
    ChatPlugin();
    ~ChatPlugin();

    // PluginInterface
    QString name() const override { return "chat"; }
    QString version() const override { return "1.0.0"; }

    // ChatInterface implementation
    Q_INVOKABLE bool initialize() override;
    Q_INVOKABLE bool joinChannel(const QString& channelName) override;
    Q_INVOKABLE void sendMessage(const QString& channelName, const QString& username, const QString& message) override;
    Q_INVOKABLE bool retrieveHistory(const std::string& channelName) override;
    Q_INVOKABLE bool retrieveHistory(const QString& channelName);

    // LogosAPI initialization
    Q_INVOKABLE void initLogos(LogosAPI* logosAPIInstance);

signals:
    // for now this is required for events, later it might not be necessary if using a proxy
    void eventResponse(const QString& eventName, const QVariantList& data);

private:
    std::string currentRelayTopic;
}; 