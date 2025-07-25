#include "ircserver.h"
#include <QDebug>
#include <QTcpSocket>
#include <QDateTime>

IRCServer::IRCServer(QObject* parent)
    : QObject(parent)
    , m_server(new QTcpServer(this))
    , m_serverName("logos-irc-server")
    , m_wakuBridge(nullptr)
{
    connect(m_server, &QTcpServer::newConnection, this, &IRCServer::onNewConnection);
}

IRCServer::~IRCServer()
{
    stop();
}

bool IRCServer::start(const QString& host, quint16 port)
{
    QHostAddress address;
    if (host == "0.0.0.0") {
        address = QHostAddress::Any;
    } else {
        address = QHostAddress(host);
    }

    if (!m_server->listen(address, port)) {
        qDebug() << "Failed to start IRC server:" << m_server->errorString();
        return false;
    }

    // Create the waku_bridge bot
    createWakuBridge();

    qDebug() << "IRC server started on" << host << ":" << port;
    return true;
}

void IRCServer::stop()
{
    if (m_server->isListening()) {
        m_server->close();
    }

    // Disconnect all clients
    for (auto it = m_clients.begin(); it != m_clients.end(); ++it) {
        IRCClient* client = it.value();
        client->socket()->disconnectFromHost();
        client->deleteLater();
    }
    m_clients.clear();
    m_channels.clear();

    // Clean up waku bridge
    if (m_wakuBridge) {
        m_wakuBridge->deleteLater();
        m_wakuBridge = nullptr;
    }

    qDebug() << "IRC server stopped";
}

void IRCServer::onNewConnection()
{
    while (m_server->hasPendingConnections()) {
        QTcpSocket* socket = m_server->nextPendingConnection();
        IRCClient* client = new IRCClient(socket, this);
        
        m_clients[socket] = client;
        
        connect(client, &IRCClient::messageReceived, this, &IRCServer::onClientMessage);
        connect(client, &IRCClient::disconnected, this, &IRCServer::onClientDisconnected);
        
        qDebug() << "New client connected from" << socket->peerAddress().toString();
    }
}

void IRCServer::onClientMessage(const QString& message)
{
    IRCClient* client = qobject_cast<IRCClient*>(sender());
    if (!client) return;
    
    handleClientMessage(client, message);
}

void IRCServer::onClientDisconnected()
{
    IRCClient* client = qobject_cast<IRCClient*>(sender());
    if (!client) return;
    
    QTcpSocket* socket = client->socket();
    
    qDebug() << "Client disconnected:" << client->nick() << "from" << client->hostAddress();
    
    // Remove client from all channels
    removeClientFromChannels(client);
    
    // Remove from clients map
    m_clients.remove(socket);
    
    // Delete the client object
    client->deleteLater();
}

void IRCServer::handleClientMessage(IRCClient* client, const QString& message)
{
    qDebug() << "Received from" << client->hostAddress() << ":" << message;
    
    QStringList parts = message.split(' ', Qt::SkipEmptyParts);
    if (parts.isEmpty()) return;
    
    QString command = parts[0].toUpper();
    QStringList args = parts.mid(1);
    
    if (command == "NICK") {
        handleNick(client, args);
    } else if (command == "USER") {
        handleUser(client, args);
    } else if (command == "PING") {
        handlePing(client, args);
    } else if (command == "JOIN") {
        handleJoin(client, args);
    } else if (command == "PART") {
        handlePart(client, args);
    } else if (command == "PRIVMSG") {
        handlePrivmsg(client, args);
    } else if (command == "WHO") {
        handleWho(client, args);
    } else if (command == "MODE") {
        handleMode(client, args);
    } else if (command == "MOTD") {
        handleMotd(client, args);
    } else if (command == "QUIT") {
        handleQuit(client, args);
    }
    
    // Check if client should be registered
    if (!client->isRegistered() && !client->nick().isEmpty() && !client->user().isEmpty()) {
        client->setRegistered(true);
        sendWelcome(client);
    }
}

