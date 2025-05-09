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

// Replace CoreManager with direct C API functions
extern "C" {
    void logos_core_set_plugins_dir(const char* plugins_dir);
    void logos_core_start();
    void logos_core_cleanup();
    char** logos_core_get_loaded_plugins();
    int logos_core_load_plugin(const char* plugin_name);
    char* logos_core_process_plugin(const char* plugin_path);
}

// Helper function to convert C-style array to QStringList
QStringList convertPluginsToStringList(char** plugins) {
    QStringList result;
    if (plugins) {
        for (int i = 0; plugins[i] != nullptr; i++) {
            result.append(plugins[i]);
        }
    }
    return result;
}

int main(int argc, char *argv[])
{
    // Create QApplication first
    QApplication app(argc, argv);

    // Set the plugins directory
    QString pluginsDir = QDir::cleanPath(QCoreApplication::applicationDirPath() + "/bin/modules");
    std::cout << "Setting plugins directory to: " << pluginsDir.toStdString() << std::endl;
    logos_core_set_plugins_dir(pluginsDir.toUtf8().constData());

    // Start the core
    logos_core_start();
    std::cout << "Logos Core started successfully!" << std::endl;

    // TODO: this should be refactored
    QString pluginExtension;
#if defined(Q_OS_MAC)
    pluginExtension = ".dylib";
#elif defined(Q_OS_WIN)
    pluginExtension = ".dll";
#else // Linux and others
    pluginExtension = ".so";
#endif

    QString pluginPath = pluginsDir + "/package_manager_plugin" + pluginExtension;
    logos_core_process_plugin(pluginPath.toUtf8().constData());
    bool loaded = logos_core_load_plugin("package_manager");

    if (loaded) {
        qInfo() << "package_manager plugin loaded by default.";
    } else {
        qWarning() << "Failed to load package_manager plugin by default.";
    }

    // Print loaded plugins initially
    char** loadedPlugins = logos_core_get_loaded_plugins();
    QStringList plugins = convertPluginsToStringList(loadedPlugins);

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
    logos_core_cleanup();

    return result;
} 
