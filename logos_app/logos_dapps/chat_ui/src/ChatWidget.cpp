#include "ChatWidget.h"
#include <QDebug>
#include <QDateTime>
#include <QMessageBox>
#include <iostream>
#include <csignal>
#include <QTimer>
#include "../../core/plugin_registry.h"
#include "logos_api.h"
#include <QRemoteObjectDynamicReplica>
#include <QRemoteObjectReplica>
#include <QRemoteObjectRegistryHost>

// Static pointer to the active ChatWidget for callbacks
static ChatWidget* activeWidget = nullptr;

// Static pointer to the testing registry host
static QRemoteObjectRegistryHost* testingRegistryHost = nullptr;

// Static callback that can be passed to the C API
void ChatWidget::handleWakuMessage(const std::string& timestamp, const std::string& nick, const std::string& message) {
    qDebug() << "RECEIVED: [" << QString::fromStdString(timestamp) << "] " 
             << QString::fromStdString(nick) << ": " 
             << QString::fromStdString(message);
    
    // Forward to the active widget if available
    if (activeWidget) {
        QMetaObject::invokeMethod(activeWidget, [=]() {
            activeWidget->displayMessage(QString::fromStdString(nick), QString::fromStdString(message));
        }, Qt::QueuedConnection);
    }
}

ChatWidget::ChatWidget(QWidget* parent) 
    : QWidget(parent), 
      isWakuInitialized(false),
      isWakuRunning(false),
      chatPlugin(nullptr),
      m_logosAPI(nullptr) {
    
    // Set as the active widget
    activeWidget = this;
    
    // Initialize LogosAPI
    m_logosAPI = new LogosAPI("local:logoscore_registry", this);
    
    // Get the chat plugin from the registry
    chatPlugin = PluginRegistry::getPlugin<ChatInterface>("chat");
    if (!chatPlugin) {
        qDebug() << "Failed to get chat plugin from registry";
    }

    // Create registry at local:testing for chat plugin
    //     testingRegistryHost = new QRemoteObjectRegistryHost(QUrl(QStringLiteral("local:testing")), this);
    //     qDebug() << "Created testing registry host at: local:testing";
    
    // // Register chat plugin as chat_replica name in the registry
    // QObject* chatPluginObject = dynamic_cast<QObject*>(chatPlugin);
    // if (chatPluginObject) {
    //     bool success = testingRegistryHost->enableRemoting(chatPluginObject, "chat_replica");
    //     if (success) {
    //         qDebug() << "Successfully registered chat plugin as 'chat_replica' in testing registry";
    //     } else {
    //         qDebug() << "Failed to register chat plugin as 'chat_replica' in testing registry";
    //     }
    // } else {
    //     qDebug() << "Failed to cast chat plugin to QObject for registry registration";
    // }

    // Generate random username with 2 digits that will persist during this class lifetime
    int randomNum = rand() % 100;
    username = QString("LogosUser_%1").arg(randomNum, 2, 10, QChar('0'));
    qDebug() << "Generated username for this session: " << username;
    
    // Main vertical layout
    mainLayout = new QVBoxLayout(this);
    
    // Create status label
    statusLabel = new QLabel("Status: Not initialized", this);
    statusLabel->setFrameStyle(QFrame::Panel | QFrame::Sunken);
    statusLabel->setLineWidth(1);
    statusLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    statusLabel->setMinimumHeight(30);
    
    // Create channel join layout
    channelLayout = new QHBoxLayout();
    channelInput = new QLineEdit(this);
    channelInput->setPlaceholderText("Enter channel name...");
    joinButton = new QPushButton("Join", this);
    
    channelLayout->addWidget(new QLabel("Channel:"));
    channelLayout->addWidget(channelInput, 4);
    channelLayout->addWidget(joinButton, 1);
    
    // Create chat display
    chatDisplay = new QTextEdit(this);
    chatDisplay->setReadOnly(true);
    chatDisplay->setMinimumHeight(300);
    
    // Create input layout
    inputLayout = new QHBoxLayout();
    messageInput = new QLineEdit(this);
    messageInput->setPlaceholderText("Type your message here...");
    sendButton = new QPushButton("Send", this);
    
    inputLayout->addWidget(messageInput, 4);
    inputLayout->addWidget(sendButton, 1);
    
    // Add all components to main layout
    mainLayout->addWidget(statusLabel);
    mainLayout->addLayout(channelLayout);
    mainLayout->addWidget(chatDisplay);
    mainLayout->addLayout(inputLayout);
    
    // Set spacing and margins
    mainLayout->setSpacing(10);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    
    // Connect signals to slots
    connect(sendButton, &QPushButton::clicked, this, &ChatWidget::onSendButtonClicked);
    connect(joinButton, &QPushButton::clicked, this, &ChatWidget::onJoinChannelClicked);
    connect(messageInput, &QLineEdit::returnPressed, this, &ChatWidget::onSendButtonClicked);
    connect(channelInput, &QLineEdit::returnPressed, this, &ChatWidget::onJoinChannelClicked);
    
    // Disable UI components until Waku is initialized
    channelInput->setEnabled(false);
    joinButton->setEnabled(false);
    messageInput->setEnabled(false);
    sendButton->setEnabled(false);
    
    // Auto-initialize Waku
    initWaku();
}