void IRCServer::sendWelcome(IRCClient* client)
{
    client->sendMessage(m_serverName, "001", client->nick() + " :Welcome to Logos IRC Server");
    client->sendMessage(m_serverName, "002", client->nick() + " :Your host is " + m_serverName);
    client->sendMessage(m_serverName, "003", client->nick() + " :This server was created " + QDateTime::currentDateTime().toString());
    client->sendMessage(m_serverName, "004", client->nick() + " " + m_serverName + " v1.0 o o");

    client->sendMessage(m_serverName, "372", client->nick() + "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@");
    client->sendMessage(m_serverName, "372", client->nick() + "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@");
    client->sendMessage(m_serverName, "372", client->nick() + "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@#=........._.....=%@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@");
    client->sendMessage(m_serverName, "372", client->nick() + "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@#=........................=#@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@");
    client->sendMessage(m_serverName, "372", client->nick() + "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*..........:=#@@@@#=:......_...*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@");
    client->sendMessage(m_serverName, "372", client->nick() + "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@#:..........-%@@@@@@@@@@@@#-..........-#@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@");
    client->sendMessage(m_serverName, "372", client->nick() + "@@@@@@@@@@@@@@@@@@@@@@@@@@@#......._....-@@@@@@@@@@@@@@@@@%:.........._.#@@@@@@@@@@@@@@@@@@@@@@@@@@");
    client->sendMessage(m_serverName, "372", client->nick() + "@@@@@@@@@@@@@@@@@@@@@@@@@#..........._+@@@@@@@@@@@@@@@@@@@@+............:#@@@@@@@@@@@@@@@@@@@@@@@@@");
    client->sendMessage(m_serverName, "372", client->nick() + "@@@@@@@@@@@@@@@@@@@@@@@%:............_=@@@@@@@@@@@@@@@@@@@@@@=._..._........@@@@@@@@@@@@@@@@@@@@@@@");
    client->sendMessage(m_serverName, "372", client->nick() + "@@@@@@@@@@@@@@@@@@@@@@#..............@@@@@@@@@@@@@@@@@@@@@@@%........._.._.#@@@@@@@@@@@@@@@@@@@@@@@");
    client->sendMessage(m_serverName, "372", client->nick() + "@@@@@@@@@@@@@@@@@@@@@+.........._...-@@@@@@@@@@@@@@@@@@@@@@@@:............._+@@@@@@@@@@@@@@@@@@@@@@");
    client->sendMessage(m_serverName, "372", client->nick() + "@@@@@@@@@@@@@@@@@@@@@-._.............-@@@@@@@@@@@@@@@@@@@@@@@@-..............@@@@@@@@@@@@@@@@@@@@@@");
    client->sendMessage(m_serverName, "372", client->nick() + "@@@@@@@@@@@@@@@@@@@@@@:..............@@@@@@@@@@@@@@@@@@@@@@@@._._._.........-@@@@@@@@@@@@@@@@@@@@@@");
    client->sendMessage(m_serverName, "372", client->nick() + "@@@@@@@@@@@@@@@@@@@@@@@*.............*@@@@@@@@@@@@@@@@@@@@@@+.............*@@@@@@@@@@@@@@@@@@@@@@@@");
    client->sendMessage(m_serverName, "372", client->nick() + "@@@@@@@@@@@@@@@@@@@@@@@@%-.........._.#@@@@@@@@@@@@@@@@@@@@#.............-@@@@@@@@@@@@@@@@@@@@@@@@@");
    client->sendMessage(m_serverName, "372", client->nick() + "@@@@@@@@@@@@@@@@@@@@@@@@@@#.._.........*@@@@@@@@@@@@@@@@@@+....._....._#@@@@@@@@@@@@@@@@@@@@@@@@@@@");
    client->sendMessage(m_serverName, "372", client->nick() + "@@@@@@@@@@@@@@@@@@@@@@@@@@@@#:........._:*@@@@@@@@@@@@@@*:..........:#@@@@@@@@@@@@@@@@@@@@@@@@@@@@@");
    client->sendMessage(m_serverName, "372", client->nick() + "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@+...........:#%@@@@@@%#:........._+@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@");
    client->sendMessage(m_serverName, "372", client->nick() + "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*:............_........._...:*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@");
    client->sendMessage(m_serverName, "372", client->nick() + "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@+-:................:-+@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@");
    client->sendMessage(m_serverName, "372", client->nick() + "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@%%#*+====+*#%%@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@");
    client->sendMessage(m_serverName, "372", client->nick() + "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@");
    client->sendMessage(m_serverName, "372", client->nick() + "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@");
    client->sendMessage(m_serverName, "372", client->nick() + "@@@@@@@@@@@@@@@@@-....._.........._................................_..............@@@@@@@@@@@@@@@@@");
    client->sendMessage(m_serverName, "372", client->nick() + "@@@@@@@@@@@@@@@@@-..........................:‒=++=-:._............................@@@@@@@@@@@@@@@@@");
    client->sendMessage(m_serverName, "372", client->nick() + "@@@@@@@@@@@@@@@@@-...................=*%@@@@@@@@@@@@@@@@@@%*=.....................@@@@@@@@@@@@@@@@@");
    client->sendMessage(m_serverName, "372", client->nick() + "@@@@@@@@@@@@@@@@@-..............._%@@@@@@@@@@@@@@@@@@@@@@@@@@@@%-..............-..@@@@@@@@@@@@@@@@@");
    client->sendMessage(m_serverName, "372", client->nick() + "@@@@@@@@@@@@@@@@@-............=%@@@@@@@@@@@@#=-:...:-*@@@@@@@@@@@@%=..............@@@@@@@@@@@@@@@@@");
    client->sendMessage(m_serverName, "372", client->nick() + "@@@@@@@@@@@@@@@@@-..._......:#@@@@@@@@@@@@%:............+@@@@@@@@@@@@#:...........@@@@@@@@@@@@@@@@@");
    client->sendMessage(m_serverName, "372", client->nick() + "@@@@@@@@@@@@@@@@@-........:%@@@@@@@@@@@@%:.........._.....+@@@@@@@@@@@@%-.........@@@@@@@@@@@@@@@@@");
    client->sendMessage(m_serverName, "372", client->nick() + "@@@@@@@@@@@@@@@@@-......_.#@@@@@@@@@@@@@*..._................-@@@@@@@@@@@@@#......@@@@@@@@@@@@@@@@@");
    client->sendMessage(m_serverName, "372", client->nick() + "@@@@@@@@@@@@@@@@@-.....=@@@@@@@@@@@@@@#..........._.........._+@@@@@@@@@@@@@@=....@@@@@@@@@@@@@@@@@");
    client->sendMessage(m_serverName, "372", client->nick() + "@@@@@@@@@@@@@@@@@-...*@@@@@@@@@@@@@@@=......._.................@@@@@@@@@@@@@@@#...@@@@@@@@@@@@@@@@@");
    client->sendMessage(m_serverName, "372", client->nick() + "@@@@@@@@@@@@@@@@@-._.#@@@@@@@@@@@@@@@@:............_........._#@@@@@@@@@@@@@@@#...@@@@@@@@@@@@@@@@@");
    client->sendMessage(m_serverName, "372", client->nick() + "@@@@@@@@@@@@@@@@@-._.*@@@@@@@@@@@@@@@@:........................#@@@@@@@@@@@@@@@*..@@@@@@@@@@@@@@@@@");
    client->sendMessage(m_serverName, "372", client->nick() + "@@@@@@@@@@@@@@@@@-....=@@@@@@@@@@@@@@@+._._......._._..._....:@@@@@@@@@@@@@@@=....@@@@@@@@@@@@@@@@@");
    client->sendMessage(m_serverName, "372", client->nick() + "@@@@@@@@@@@@@@@@@-.....:%@@@@@@@@@@@@@%:....._..............*@@@@@@@@@@@@@%:.._...@@@@@@@@@@@@@@@@@");
    client->sendMessage(m_serverName, "372", client->nick() + "@@@@@@@@@@@@@@@@@-...._..=@@@@@@@@@@@@@%._.................+@@@@@@@@@@@@@=........@@@@@@@@@@@@@@@@@");
    client->sendMessage(m_serverName, "372", client->nick() + "@@@@@@@@@@@@@@@@@-........_+@@@@@@@@@@@@@=..._...........:#@@@@@@@@@@@@+..........@@@@@@@@@@@@@@@@@");
    client->sendMessage(m_serverName, "372", client->nick() + "@@@@@@@@@@@@@@@@@-........_.=@@@@@@@@@@@@@=......_...._:%@@@@@@@@@@@@=............@@@@@@@@@@@@@@@@@");
    client->sendMessage(m_serverName, "372", client->nick() + "@@@@@@@@@@@@@@@@@-............_+@@@@@@@@@@@@@%*=---+#%@@@@@@@@@@@@+:..............@@@@@@@@@@@@@@@@@");
    client->sendMessage(m_serverName, "372", client->nick() + "@@@@@@@@@@@@@@@@@-._...._........_.=#@@@@@@@@@@@@@@@@@@@@@@@@@@#=._._.._..........@@@@@@@@@@@@@@@@@");
    client->sendMessage(m_serverName, "372", client->nick() + "@@@@@@@@@@@@@@@@@-.....................=#@@@@@@@@@@@@@@@@#=:..........._..........@@@@@@@@@@@@@@@@@");
    client->sendMessage(m_serverName, "372", client->nick() + "@@@@@@@@@@@@@@@@@-........._...._..............::........_......._.............-..@@@@@@@@@@@@@@@@@");
    client->sendMessage(m_serverName, "372", client->nick() + "@@@@@@@@@@@@@@@@@-................................_.................._.....-......@@@@@@@@@@@@@@@@@");
    client->sendMessage(m_serverName, "372", client->nick() + "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@");
    client->sendMessage(m_serverName, "372", client->nick() + "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@");

    // Send MOTD
    client->sendMessage(m_serverName, "375", client->nick() + " :- " + m_serverName + " Message of the day -");
    client->sendMessage(m_serverName, "372", client->nick() + " :- Welcome to the Logos IRC Server");
    client->sendMessage(m_serverName, "372", client->nick() + " :- This is a simple IRC server implementation");
    client->sendMessage(m_serverName, "376", client->nick() + " :End of /MOTD command");
}

