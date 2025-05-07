#include "WakuUIWidget.h"
#include <QPushButton>
#include <QVBoxLayout>
#include <QLabel>
#include <QDebug>
#include <QCoreApplication>
#include <QMessageBox>
#include "../../../core/plugin_registry.h"

WakuUIWidget::WakuUIWidget(QWidget* parent) : QWidget(parent), wakuPlugin(nullptr) {
    // Create UI elements
    initButton = new QPushButton("Init Waku", this);
    versionButton = new QPushButton("Get Waku Version", this);
    statusLabel = new QLabel("Status: Not initialized", this);
    
    // Style the status label
    statusLabel->setFrameStyle(QFrame::Panel | QFrame::Sunken);
    statusLabel->setLineWidth(1);
    statusLabel->setMidLineWidth(0);
    statusLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    statusLabel->setMinimumHeight(30);
    statusLabel->setWordWrap(true);

    // Set up layout
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addWidget(initButton);
    layout->addWidget(versionButton);
    layout->addWidget(statusLabel);
    
    // Add some spacing and margins
    layout->setSpacing(10);
    layout->setContentsMargins(20, 20, 20, 20);

    // Connect buttons to their slots
    connect(initButton, &QPushButton::clicked, this, &WakuUIWidget::onInitButtonClicked);
    connect(versionButton, &QPushButton::clicked, this, &WakuUIWidget::onVersionButtonClicked);
    
    // Connect to the waku plugin
    bool connected = connectToWakuPlugin();
    if (!connected) {
        statusLabel->setText("Status: Failed to connect to waku plugin");
        initButton->setEnabled(false);
        versionButton->setEnabled(false);
    }
}

bool WakuUIWidget::connectToWakuPlugin() {
    // Get the waku plugin from the PluginRegistry
    wakuPlugin = PluginRegistry::getPlugin<WakuInterface>("waku");
    
    if (!wakuPlugin) {
        qWarning() << "Could not find Waku Plugin";
        return false;
    }
    
    return true;
}

void WakuUIWidget::onInitButtonClicked() {
    qDebug() << "Init Waku button clicked";

    if (!connectToWakuPlugin()) {
        QMessageBox::warning(this, "Plugin Error", "Could not connect to waku plugin");
        return;
    }

    statusLabel->setText("Status: Initializing Waku...");
    
    // Call initWaku with inline callback
    wakuPlugin->initWaku("{}", [this](bool success, const QString &message) {
        if (success) {
            statusLabel->setText(QString("Status: %1").arg(message));
        } else {
            statusLabel->setText(QString("Error: %1").arg(message));
        }
    });
}

void WakuUIWidget::onVersionButtonClicked() {
    qDebug() << "Get Version button clicked";

    if (!connectToWakuPlugin()) {
        QMessageBox::warning(this, "Plugin Error", "Could not connect to waku plugin");
        return;
    }

    statusLabel->setText("Status: Getting version...");

    // Call getVersion with inline callback
    wakuPlugin->getVersion([this](const QString &version) {
        QString versionText = QString("Waku Version: %1").arg(version);
        statusLabel->setText(versionText);
    });
}
