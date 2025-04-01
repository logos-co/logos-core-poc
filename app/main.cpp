#include "ui/mainwindow.h"
#include <QApplication>
#include <QIcon>
#include <QDir>
#include <QTimer>
#include <iostream>
#include <memory>
#include <QStringList>
#include <QDebug>
#include "core_manager.h"

int main(int argc, char *argv[])
{
    // Create QApplication first
    QApplication app(argc, argv);
    
    // Initialize CoreManager
    CoreManager& core = CoreManager::instance();
    core.initialize(argc, argv);

    // Set the plugins directory
    QString pluginsDir = QDir::cleanPath(QCoreApplication::applicationDirPath() + "/bin/plugins");
    std::cout << "Setting plugins directory to: " << pluginsDir.toStdString() << std::endl;
    core.setPluginsDirectory(pluginsDir);

    // Start the core
    core.start();
    std::cout << "Logos Core started successfully!" << std::endl;
    
    // Print loaded plugins initially
    QStringList plugins = core.getLoadedPlugins();
    if (plugins.isEmpty()) {
        qInfo() << "No plugins loaded.";
    } else {
        qInfo() << "Currently loaded plugins:";
        foreach (const QString &plugin, plugins) {
            qInfo() << "  -" << plugin;
        }
        qInfo() << "Total plugins:" << plugins.size();
    }

    // Set application icon
    app.setWindowIcon(QIcon(":/icons/logos.png"));
    
    // Create and show the main window
    MainWindow mainWindow;
    mainWindow.show();
    
    // Run the application
    int result = app.exec();

    // Cleanup
    core.cleanup();
    
    return result;
} 