void IRCServer::broadcastToChannel(const QString& channel, IRCClient* sender, const QString& message)
{
    if (!m_channels.contains(channel)) return;
    
    QSet<IRCClient*> clients = m_channels[channel];
    for (IRCClient* client : clients) {
        if (client != sender) {
            QString prefix = sender->nick() + "!" + sender->user() + "@" + sender->hostAddress();
            client->sendMessage(prefix, "PRIVMSG", channel + " :" + message);
        }
    }
}

void IRCServer::removeClientFromChannels(IRCClient* client)
{
    for (auto it = m_channels.begin(); it != m_channels.end();) {
        it.value().remove(client);
        // Don't remove channels that still have the bot or other users
        if (it.value().isEmpty()) {
            it = m_channels.erase(it);
        } else {
            ++it;
        }
    }
}

void IRCServer::handleNick(IRCClient* client, const QStringList& args)
{
    if (args.isEmpty()) return;
    
    QString oldNick = client->nick();
    QString newNick = args[0];
    
    // Check if nick is already in use (simple check)
    for (auto it = m_clients.begin(); it != m_clients.end(); ++it) {
        IRCClient* otherClient = it.value();
        if (otherClient != client && otherClient->nick().toLower() == newNick.toLower()) {
            client->sendMessage(m_serverName, "433", client->nick().isEmpty() ? "*" : client->nick() + " " + newNick + " :Nickname is already in use");
            return;
        }
    }
    
    // If client is registered, send nick change notification to all channels
    if (client->isRegistered() && !oldNick.isEmpty()) {
        QString prefix = oldNick + "!" + client->user() + "@" + client->hostAddress();
        
        // Send nick change notification to the client itself first
        client->sendMessage(prefix, "NICK", ":" + newNick);
        
        // Notify all users in channels where this client is present
        QSet<IRCClient*> notifiedClients;
        notifiedClients.insert(client); // Don't notify the client twice
        
        for (const QString& channel : client->channels()) {
            if (m_channels.contains(channel)) {
                for (IRCClient* channelClient : m_channels[channel]) {
                    if (!notifiedClients.contains(channelClient)) {
                        channelClient->sendMessage(prefix, "NICK", ":" + newNick);
                        notifiedClients.insert(channelClient);
                    }
                }
            }
        }
    }
    
    client->setNick(newNick);
    qDebug() << "Client" << client->hostAddress() << "changed nick from" << oldNick << "to" << newNick;
}