ChatWidget::~ChatWidget() {
    // Reset the active widget if it's this instance
    if (activeWidget == this) {
        activeWidget = nullptr;
    }
    
    // Cleanup is now handled by the plugin
    stopWaku();
}

void ChatWidget::initWaku() {
    if (!chatPlugin) {
        updateStatus("Error: Chat plugin not loaded");
        return;
    }

    updateStatus("Status: Initializing Waku...");
    
    // Register the callback with LogosAPI
    //m_logosAPI->onEvent("chat", [this](const QVariantList& data) {
    //    qDebug() << "Received chat event:" << data;
    //});
    
    // request object from logos api
    QObject* chatObject = m_logosAPI->requestObject("chat");

    // ====================================
    // TODO: get replica here directly
    // ====================================

    // cast to ChatInterface
    // ChatInterface* chatInterface = dynamic_cast<ChatInterface*>(chatObject);
    // QObject* chatInterfaceObject = dynamic_cast<QObject*>(chatInterface);

    // TODO: is this chatObject valid??
    // check if object is valid
    // print signals etc.. to check it's really there
    // can also do poc to check what's going on

    // listen to eventResponse signal from chatPlugin
    // if (chatPlugin) {
    //     // Get the QObject pointer directly from the plugin registry
    //     // The plugin is actually a QObject (ChatPlugin) but returned as ChatInterface*

        QObject* pluginObject = dynamic_cast<QObject*>(chatPlugin);

        // check object type
        qDebug() << "chatObject type:" << chatObject->metaObject()->className();
        // qDebug() << "chatInterface type:" << chatInterface->metaObject()->className();
        qDebug() << "pluginObject type:" << pluginObject->metaObject()->className();

        // check if chatObject has eventResponse signal
        qDebug() << "chatObject has eventResponse signal:" << chatObject->metaObject()->indexOfSignal("eventResponse(QString,QVariantList)");
        qDebug() << "pluginObject has eventResponse signal:" << pluginObject->metaObject()->indexOfSignal("eventResponse(QString,QVariantList)");

        // check if chatObject has eventResponse_alternative signal
        qDebug() << "chatObject has eventResponse_alternative signal:" << chatObject->metaObject()->indexOfSignal("eventResponse_alternative(QString,QVariantList)");
        qDebug() << "pluginObject has eventResponse_alternative signal:" << pluginObject->metaObject()->indexOfSignal("eventResponse_alternative(QString,QVariantList)");

        // check if chatObject has eventResponse_another signal
        qDebug() << "chatObject has eventResponse_another signal:" << chatObject->metaObject()->indexOfSignal("eventResponse_another(QString,QVariantList)");
        qDebug() << "pluginObject has eventResponse_another signal:" << pluginObject->metaObject()->indexOfSignal("eventResponse_another(QString,QVariantList)");

        // check if chatObject has eventResponse slot
        qDebug() << "chatObject has eventResponse slot:" << chatObject->metaObject()->indexOfSlot("onEventResponse(QString,QVariantList)");
        qDebug() << "pluginObject has eventResponse slot:" << pluginObject->metaObject()->indexOfSlot("onEventResponse(QString,QVariantList)");

        // exit(1);

        // if (pluginObject) {
            // QObject::connect(pluginObject, SIGNAL(eventResponse(QString, QVariantList)), 
            //QObject::connect(chatObject, SIGNAL(eventResponse(QString,QVariantList)), 
            //                this, SLOT(onEventResponse(QString,QVariantList)), Qt::AutoConnection);
                        //    this, SLOT(onEventResponse(QString,QVariantList)), Qt::QueuedConnection);
        // }
    // }

    // QObject::connect(chatObject, SIGNAL(eventResponse(QString, QVariantList)), 
    //                this, SLOT(onEventResponse(QString,QVariantList)), Qt::AutoConnection);

    m_logosAPI->onEvent(chatObject, this, "chatMessage", [this](const QString& eventName, const QVariantList& data) {
        qDebug() << "RECEIVED via onEvent callback: [" << eventName << "] " << data;
        // use handleWakuMessage to handle the message
        handleWakuMessage(data[0].toString().toStdString(), data[1].toString().toStdString(), data[2].toString().toStdString());
    });

    m_logosAPI->onEvent(chatObject, this, "chatMessage", [this](const QString& eventName, const QVariantList& data) {
        qDebug() << "RECEIVED2 via onEvent callback: [" << eventName << "] " << data;
        // use handleWakuMessage to handle the message
        handleWakuMessage(data[0].toString().toStdString(), data[1].toString().toStdString(), data[2].toString().toStdString() + " (via onEvent callback)");
    });

    //m_logosAPI->onEvent(chatObject, this, "chatMessage", [this](const QString& eventName, const QVariantList& data) {
    //    qDebug() << "RECEIVED via onEvent callback: [" << eventName << "] " << data;
    //    if (data.size() >= 3) {
    //        QString timestamp = data[0].toString();
    //        QString nick = data[1].toString();
    //        QString message = data[2].toString();
    //        qDebug() << "RECEIVED via callback: [" << timestamp << "] " << nick << ": " << message;
    //        // Display the message in the chat widget
    //        displayMessage(nick, message);
    //    }
    //});


    // connect to local:testing and acquire chat_replica
    // QRemoteObjectNode remoteNode;
    // remoteNode.connectToNode(QUrl(QStringLiteral("local:testing")));

    // // Acquire the replica
    // QRemoteObjectReplica* replica = remoteNode.acquireDynamic("chat_replica");
    // if (!replica) {
    //     qDebug() << "Failed to acquire chat_replica";
    //     return;
    // }

    // Wait 5 seconds before connecting to the replica
    //QTimer::singleShot(5000, [replica, this]() {
    //    QObject::connect(replica, SIGNAL(eventResponse(QString, QVariantList)), 
    //                    this, SLOT(onEventResponse(QString, QVariantList)), Qt::AutoConnection);
    //});

    // m_logosAPI->onEvent(chatObject, this, "chatMessage", [this](const QString& eventName, const QVariantList& data) {
    //   qDebug() << "RECEIVED via onEvent: [" << eventName << "] " << data;
    //   exit(1);
    //});

    // Initialize chat with message handler after 10 seconds
    // QTimer::singleShot(10000, [this]() {
        bool success = chatPlugin->initialize();
        
        if (success) {
            isWakuInitialized = true;
            isWakuRunning = true;
            updateStatus("Status: Waku initialized and running");
            
            // Enable UI components
            channelInput->setEnabled(true);
            joinButton->setEnabled(true);
            messageInput->setEnabled(true);
            sendButton->setEnabled(true);
            
            // Set default channel name
            currentChannel = "huilong"; // Default channel
            channelInput->setText(currentChannel);
            
            // Join the default channel
            onJoinChannelClicked();
        } else {
            updateStatus("Error: Failed to initialize Waku");
        }
    // });
}

