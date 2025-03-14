#include "coremoduleview.h"
#include <QFont>
#include <memory>
#include <QStringList>
#include <QCoreApplication>
#include "../core/interface.h"

// Forward declare the logos_core functions we need
extern "C" {
    char** logos_core_get_loaded_plugins();
}

// Custom deleter for the plugins array
struct PluginsArrayDeleter {
    void operator()(char** plugins) {
        if (!plugins) return;
        for (char** p = plugins; *p != nullptr; ++p) {
            delete[] *p;
        }
        delete[] plugins;
    }
};

CoreModuleView::CoreModuleView(QWidget *parent)
    : QWidget(parent)
    , m_layout(nullptr)
    , m_titleLabel(nullptr)
    , m_subtitleLabel(nullptr)
    , m_pluginList(nullptr)
    , m_updateTimer(nullptr)
{
    setupUi();
    createPluginList();
    
    // Set up timer to update plugin list every 5 seconds
    m_updateTimer = new QTimer(this);
    connect(m_updateTimer, &QTimer::timeout, this, &CoreModuleView::updatePluginList);
    m_updateTimer->start(5000);
    
    // Initial update
    updatePluginList();
}

CoreModuleView::~CoreModuleView()
{
    // Qt automatically cleans up child widgets
}

void CoreModuleView::setupUi()
{
    // Set the background color for the entire view
    setStyleSheet("QWidget#coreModuleView { background-color: #1e1e1e; }");
    setObjectName("coreModuleView");

    m_layout = new QVBoxLayout(this);
    m_layout->setSpacing(20);
    m_layout->setContentsMargins(40, 40, 40, 40);

    // Create and style the title
    m_titleLabel = new QLabel("Core Modules", this);
    QFont titleFont = m_titleLabel->font();
    titleFont.setPointSize(24);
    titleFont.setBold(true);
    m_titleLabel->setFont(titleFont);
    m_titleLabel->setStyleSheet("color: #ffffff;");

    // Add a subtitle
    m_subtitleLabel = new QLabel("Currently loaded plugins in the system", this);
    m_subtitleLabel->setStyleSheet("color: #a0a0a0; font-size: 14px; margin-bottom: 20px;");

    m_layout->addWidget(m_titleLabel);
    m_layout->addWidget(m_subtitleLabel);
}

void CoreModuleView::createPluginList()
{
    // Create a container widget for the list with padding
    QWidget* listContainer = new QWidget(this);
    listContainer->setObjectName("listContainer");
    listContainer->setStyleSheet(
        "QWidget#listContainer {"
        "   background-color: #2d2d2d;"
        "   border-radius: 8px;"
        "   padding: 20px;"
        "   border: 1px solid #3d3d3d;"
        "}"
    );
    QVBoxLayout* containerLayout = new QVBoxLayout(listContainer);
    containerLayout->setContentsMargins(20, 20, 20, 20);
    
    // Create the list widget
    m_pluginList = new QListWidget(listContainer);
    m_pluginList->setMinimumHeight(300);
    m_pluginList->setFrameShape(QFrame::NoFrame); // Remove the default border
    m_pluginList->setStyleSheet(
        "QListWidget {"
        "   background-color: transparent;"
        "   border: none;"
        "}"
        "QListWidget::item {"
        "   color: #e0e0e0;"
        "   padding: 12px;"
        "   margin: 2px 0px;"
        "   border-radius: 6px;"
        "   background-color: #363636;"
        "}"
        "QListWidget::item:hover {"
        "   background-color: #404040;"
        "}"
        "QListWidget::item:selected {"
        "   background-color: #404040;"
        "   color: #ffffff;"
        "}"
    );
    
    containerLayout->addWidget(m_pluginList);
    m_layout->addWidget(listContainer);
}

void CoreModuleView::updatePluginList()
{
    // Use smart pointer with custom deleter for automatic cleanup
    std::unique_ptr<char*, PluginsArrayDeleter> plugins(logos_core_get_loaded_plugins());
    
    // Clear the current list
    m_pluginList->clear();
    
    if (!plugins || !plugins.get()[0]) {
        QListWidgetItem* item = new QListWidgetItem("No plugins loaded");
        item->setIcon(QIcon(":/icons/plugin.png"));
        item->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        m_pluginList->addItem(item);
        return;
    }
    
    // Add each plugin to the list
    for (char** p = plugins.get(); *p != nullptr; ++p) {
        QString pluginName = QString::fromUtf8(*p);
        QListWidgetItem* item = new QListWidgetItem(pluginName);
        item->setIcon(QIcon(":/icons/plugin.png"));
        item->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        m_pluginList->addItem(item);
    }
} 