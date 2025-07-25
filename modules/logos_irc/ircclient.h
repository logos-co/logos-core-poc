#ifndef IRCCLIENT_H
#define IRCCLIENT_H

#include <QObject>
#include <QTcpSocket>
#include <QString>
#include <QSet>
#include <QHostAddress>

class IRCClient : public QObject
{
    Q_OBJECT

public:
    explicit IRCClient(QTcpSocket* socket, QObject* parent = nullptr);
    ~IRCClient();

    // Getters
    QString nick() const { return m_nick; }
    QString user() const { return m_user; }
    QString hostAddress() const { return m_socket ? m_socket->peerAddress().toString() : "bot.localhost"; }
    bool isRegistered() const { return m_registered; }
    QSet<QString> channels() const { return m_channels; }
    QTcpSocket* socket() const { return m_socket; }

    // Setters
    void setNick(const QString& nick) { m_nick = nick; }
    void setUser(const QString& user) { m_user = user; }
    void setRegistered(bool registered) { m_registered = registered; }

    // Channel management
    void joinChannel(const QString& channel);
    void leaveChannel(const QString& channel);
    bool isInChannel(const QString& channel) const;

    // Send message to client
    void sendMessage(const QString& message);
    void sendMessage(const QString& prefix, const QString& command, const QString& params = QString());

signals:
    void messageReceived(const QString& message);
    void disconnected();

private slots:
    void onReadyRead();
    void onDisconnected();

private:
    QTcpSocket* m_socket;
    QString m_nick;
    QString m_user;
    bool m_registered;
    QSet<QString> m_channels;
    QString m_buffer;
};

#endif // IRCCLIENT_H 