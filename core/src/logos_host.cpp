#include <QCoreApplication>
#include <QPluginLoader>
#include <QObject>
#include <QDebug>
#include <QCommandLineParser>
#include <QRemoteObjectRegistryHost>
#include "../interface.h"

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

    // Create Qt Remote Object registry host with plugin-specific URL
    QString registryUrl = QString("local:logos_%1").arg(pluginName);
    QRemoteObjectRegistryHost* g_registry_host = new QRemoteObjectRegistryHost(QUrl(registryUrl));
    
    if (!g_registry_host) {
        qCritical() << "Failed to create registry host";
        return 1;
    }

    qDebug() << "Created registry host with URL:" << registryUrl;

    // Load the plugin
    QPluginLoader loader(pluginPath);
    QObject *plugin = loader.instance();

    if (!plugin) {
        qCritical() << "Failed to load plugin:" << loader.errorString();
        delete g_registry_host;
        return 1;
    }

    qDebug() << "Plugin loaded successfully";

    // Cast to the base PluginInterface
    PluginInterface *basePlugin = qobject_cast<PluginInterface *>(plugin);
    if (!basePlugin) {
        qCritical() << "Plugin does not implement the PluginInterface";
        delete plugin;
        delete g_registry_host;
        return 1;
    }

    // Verify that the plugin name matches
    if (pluginName != basePlugin->name()) {
        qWarning() << "Plugin name mismatch! Expected:" << pluginName << "Actual:" << basePlugin->name();
    }

    qDebug() << "Plugin name:" << basePlugin->name();
    qDebug() << "Plugin version:" << basePlugin->version();

    // Enable remote access for the plugin
    bool success = g_registry_host->enableRemoting(plugin, basePlugin->name());
    if (success) {
        qDebug() << "Plugin enabled for remote access with name:" << basePlugin->name();
    } else {
        qCritical() << "Failed to enable remote access for plugin:" << basePlugin->name();
        delete plugin;
        delete g_registry_host;
        return 1;
    }

    qDebug() << "Logos host ready, entering event loop...";
    
    // Run the application event loop
    int result = app.exec();

    // Cleanup
    delete g_registry_host;
    qDebug() << "Logos host shutting down";
    
    return result;
} 