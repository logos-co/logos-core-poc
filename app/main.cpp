#include "mainwindow.h"
#include <QApplication>
#include <QIcon>
#include <QDir>
#include <QTimer>
#include <iostream>
#include <memory>
#include <QStringList>
#include <QDebug>
#include "../core/host/logos_core.h"

// Forward declare the logos_core functions we need to call directly
extern "C" {
    void logos_core_set_plugins_dir(const char* plugins_dir);
    void logos_core_start();
    void logos_core_cleanup();
    char** logos_core_get_loaded_plugins();
}

void printLoadedPlugins() {
    // Get the array of loaded plugins
    char** plugins = logos_core_get_loaded_plugins();
    
    // Convert to QStringList for easier handling
    QStringList pluginList;
    if (plugins) {
        for (char** p = plugins; *p != nullptr; ++p) {
            pluginList << QString::fromUtf8(*p);
            delete[] *p;  // Free each string as we go
        }
        delete[] plugins;  // Free the array itself
    }
    
    // Display the results using Qt
    if (pluginList.isEmpty()) {
        qInfo() << "No plugins loaded.";
        return;
    }
    
    qInfo() << "Currently loaded plugins:";
    foreach (const QString &plugin, pluginList) {
        qInfo() << "  -" << plugin;
    }
    qInfo() << "Total plugins:" << pluginList.size();
}

int main(int argc, char *argv[])
{
    // Create QApplication first - this will be used by both our UI and logos_core
    QApplication app(argc, argv);
    
    // commenting for now since this seems to create a new instance of QApplication
    // logos_core_init(argc, argv);

    std::cout << "Logos Core initialized successfully!" << std::endl;

    // Set the plugins directory
    QString pluginsDir = QDir::cleanPath(QCoreApplication::applicationDirPath() + "/bin/plugins");
    std::cout << "Setting plugins directory to: " << pluginsDir.toStdString() << std::endl;
    logos_core_set_plugins_dir(pluginsDir.toUtf8().constData());

    // Start logos core
    logos_core_start();
    std::cout << "Logos Core started successfully!" << std::endl;
    
    // Print loaded plugins initially
    std::cout << "\nInitial plugin list:" << std::endl;
    printLoadedPlugins();

    // Set application icon
    app.setWindowIcon(QIcon(":/icons/logos.png"));
    
    // Create and show the main window
    MainWindow mainWindow;
    mainWindow.show();
    
    // Use QApplication::exec() to handle both our UI and logos_core events
    int result = app.exec();

    // Cleanup
    logos_core_cleanup();
    
    return result;
} 