void IRCServer::handleUser(IRCClient* client, const QStringList& args)
{
    if (args.size() < 4) return;
    
    QString username = args[0];
    client->setUser(username);
    qDebug() << "Client" << client->hostAddress() << "set user to" << username;
}

void IRCServer::handlePing(IRCClient* client, const QStringList& args)
{
    QString token = args.isEmpty() ? "ping" : args[0];
    client->sendMessage("PONG", ":" + token);
}

void IRCServer::handleJoin(IRCClient* client, const QStringList& args)
{
    if (args.isEmpty()) return;
    if (!client->isRegistered()) return;
    
    QString channel = args[0];
    if (!channel.startsWith("#")) {
        channel = "#" + channel;
    }
    
    // Add client to channel
    client->joinChannel(channel);
    m_channels[channel].insert(client);
    
    // Send JOIN confirmation to the client
    QString prefix = client->nick() + "!" + client->user() + "@" + client->hostAddress();
    client->sendMessage(prefix, "JOIN", ":" + channel);
    
    // Send channel topic (if any)
    client->sendMessage(m_serverName, "332", client->nick() + " " + channel + " :Welcome to " + channel);
    
    // Send names list (who's in the channel)
    QStringList names;
    for (IRCClient* channelClient : m_channels[channel]) {
        if (channelClient->isRegistered()) {
            names << channelClient->nick();
        }
    }
    
    if (!names.isEmpty()) {
        QString namesList = names.join(" ");
        client->sendMessage(m_serverName, "353", client->nick() + " = " + channel + " :" + namesList);
    }
    client->sendMessage(m_serverName, "366", client->nick() + " " + channel + " :End of /NAMES list");
    
    // Notify other users in the channel that this user joined
    for (IRCClient* channelClient : m_channels[channel]) {
        if (channelClient != client && channelClient->isRegistered()) {
            channelClient->sendMessage(prefix, "JOIN", ":" + channel);
        }
    }
    
    qDebug() << "Client" << client->nick() << "joined channel" << channel;
    
    // Emit signal to notify that a channel was joined
    emit channelJoined(channel);
}

