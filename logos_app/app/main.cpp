#include "window.h"
#include <QApplication>
#include <QIcon>
#include <QDir>
#include <QTimer>
#include <iostream>
#include <memory>
#include <QStringList>
#include <QDebug>
#include <QMetaObject>
#include "core_manager.h"
#include "../../core/plugin_registry.h"

int main(int argc, char *argv[])
{
    // Create QApplication first
    QApplication app(argc, argv);


    // TODO: this usage of CoreManager seems incorrect, as it's likely creating two instances, one here
    // and another in the core_manager plugin itself, so things like setPluginsDirectory is not working properly

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

    // Load package_manager plugin by default
    bool loaded = core.loadPlugin("package_manager");
    if (loaded) {
        qInfo() << "package_manager plugin loaded by default.";
    } else {
        qWarning() << "Failed to load package_manager plugin by default.";
    }

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
    Window mainWindow;
    mainWindow.show();

    // Run the application
    int result = app.exec();

    // Cleanup
    core.cleanup();

    return result;
} 
