#pragma once

#include <QtCore/QObject>
#include <functional>
#include "chat_interface.h"
#include "src/chat_api.h"
#include "../../modules/waku/waku_interface.h"
#include "../../SDK/cpp/logos_api.h"

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
    Q_INVOKABLE bool initialize(MessageCallback messageCallback = nullptr) override;
    Q_INVOKABLE bool joinChannel(const QString& channelName) override;
    Q_INVOKABLE void sendMessage(const QString& channelName, const QString& username, const QString& message) override;
    Q_INVOKABLE void retrieveHistory(const std::string& channelName, MessageCallback callback = nullptr) override;

private:
    void* wakuCtx;
    std::string currentRelayTopic;
    WakuInterface* wakuPlugin;
}; 