void ChatWidget::stopWaku() {
    // Nothing to do here as the plugin handles the cleanup internally
    updateStatus("Status: Stopping Waku...");
    isWakuInitialized = false;
    isWakuRunning = false;
    updateStatus("Status: Waku stopped");
    
    // Disable UI components
    channelInput->setEnabled(false);
    joinButton->setEnabled(false);
    messageInput->setEnabled(false);
    sendButton->setEnabled(false);
}

void ChatWidget::onJoinChannelClicked() {
    if (!chatPlugin) {
        updateStatus("Error: Chat plugin not loaded");
        return;
    }

    QString channelName = channelInput->text().trimmed();
    if (channelName.isEmpty()) {
        QMessageBox::warning(this, "Channel Error", "Please enter a channel name");
        return;
    }
    
    // Clear the chat display when joining a new channel
    chatDisplay->clear();
    
    // Update the channel name
    currentChannel = channelName;
    
    // Check if Waku is running
    if (!isWakuRunning) {
        QMessageBox::warning(this, "Waku Error", "Waku is not running. Please initialize Waku first.");
        return;
    }
    
    QVariant result = m_logosAPI->callRemoteMethod("chat", "joinChannel", currentChannel);
    bool success = result.toBool();
    if (success) {
        updateStatus("Joined channel: " + currentChannel);
        QString joinMessage = "You have joined channel: " + currentChannel;
        chatDisplay->append("<i>" + joinMessage + "</i>");

        // Automatically retrieve message history for the joined channel
        updateStatus("Retrieving message history for the channel...");
        chatDisplay->append("<i>--- Message History ---</i>");
        
        // Call retrieveHistory for the joined channel
        chatPlugin->retrieveHistory(currentChannel.toStdString(), [](const std::string& timestamp, const std::string& nick, const std::string& message) {
            qDebug() << "HISTORY: [" << QString::fromStdString(timestamp) << "] "
                    << QString::fromStdString(nick) << ": "
                    << QString::fromStdString(message);
            
            // Forward to the active widget if available
            if (activeWidget) {
                QMetaObject::invokeMethod(activeWidget, [=]() {
                    QString historyPrefix = "[HISTORY] ";
                    activeWidget->displayMessage(historyPrefix + QString::fromStdString(nick), QString::fromStdString(message));
                }, Qt::QueuedConnection);
            }
        });
    } else {
        updateStatus("Failed to join channel: " + currentChannel);
        QMessageBox::warning(this, "Channel Error", "Failed to join channel: " + currentChannel);
    }
    
    // Clear input field
    channelInput->clear();
    channelInput->setText(currentChannel);
}

