#pragma once

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QTextEdit>
#include <QLineEdit>
#include <QLabel>
#include <string>
#include "../../modules/chat/chat_interface.h"
#include "logos_api.h"

class ChatWidget : public QWidget {
    Q_OBJECT

public:
    explicit ChatWidget(QWidget* parent = nullptr);
    ~ChatWidget();
    
    // Waku operations
    void initWaku();
    void stopWaku();
    void retrieveMessageHistory();

private slots:
    void onSendButtonClicked();
    void onJoinChannelClicked();
    void onEventResponse(const QString& eventName, const QVariantList& data);

private:
    // UI elements
    QVBoxLayout* mainLayout;
    QHBoxLayout* inputLayout;
    QHBoxLayout* channelLayout;
    
    QTextEdit* chatDisplay;
    QLineEdit* messageInput;
    QLineEdit* channelInput;
    QPushButton* sendButton;
    QPushButton* joinButton;
    QLabel* statusLabel;
    
    // Chat plugin
    ChatInterface* chatPlugin;
    
    // LogosAPI instance for remote method calls
    LogosAPI* m_logosAPI;
    
    // Connection status
    bool isWakuInitialized;
    bool isWakuRunning;
    QString currentPubSubTopic;
    QString currentChannel;
    QString username; // Persistent username for this chat session
    
    // Helper methods
    void updateStatus(const QString& message);
    void displayMessage(const QString& sender, const QString& message);
    
    // Static callback handlers
    static void handleWakuMessage(const std::string& timestamp, const std::string& nick, const std::string& message);
}; 
