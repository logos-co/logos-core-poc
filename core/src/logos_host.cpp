#include <QCoreApplication>
#include <QPluginLoader>
#include <QObject>
#include <QDebug>
#include <QCommandLineParser>
#include "../interface.h"
#include "../../SDK/cpp/logos_api.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    app.setApplicationName("logos_host");
    app.setApplicationVersion("1.0");

    // Setup command line parser
    QCommandLineParser parser;
    parser.setApplicationDescription("Logos host for loading plugins in separate processes");
    parser.addHelpOption();
    parser.addVersionOption();

    // Add plugin name option
    QCommandLineOption pluginNameOption(QStringList() << "n" << "name",
                                       "Name of the plugin to load",
                                       "plugin_name");
    parser.addOption(pluginNameOption);

    // Add plugin path option
    QCommandLineOption pluginPathOption(QStringList() << "p" << "path",
                                       "Path to the plugin file",
                                       "plugin_path");
    parser.addOption(pluginPathOption);

    // Process the command line arguments
    parser.process(app);

    // Get plugin name and path
    QString pluginName = parser.value(pluginNameOption);
    QString pluginPath = parser.value(pluginPathOption);

    if (pluginName.isEmpty() || pluginPath.isEmpty()) {
        qCritical() << "Both plugin name and path must be specified";
        qCritical() << "Usage:" << argv[0] << "--name <plugin_name> --path <plugin_path>";
        return 1;
    }

    qDebug() << "Logos host starting for plugin:" << pluginName;
    qDebug() << "Plugin path:" << pluginPath;

    // Create LogosAPI instance for this plugin
    LogosAPI* api = new LogosAPI(pluginName);
    if (!api) {
        qCritical() << "Failed to create LogosAPI instance";
        return 1;
    }

    qDebug() << "Created LogosAPI instance for plugin:" << pluginName;

    // Load the plugin
    QPluginLoader loader(pluginPath);
    QObject *plugin = loader.instance();

    if (!plugin) {
        qCritical() << "Failed to load plugin:" << loader.errorString();
        delete api;
        return 1;
    }

    qDebug() << "Plugin loaded successfully";

    // Cast to the base PluginInterface
    PluginInterface *basePlugin = qobject_cast<PluginInterface *>(plugin);
    if (!basePlugin) {
        qCritical() << "Plugin does not implement the PluginInterface";
        delete plugin;
        delete api;
        return 1;
    }

    // Verify that the plugin name matches
    if (pluginName != basePlugin->name()) {
        qWarning() << "Plugin name mismatch! Expected:" << pluginName << "Actual:" << basePlugin->name();
    }

    qDebug() << "Plugin name:" << basePlugin->name();
    qDebug() << "Plugin version:" << basePlugin->version();

    // Register the plugin object for remote access
    bool success = api->registerObject(basePlugin->name(), plugin);
    if (success) {
        qDebug() << "Plugin registered for remote access with name:" << basePlugin->name();
    } else {
        qCritical() << "Failed to register plugin for remote access:" << basePlugin->name();
        delete plugin;
        delete api;
        return 1;
    }

    qDebug() << "Logos host ready, entering event loop...";
    
    // Run the application event loop
    int result = app.exec();

    // Cleanup
    delete api;
    qDebug() << "Logos host shutting down";
    
    return result;
} 