void ChatWidget::onSendButtonClicked() {
    if (!chatPlugin) {
        updateStatus("Error: Chat plugin not loaded");
        return;
    }

    QString message = messageInput->text().trimmed();
    if (message.isEmpty()) return;

    // Check if Waku is running
    if (!isWakuRunning) {
        QMessageBox::warning(this, "Waku Error", "Waku is not running. Please initialize Waku first.");
        return;
    }

    // Send the message
    // Use LogosAPI to call sendMessage on the chat plugin
    if (m_logosAPI && m_logosAPI->isConnected()) {
        QVariant result = m_logosAPI->callRemoteMethod("chat", "sendMessage", currentChannel, username, message);
        qDebug() << "LogosAPI sendMessage result:" << result;
    } else {
        qDebug() << "LogosAPI not connected";
    }

    // Clear input field
    messageInput->clear();
}

void ChatWidget::updateStatus(const QString& message) {
    statusLabel->setText(message);
    qDebug() << message;
}

void ChatWidget::displayMessage(const QString& sender, const QString& message) {
    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    QString formattedMessage = QString("[%1] %2: %3").arg(timestamp, sender, message);
    chatDisplay->append(formattedMessage);
}

void ChatWidget::onEventResponse(const QString& eventName, const QVariantList& data) {
    qDebug() << "RECEIVED via eventResponse: [" << eventName << "] " << data;
    if (data.size() >= 3) {
        QString timestamp = data[0].toString();
        QString nick = data[1].toString();
        QString message = data[2].toString() + " (via eventResponse)";
        qDebug() << "RECEIVED via eventResponse: [" << timestamp << "] " << nick << ": " << message;
        // Display the message in the chat widget
        displayMessage(nick, message);
    }
} 
