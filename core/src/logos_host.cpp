#include <QCoreApplication>
#include <QPluginLoader>
#include <QObject>
#include <QDebug>
#include <QCommandLineParser>
#include <QRemoteObjectRegistryHost>
#include <QLocalSocket>
#include <QLocalServer>
#include "../interface.h"
#include "../../SDK/cpp/logos_api.h"
#include "../../SDK/cpp/logos_api_provider.h"
#include "../../SDK/cpp/token_manager.h"

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

    // Set up IPC server to receive auth token securely
    QString socketName = QString("logos_token_%1").arg(pluginName);
    QLocalServer* tokenServer = new QLocalServer();

    // Remove any existing socket file
    QLocalServer::removeServer(socketName);

    if (!tokenServer->listen(socketName)) {
        qCritical() << "Failed to start token server:" << tokenServer->errorString();
        return 1;
    }

    qDebug() << "Token server started, waiting for auth token...";

    // Wait for connection and receive token
    QString authToken;
    if (tokenServer->waitForNewConnection(10000)) { // Wait up to 10 seconds
        QLocalSocket* clientSocket = tokenServer->nextPendingConnection();
        if (clientSocket->waitForReadyRead(5000)) {
            QByteArray tokenData = clientSocket->readAll();
            authToken = QString::fromUtf8(tokenData);
            qDebug() << "Auth token received securely";
        }
        clientSocket->deleteLater();
    } else {
        qCritical() << "Timeout waiting for auth token";
        tokenServer->deleteLater();
        return 1;
    }

    tokenServer->deleteLater();

    if (authToken.isEmpty()) {
        qCritical() << "No auth token received";
        return 1;
    }

    // Initialize LogosAPI for this plugin
    LogosAPI* logos_api = new LogosAPI(pluginName);

    if (!logos_api) {
        qCritical() << "Failed to create LogosAPI instance";
        return 1;
    }

    qDebug() << "LogosAPI initialized for plugin:" << pluginName;

    // Load the plugin
    QPluginLoader loader(pluginPath);
    QObject *plugin = loader.instance();

    if (!plugin) {
        qCritical() << "Failed to load plugin:" << loader.errorString();
        delete logos_api;
        return 1;
    }

    qDebug() << "Plugin loaded successfully";

    // Cast to the base PluginInterface
    PluginInterface *basePlugin = qobject_cast<PluginInterface *>(plugin);
    if (!basePlugin) {
        qCritical() << "Plugin does not implement the PluginInterface";
        delete plugin;
        delete logos_api;
        return 1;
    }

    // Verify that the plugin name matches
    if (pluginName != basePlugin->name()) {
        qWarning() << "Plugin name mismatch! Expected:" << pluginName << "Actual:" << basePlugin->name();
    }

    qDebug() << "Plugin name:" << basePlugin->name();
    qDebug() << "Plugin version:" << basePlugin->version();

    // Register the plugin for remote access using LogosAPI Provider
    bool success = logos_api->getProvider()->registerObject(basePlugin->name(), plugin);
    if (success) {
        qDebug() << "Plugin registered for remote access with name:" << basePlugin->name();
        // Save the auth token using the TokenManager
        logos_api->getTokenManager()->saveToken("core", authToken);
        qDebug() << "Auth token saved for core access";
    } else {
        qCritical() << "Failed to register plugin for remote access:" << basePlugin->name();
        delete plugin;
        delete logos_api;
        return 1;
    }

    qDebug() << "Logos host ready, entering event loop...";
    
    // Run the application event loop
    int result = app.exec();

    // Cleanup
    delete logos_api;
    qDebug() << "Logos host shutting down";

    return result;
} 