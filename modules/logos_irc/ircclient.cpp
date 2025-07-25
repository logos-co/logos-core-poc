#include "ircclient.h"
#include <QDebug>

IRCClient::IRCClient(QTcpSocket* socket, QObject* parent)
    : QObject(parent)
    , m_socket(socket)
    , m_registered(false)
{
    if (m_socket) {
        m_socket->setParent(this);
        
        connect(m_socket, &QTcpSocket::readyRead, this, &IRCClient::onReadyRead);
        connect(m_socket, &QTcpSocket::disconnected, this, &IRCClient::onDisconnected);
    }
}

IRCClient::~IRCClient()
{
    if (m_socket && m_socket->state() == QTcpSocket::ConnectedState) {
        m_socket->disconnectFromHost();
    }
}

void IRCClient::joinChannel(const QString& channel)
{
    m_channels.insert(channel);
}

void IRCClient::leaveChannel(const QString& channel)
{
    m_channels.remove(channel);
}

bool IRCClient::isInChannel(const QString& channel) const
{
    return m_channels.contains(channel);
}

void IRCClient::sendMessage(const QString& message)
{
    if (m_socket && m_socket->state() == QTcpSocket::ConnectedState) {
        QString msg = message + "\r\n";
        m_socket->write(msg.toUtf8());
        m_socket->flush();
    }
    // For bot clients (no socket), we don't need to send anything
}

void IRCClient::sendMessage(const QString& prefix, const QString& command, const QString& params)
{
    QString message;
    if (!prefix.isEmpty()) {
        message = ":" + prefix + " ";
    }
    message += command;
    if (!params.isEmpty()) {
        message += " " + params;
    }
    sendMessage(message);
}

void IRCClient::onReadyRead()
{
    QByteArray data = m_socket->readAll();
    m_buffer.append(QString::fromUtf8(data));
    
    // Process complete lines
    while (m_buffer.contains("\r\n") || m_buffer.contains("\n")) {
        int pos = m_buffer.indexOf("\r\n");
        if (pos == -1) {
            pos = m_buffer.indexOf("\n");
        }
        
        QString line = m_buffer.left(pos).trimmed();
        
        if (m_buffer.indexOf("\r\n") != -1) {
            m_buffer.remove(0, pos + 2);
        } else {
            m_buffer.remove(0, pos + 1);
        }
        
        if (!line.isEmpty()) {
            emit messageReceived(line);
        }
    }
}

void IRCClient::onDisconnected()
{
    emit disconnected();
} 