void IRCServer::handlePrivmsg(IRCClient* client, const QStringList& args)
{
    if (args.size() < 2) return;
    
    QString target = args[0];
    QString message = args.mid(1).join(" ");
    if (message.startsWith(":")) {
        message = message.mid(1);
    }
    
    if (target.startsWith("#")) {
        // Channel message
        if (client->isInChannel(target)) {
            broadcastToChannel(target, client, message);
            qDebug() << "Broadcasting message from" << client->nick() << "to channel" << target << ":" << message;
            
            // Emit signal to notify that a message was sent (for chat bridge)
            emit messageSent(target, client->nick(), message);
            
            // Trigger waku_bridge response if it's in the channel
            wakuBridgeResponse(target, client, message);
        }
    } else {
        // Private message (not implemented in this simple version)
        qDebug() << "Private message from" << client->nick() << "to" << target << ":" << message;
    }
}

void IRCServer::handlePart(IRCClient* client, const QStringList& args)
{
    if (args.isEmpty()) return;
    if (!client->isRegistered()) return;
    
    QString channel = args[0];
    if (!channel.startsWith("#")) {
        channel = "#" + channel;
    }
    
    QString reason = args.size() > 1 ? args.mid(1).join(" ") : "Leaving";
    if (reason.startsWith(":")) {
        reason = reason.mid(1);
    }
    
    if (client->isInChannel(channel) && m_channels.contains(channel)) {
        QString prefix = client->nick() + "!" + client->user() + "@" + client->hostAddress();
        
        // Notify all users in the channel that this user left
        for (IRCClient* channelClient : m_channels[channel]) {
            if (channelClient->isRegistered()) {
                channelClient->sendMessage(prefix, "PART", channel + " :" + reason);
            }
        }
        
        // Remove client from channel
        client->leaveChannel(channel);
        m_channels[channel].remove(client);
        
        // Remove empty channels
        if (m_channels[channel].isEmpty()) {
            m_channels.remove(channel);
        }
        
        qDebug() << "Client" << client->nick() << "left channel" << channel << ":" << reason;
    }
}

void IRCServer::handleWho(IRCClient* client, const QStringList& args)
{
    if (args.isEmpty()) return;
    if (!client->isRegistered()) return;
    
    QString target = args[0];
    
    if (target.startsWith("#")) {
        // WHO for channel
        if (m_channels.contains(target)) {
            for (IRCClient* channelClient : m_channels[target]) {
                if (channelClient->isRegistered()) {
                    QString response = QString("%1 %2 %3 %4 %5 H :0 %6")
                        .arg(channelClient->nick())
                        .arg(channelClient->user())
                        .arg(channelClient->hostAddress())
                        .arg(m_serverName)
                        .arg(channelClient->nick())
                        .arg(channelClient->user());
                    client->sendMessage(m_serverName, "352", client->nick() + " " + target + " " + response);
                }
            }
        }
        client->sendMessage(m_serverName, "315", client->nick() + " " + target + " :End of /WHO list");
    }
}

