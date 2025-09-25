#include "logos_irc_plugin.h"
#include <QDebug>
#include <QCoreApplication>
#include <QVariantList>
#include <QDateTime>
#include <QJsonArray>
#include <QJsonObject>
#include "../../logos-cpp-sdk/cpp/token_manager.h"
#include "ircserver.h"

LogosIRCPlugin::LogosIRCPlugin()
{
    qDebug() << "LogosIRCPlugin: Initializing...";
    
    // Create and start the IRC server
    ircServer = new IRCServer(this);
    
    // Connect IRC server signals
    connect(ircServer, &IRCServer::channelJoined, this, &LogosIRCPlugin::onIRCChannelJoined);
    connect(ircServer, &IRCServer::messageSent, this, &LogosIRCPlugin::onIRCMessageSent);
    
    if (ircServer->start("0.0.0.0", 6667)) {
        qDebug() << "LogosIRCPlugin: IRC Server started successfully on port 6667";
    } else {
        qWarning() << "LogosIRCPlugin: Failed to start IRC Server";
    }
    
    qDebug() << "LogosIRCPlugin: Initialized successfully";
}

LogosIRCPlugin::~LogosIRCPlugin() 
{
    // Clean up resources
    if (ircServer) {
        ircServer->stop();
        delete ircServer;
        ircServer = nullptr;
        qDebug() << "LogosIRCPlugin: IRC Server stopped and cleaned up";
    }
    if (logos) {
        delete logos;
        logos = nullptr;
    }
    if (logosAPI) {
        delete logosAPI;
        logosAPI = nullptr;
    }
}

bool LogosIRCPlugin::foo(const QString &bar)
{
    qDebug() << "LogosIRCPlugin::foo called with:" << bar;
    
    // Create event data with the bar parameter
    QVariantList eventData;
    eventData << bar; // Add the bar parameter to the event data
    eventData << QDateTime::currentDateTime().toString(Qt::ISODate); // Add timestamp
    
    // This is here for now for testing purposes
    // get token manager from logos api and print the keys
    TokenManager* tokenManager = logosAPI->getTokenManager();
    if (tokenManager) {
        qDebug() << "--------------------------------------------------------";
        qDebug() << "LogosIRCPlugin: Token manager keys:";
        // print the keys and values
        QList<QString> keys = tokenManager->getTokenKeys();
        for (const QString& key : keys) {
            qDebug() << "LogosIRCPlugin: Token key:" << key << "value:" << tokenManager->getToken(key);
        }
        qDebug() << "--------------------------------------------------------";
    } else {
        qWarning() << "LogosIRCPlugin: Token manager not available";
    }
    
    // Trigger the event using LogosAPI client (like chat module does)
    if (logosAPI) {
        // print triggering signal
        qDebug() << "LogosIRCPlugin: Triggering event 'fooTriggered' with data:" << eventData;
        logosAPI->getClient("core_manager")->onEventResponse(this, "fooTriggered", eventData);
        qDebug() << "LogosIRCPlugin: Event 'fooTriggered' triggered with data:" << eventData;
    } else {
        qWarning() << "LogosIRCPlugin: LogosAPI not available, cannot trigger event";
    }
    
    return true;
}

void LogosIRCPlugin::initLogos(LogosAPI* logosAPIInstance) {
    logosAPI = logosAPIInstance;
    if (logos) {
        delete logos;
        logos = nullptr;
    }
    logos = new LogosModules(logosAPI);

    // Initialize chat bridge after LogosAPI is available
    initChatBridge();
}

void LogosIRCPlugin::initChatBridge() {
    if (!logosAPI) {
        qWarning() << "LogosIRCPlugin: Cannot initialize chat bridge - LogosAPI not available";
        return;
    }
    
    qDebug() << "LogosIRCPlugin: Initializing chat bridge...";
    
    if (!logos->chat.on("chatMessage", [this](const QVariantList& data) {
            onChatMessage(data);
        })) {
        qWarning() << "LogosIRCPlugin: failed to subscribe to chatMessage";
    }

    if (!logos->chat.on("historyMessage", [this](const QVariantList& data) {
            onHistoryMessage(data);
        })) {
        qWarning() << "LogosIRCPlugin: failed to subscribe to historyMessage";
    }

    // Initialize the chat module
    QVariant result = logos->chat.initialize();
    bool success = result.toBool();
    
    if (success) {
        qDebug() << "LogosIRCPlugin: Chat bridge initialized successfully";
        qDebug() << "LogosIRCPlugin: Waiting for IRC users to join channels...";
    } else {
        qWarning() << "LogosIRCPlugin: Failed to initialize chat bridge";
    }
}

