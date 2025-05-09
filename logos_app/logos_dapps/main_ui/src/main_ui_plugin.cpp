#include "main_ui_plugin.h"
#include "mainwindow.h"
#include "modules_loader/coremoduleview.h"
#include <QDebug>

MainUIPlugin::MainUIPlugin(QObject* parent)
    : QObject(parent)
    , m_mainWindow(nullptr)
    , m_coreModuleView(nullptr)
{
    qDebug() << "MainUIPlugin created";
}

MainUIPlugin::~MainUIPlugin()
{
    qDebug() << "MainUIPlugin destroyed";
    destroyWidget(m_mainWindow);
    if (m_coreModuleView) {
        delete m_coreModuleView;
        m_coreModuleView = nullptr;
    }
}

QWidget* MainUIPlugin::createWidget()
{
    if (!m_mainWindow) {
        m_mainWindow = new MainWindow();
    }
    return m_mainWindow;
}

void MainUIPlugin::destroyWidget(QWidget* widget)
{
    if (widget) {
        delete widget;
        if (widget == m_mainWindow) {
            m_mainWindow = nullptr;
        }
    }
}

CoreModuleView* MainUIPlugin::createCoreModuleView(QWidget* parent)
{
    if (!m_coreModuleView) {
        m_coreModuleView = new CoreModuleView(parent);
    }
    return m_coreModuleView;
} 