#include "ChatWidget.h"
#include <QDebug>
#include <QDateTime>
#include <QMessageBox>
#include <iostream>
#include <csignal>
#include <QTimer>
#include "../../core/plugin_registry.h"

// Static pointer to the active ChatWidget for callbacks
static ChatWidget* activeWidget = nullptr;

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
      chatPlugin(nullptr) {
    
    // Set as the active widget
    activeWidget = this;
    
    // Get the chat plugin from the registry
    chatPlugin = PluginRegistry::getPlugin<ChatInterface>("chat");
    if (!chatPlugin) {
        qDebug() << "Failed to get chat plugin from registry";
    }
    
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
    
    // Use QTimer to initialize Waku asynchronously after constructor finishes
    QTimer::singleShot(1, this, &ChatWidget::initWaku);
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
    
    // Initialize chat with message handler
    bool success = chatPlugin->initialize(handleWakuMessage);
    
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
    
    if (chatPlugin->joinChannel(currentChannel.toStdString())) {
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
    chatPlugin->sendMessage(currentChannel.toStdString(), username.toStdString(), message.toStdString());
    
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