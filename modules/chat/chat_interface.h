#pragma once

#include <QtCore/QObject>
#include "../../core/interface.h"
#include <functional>

// Define a callback type for message handling
using MessageCallback = std::function<void(const std::string&, const std::string&, const std::string&)>;

class ChatInterface : public PluginInterface {
public:
    virtual ~ChatInterface() {}

    // Core chat functionality
    Q_INVOKABLE virtual bool initialize(MessageCallback messageCallback = nullptr) = 0;
    Q_INVOKABLE virtual bool joinChannel(const QString& channelName) = 0;
    Q_INVOKABLE virtual void sendMessage(const QString& channelName, const QString& username, const QString& message) = 0;
    Q_INVOKABLE virtual void retrieveHistory(const std::string& channelName, MessageCallback callback = nullptr) = 0;

signals:
    void eventResponse(const QString& timestamp, const QString& nick, const QString& message);
};

#define ChatInterface_iid "org.logos.ChatInterface"
Q_DECLARE_INTERFACE(ChatInterface, ChatInterface_iid) 