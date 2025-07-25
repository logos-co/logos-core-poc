#ifndef IRCSERVER_H
#define IRCSERVER_H

#include <QObject>
#include <QTcpServer>
#include <QMap>
#include <QString>
#include <QSet>
#include "ircclient.h"

class IRCServer : public QObject
{
    Q_OBJECT

signals:
    void channelJoined(const QString& channel);
    void messageSent(const QString& channel, const QString& nick, const QString& message);

public:
    explicit IRCServer(QObject* parent = nullptr);
    ~IRCServer();

    bool start(const QString& host = "0.0.0.0", quint16 port = 6667);
    void stop();
    
    // Bridge methods for external message injection
    void injectBridgeMessage(const QString& channel, const QString& nick, const QString& message);

private slots:
    void onNewConnection();
    void onClientMessage(const QString& message);
    void onClientDisconnected();

private:
    void handleClientMessage(IRCClient* client, const QString& message);
    void sendWelcome(IRCClient* client);
    void broadcastToChannel(const QString& channel, IRCClient* sender, const QString& message);
    void removeClientFromChannels(IRCClient* client);
    void createWakuBridge();
    void wakuBridgeResponse(const QString& channel, IRCClient* sender, const QString& message);
    
    // Command handlers
    void handleNick(IRCClient* client, const QStringList& args);
    void handleUser(IRCClient* client, const QStringList& args);
    void handlePing(IRCClient* client, const QStringList& args);
    void handleJoin(IRCClient* client, const QStringList& args);
    void handlePart(IRCClient* client, const QStringList& args);
    void handlePrivmsg(IRCClient* client, const QStringList& args);
    void handleWho(IRCClient* client, const QStringList& args);
    void handleMode(IRCClient* client, const QStringList& args);
    void handleMotd(IRCClient* client, const QStringList& args);
    void handleQuit(IRCClient* client, const QStringList& args);

    QTcpServer* m_server;
    QMap<QTcpSocket*, IRCClient*> m_clients;
    QMap<QString, QSet<IRCClient*>> m_channels;
    QString m_serverName;
    IRCClient* m_wakuBridge;  // Built-in bot user
};

#endif // IRCSERVER_H 