void IRCServer::handleMode(IRCClient* client, const QStringList& args)
{
    if (args.isEmpty()) return;
    if (!client->isRegistered()) return;
    
    QString target = args[0];
    
    if (target == client->nick()) {
        // User mode query
        if (args.size() == 1) {
            client->sendMessage(m_serverName, "221", client->nick() + " +");
        }
    } else if (target.startsWith("#")) {
        // Channel mode query
        if (args.size() == 1) {
            client->sendMessage(m_serverName, "324", client->nick() + " " + target + " +");
            client->sendMessage(m_serverName, "329", client->nick() + " " + target + " " + QString::number(QDateTime::currentSecsSinceEpoch()));
        }
    }
}

void IRCServer::handleMotd(IRCClient* client, const QStringList& args)
{
    Q_UNUSED(args)
    if (!client->isRegistered()) return;
    
    client->sendMessage(m_serverName, "375", client->nick() + " :- " + m_serverName + " Message of the day -");

    client->sendMessage(m_serverName, "372", client->nick() + " :- Welcome to the Logos IRC Proxy Server");
    client->sendMessage(m_serverName, "376", client->nick() + " :End of /MOTD command");
}

void IRCServer::createWakuBridge()
{
    // Create a bot client without a socket
    m_wakuBridge = new IRCClient(nullptr, this);
    m_wakuBridge->setNick("waku_bridge");
    m_wakuBridge->setUser("waku");
    m_wakuBridge->setRegistered(true);
    
    // Add the bot to #general channel
    QString generalChannel = "#general";
    m_wakuBridge->joinChannel(generalChannel);
    m_channels[generalChannel].insert(m_wakuBridge);
    
    qDebug() << "Created waku_bridge bot and added to #general";
}

void IRCServer::wakuBridgeResponse(const QString& channel, IRCClient* sender, const QString& message)
{
    Q_UNUSED(message)
    
    // Only respond in #general channel and not to the bot itself
    if (channel == "#general" && sender != m_wakuBridge) {
        // Send the response from waku_bridge
        QString prefix = m_wakuBridge->nick() + "!" + m_wakuBridge->user() + "@" + m_wakuBridge->hostAddress();
        
        // Broadcast the response to all users in the channel
        if (m_channels.contains(channel)) {
            for (IRCClient* channelClient : m_channels[channel]) {
                if (channelClient != m_wakuBridge && channelClient->isRegistered()) {
                    channelClient->sendMessage(prefix, "PRIVMSG", channel + " :hello back!");
                }
            }
        }
        
        qDebug() << "waku_bridge responded to message from" << sender->nick() << "in" << channel;
    }
}

void IRCServer::handleQuit(IRCClient* client, const QStringList& args)
{
    QString reason = args.isEmpty() ? "Client quit" : args.join(" ");
    if (reason.startsWith(":")) {
        reason = reason.mid(1);
    }
    
    // Notify all users in channels where this client is present
    if (client->isRegistered()) {
        QString prefix = client->nick() + "!" + client->user() + "@" + client->hostAddress();
        QSet<IRCClient*> notifiedClients;
        
        for (const QString& channel : client->channels()) {
            if (m_channels.contains(channel)) {
                for (IRCClient* channelClient : m_channels[channel]) {
                    if (channelClient != client && !notifiedClients.contains(channelClient)) {
                        channelClient->sendMessage(prefix, "QUIT", ":" + reason);
                        notifiedClients.insert(channelClient);
                    }
                }
            }
        }
    }
    
    qDebug() << "Client" << client->nick() << "quit:" << reason;
    if (client->socket()) {
        client->socket()->disconnectFromHost();
    }
}

void IRCServer::injectBridgeMessage(const QString& channel, const QString& nick, const QString& message)
{
    if (!m_channels.contains(channel)) {
        qDebug() << "IRCServer::injectBridgeMessage: Channel" << channel << "does not exist";
        return;
    }
    
    // Create a bridge user prefix
    QString prefix = nick + "!bridge@waku.bridge";
    
    // Send the message to all users in the channel
    QSet<IRCClient*> clients = m_channels[channel];
    for (IRCClient* client : clients) {
        if (client->isRegistered()) {
            client->sendMessage(prefix, "PRIVMSG", channel + " :" + message);
        }
    }
    
    qDebug() << "IRCServer: Injected bridge message from" << nick << "to channel" << channel << ":" << message;
} 