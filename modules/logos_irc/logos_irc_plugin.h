#pragma once

#include <QtCore/QObject>
#include <QtCore/QJsonArray>
#include <QtCore/QStringList>
#include "logos_irc_interface.h"
#include "../../logos-cpp-sdk/cpp/logos_api.h"
#include "../../logos-cpp-sdk/cpp/logos_api_client.h"
#include "ircserver.h"
#include "logos_sdk.h"

class LogosIRCPlugin : public QObject, public LogosIRCInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID LogosIRCInterface_iid FILE "metadata.json")
    Q_INTERFACES(LogosIRCInterface PluginInterface)

public:
    LogosIRCPlugin();
    ~LogosIRCPlugin();

    Q_INVOKABLE bool foo(const QString &bar) override;
    QString name() const override { return "logos_irc"; }
    QString version() const override { return "1.0.0"; }

    // LogosAPI initialization
    Q_INVOKABLE void initLogos(LogosAPI* logosAPIInstance);

private slots:
    void onIRCChannelJoined(const QString& channel);
    void onIRCMessageSent(const QString& channel, const QString& nick, const QString& message);

private:
    void initChatBridge();
    void onChatMessage(const QVariantList& data);
    void onHistoryMessage(const QVariantList& data);
    
    LogosAPI* logosAPI = nullptr;
    LogosModules* logos = nullptr;
    IRCServer* ircServer = nullptr;
    QStringList joinedChannels;

signals:
    // for now this is required for events, later it might not be necessary if using a proxy
    void eventResponse(const QString& eventName, const QVariantList& data);
}; 