void LogosIRCPlugin::onChatMessage(const QVariantList& data) {
    if (data.size() >= 3) {
        QString timestamp = data[0].toString();
        QString nick = data[1].toString();
        QString message = data[2].toString();
        
        qDebug() << "LogosIRCPlugin: Received chat message from" << nick << ":" << message;
        
        // Forward this message to IRC clients as a bridge message
        if (ircServer) {
            // Forward to all joined channels for now
            // TODO: We need channel information in the message data to properly route
            for (const QString& channel : joinedChannels) {
                QString ircChannel = QString("#%1").arg(channel);
                
                // Prefix the nick to indicate it's from the bridge
                QString bridgeNick = QString("[WAKU]%1").arg(nick);
                
                ircServer->injectBridgeMessage(ircChannel, bridgeNick, message);
            }
        }
        
        Q_UNUSED(timestamp)
    }
}

void LogosIRCPlugin::onHistoryMessage(const QVariantList& data) {
    if (data.size() >= 3) {
        QString timestamp = data[0].toString();
        QString nick = data[1].toString();
        QString message = data[2].toString();
        
        qDebug() << "LogosIRCPlugin: Received history message from" << nick << ":" << message;
        
        // Forward this history message to IRC clients as a bridge message
        if (ircServer) {
            // Forward to all joined channels for now
            // TODO: We need channel information in the message data to properly route
            for (const QString& channel : joinedChannels) {
                QString ircChannel = QString("#%1").arg(channel);
                
                // Prefix the nick to indicate it's from the bridge and mark as history
                QString bridgeNick = QString("[HISTORY][WAKU]%1").arg(nick);
                
                ircServer->injectBridgeMessage(ircChannel, bridgeNick, message);
            }
        }
        
        Q_UNUSED(timestamp)
    }
}

void LogosIRCPlugin::onIRCChannelJoined(const QString& channel) {
    if (!logosAPI) {
        qWarning() << "LogosIRCPlugin: Cannot join chat channel - LogosAPI not available";
        return;
    }
    
    // Extract channel name without # prefix for chat API
    QString channelName = channel;
    if (channelName.startsWith("#")) {
        channelName = channelName.mid(1);
    }
    
    // Check if we've already joined this channel
    if (joinedChannels.contains(channelName)) {
        qDebug() << "LogosIRCPlugin: Channel" << channelName << "already joined on chat side";
        return;
    }
    
    qDebug() << "LogosIRCPlugin: IRC user joined channel" << channel << ", joining chat channel:" << channelName;
    
    // Join the channel on the chat side
    QVariant joinResult = logos->chat.joinChannel(channelName);
    if (joinResult.toBool()) {
        qDebug() << "LogosIRCPlugin: Successfully joined chat channel:" << channelName;
        
        // Add to our list of joined channels
        joinedChannels.append(channelName);
        
        // Automatically retrieve message history for the joined channel
        qDebug() << "LogosIRCPlugin: Retrieving message history for channel:" << channelName;
        QVariant historyResult = logos->chat.retrieveHistory(channelName);
        qDebug() << "LogosIRCPlugin: retrieveHistory result:" << historyResult;
    } else {
        qWarning() << "LogosIRCPlugin: Failed to join chat channel:" << channelName;
    }
}

void LogosIRCPlugin::onIRCMessageSent(const QString& channel, const QString& nick, const QString& message) {
    if (!logosAPI) {
        qWarning() << "LogosIRCPlugin: Cannot send message to chat - LogosAPI not available";
        return;
    }
    
    // Extract channel name without # prefix for chat API
    QString channelName = channel;
    if (channelName.startsWith("#")) {
        channelName = channelName.mid(1);
    }
    
    qDebug() << "LogosIRCPlugin: Forwarding IRC message from" << nick << "in channel" << channelName << ":" << message;
    
    // Send the message to the chat module
    logos->chat.sendMessage(channelName, nick, message);
} 
