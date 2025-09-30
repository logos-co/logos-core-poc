#include "logos_core.h"
#include <QCoreApplication>
#include <QPluginLoader>
#include <QObject>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QMetaProperty>
#include <QMetaMethod>
#include <QTimer>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QHash>
#include <QRemoteObjectRegistryHost>
#include <QProcess>
#include <QLocalSocket>
#include <QLocalServer>
#include <QUuid>
#include <QThread>
#include "../interface.h"
#include "core_manager/core_manager.h"
#include "../../logos-cpp-sdk/cpp/logos_api.h"
#include "../../logos-cpp-sdk/cpp/logos_api_provider.h"
#include "../../logos-cpp-sdk/cpp/logos_api_client.h"
#include "../../logos-cpp-sdk/cpp/token_manager.h"

// Declare QObject* as a metatype so it can be stored in QVariant
Q_DECLARE_METATYPE(QObject*)

// Global application pointer
static QCoreApplication* g_app = nullptr;

// Custom plugins directory
static QString g_plugins_dir = "";

// Global list to store loaded plugin names
static QStringList g_loaded_plugins;

// Global hash to store known plugin names and paths
static QHash<QString, QString> g_known_plugins;

// Global hash to store plugin processes
static QHash<QString, QProcess*> g_plugin_processes;

// Global Qt Remote Object registry host
static QRemoteObjectRegistryHost* g_registry_host = nullptr;

// Structure to store event listener information
struct EventListener {
    QString pluginName;
    QString eventName;
    AsyncCallback callback;
    void* userData;
};

// Global list to store registered event listeners
static QList<EventListener> g_event_listeners;

// Helper function to process a plugin and extract its metadata
static QString processPlugin(const QString &pluginPath)
{
    qDebug() << "\n------------------------------------------";
    qDebug() << "Processing plugin from:" << pluginPath;

    // Load the plugin metadata without instantiating the plugin
    QPluginLoader loader(pluginPath);

    // Read the metadata
    QJsonObject metadata = loader.metaData();
    if (metadata.isEmpty()) {
        qWarning() << "No metadata found for plugin:" << pluginPath;
        return QString();
    }

    // Read our custom metadata from the metadata.json file
    QJsonObject customMetadata = metadata.value("MetaData").toObject();
    if (customMetadata.isEmpty()) {
        qWarning() << "No custom metadata found for plugin:" << pluginPath;
        return QString();
    }

    QString pluginName = customMetadata.value("name").toString();
    if (pluginName.isEmpty()) {
        qWarning() << "Plugin name not specified in metadata for:" << pluginPath;
        return QString();
    }

    qDebug() << "Plugin Metadata:";
    qDebug() << " - Name:" << pluginName;
    qDebug() << " - Version:" << customMetadata.value("version").toString();
    qDebug() << " - Description:" << customMetadata.value("description").toString();
    qDebug() << " - Author:" << customMetadata.value("author").toString();
    qDebug() << " - Type:" << customMetadata.value("type").toString();
    
    // Log capabilities
    QJsonArray capabilities = customMetadata.value("capabilities").toArray();
    if (!capabilities.isEmpty()) {
        qDebug() << " - Capabilities:";
        for (const QJsonValue &cap : capabilities) {
            qDebug() << "   *" << cap.toString();
        }
    }

    // Check dependencies
    QJsonArray dependencies = customMetadata.value("dependencies").toArray();
    if (!dependencies.isEmpty()) {
        qDebug() << " - Dependencies:";
        for (const QJsonValue &dep : dependencies) {
            QString dependency = dep.toString();
            qDebug() << "   *" << dependency;
            if (!g_loaded_plugins.contains(dependency)) {
                qWarning() << "Required dependency not loaded:" << dependency;
            }
        }
    }

    // Store the plugin in the known plugins hash
    g_known_plugins.insert(pluginName, pluginPath);
    qDebug() << "Added to known plugins: " << pluginName << " -> " << pluginPath;
    
    return pluginName;
}

// Helper function to load a plugin by name using logos_host in a separate process
static bool loadPlugin(const QString &pluginName)
{
    if (!g_known_plugins.contains(pluginName)) {
        qWarning() << "Cannot load unknown plugin:" << pluginName;
        return false;
    }

    QString pluginPath = g_known_plugins.value(pluginName);
    qDebug() << "Loading plugin:" << pluginName << "from path:" << pluginPath << "in separate process";

    // Check if plugin is already loaded
    if (g_plugin_processes.contains(pluginName)) {
        qWarning() << "Plugin already loaded:" << pluginName;
        return false;
    }

    // Find the logos_host executable with multiple strategies
    QString logosHostPath;
    // 1) Environment override
    QByteArray envPathBytes = qgetenv("LOGOS_HOST_PATH");
    if (!envPathBytes.isEmpty()) {
        logosHostPath = QString::fromUtf8(envPathBytes);
    }
    // 2) Default next to Electron/host executable
    if (logosHostPath.isEmpty()) {
        logosHostPath = QDir::cleanPath(QCoreApplication::applicationDirPath() + "/logos_host");
    }
#ifdef Q_OS_WIN
    if (!logosHostPath.endsWith(".exe")) {
        logosHostPath += ".exe";
    }
#endif

    // 3) Fallback relative to plugins directory (../../logos-liblogos/build/bin/logos_host)
    if (!QFile::exists(logosHostPath)) {
        if (!g_plugins_dir.isEmpty()) {
            QDir pluginsDirCandidate(g_plugins_dir);
            QString candidate = QDir::cleanPath(pluginsDirCandidate.absoluteFilePath("../bin/logos_host"));
#ifdef Q_OS_WIN
            if (!candidate.endsWith(".exe")) candidate += ".exe";
#endif
            if (QFile::exists(candidate)) {
                logosHostPath = candidate;
            }
        }
    }

    qDebug() << "Logos host path (resolved):" << logosHostPath;

    // Check if logos_host exists
    if (!QFile::exists(logosHostPath)) {
        qCritical() << "logos_host executable not found at:" << logosHostPath;
        qCritical() << "Set environment variable LOGOS_HOST_PATH to the absolute path of logos_host or ensure it is next to the Electron executable or under ../bin from the plugins directory.";
        return false;
    }

    // Create a new process for the plugin
    QProcess* process = new QProcess();
    
    // Set up the process to capture output (merge stdout and stderr)
    process->setProcessChannelMode(QProcess::MergedChannels);
    
    // Set up arguments for logos_host
    QStringList arguments;
    arguments << "--name" << pluginName;
    arguments << "--path" << pluginPath;

    qDebug() << "Starting logos_host with arguments:" << arguments;

    // Start the process
    process->start(logosHostPath, arguments);

    if (!process->waitForStarted(5000)) { // Wait up to 5 seconds for the process to start
        qCritical() << "Failed to start logos_host process:" << process->errorString();
        delete process;
        return false;
    }

    qDebug() << "Logos host process started successfully for plugin:" << pluginName;
    qDebug() << "Process ID:" << process->processId();

    // Set up IPC to securely send the auth token
    QString socketName = QString("logos_token_%1").arg(pluginName);
    QLocalSocket* tokenSocket = new QLocalSocket();

    // Try to connect to the socket (with retries since the process might need time to set up)
    bool connected = false;
    for (int i = 0; i < 10; ++i) { // Try for up to 1 second
        tokenSocket->connectToServer(socketName);
        if (tokenSocket->waitForConnected(100)) {
            connected = true;
            break;
        }
        QThread::msleep(100); // Wait 100ms before retry
    }

    if (!connected) {
        qCritical() << "Failed to connect to token socket for plugin:" << pluginName;
        tokenSocket->deleteLater();
        process->terminate();
        delete process;
        return false;
    }

    // generate a guid and print it
    QUuid authToken = QUuid::createUuid();
    QString authTokenString = authToken.toString(QUuid::WithoutBraces);
    qDebug() << "Generated auth token:" << authTokenString;

    // Send the auth token securely via IPC
    QByteArray tokenData = QString(authTokenString).toUtf8();
    tokenSocket->write(tokenData);
    tokenSocket->waitForBytesWritten(1000);
    tokenSocket->disconnectFromServer();
    tokenSocket->deleteLater();

    qDebug() << "Auth token sent securely to plugin:" << pluginName;

    // Store the process
    g_plugin_processes.insert(pluginName, process);

    // Add the plugin name to our loaded plugins list
    g_loaded_plugins.append(pluginName);

    TokenManager& tokenManager = TokenManager::instance();
    tokenManager.saveToken(pluginName, authTokenString);

    // call InformModuleToken on Capability Module
    if (g_loaded_plugins.contains("capability_module")) {
        qDebug() << "Informing capability module about new module token for:" << pluginName;

        // Create LogosAPI instance to connect to the capability module
        LogosAPI* coreAPI = new LogosAPI("core");

        // Use a timer to ensure the capability module is ready
        QTimer* informTimer = new QTimer();
        informTimer->setSingleShot(true);
        informTimer->setInterval(1000); // 1 second delay to ensure connection is ready

        // get token for capability_module
        QString capabilityModuleToken = tokenManager.getToken("capability_module");
        qDebug() << "Capability module token:" << capabilityModuleToken;

        QObject::connect(informTimer, &QTimer::timeout, [=]() {
            if (coreAPI->getClient("capability_module")->isConnected()) {
                qDebug() << "Calling informModuleToken on capability module";
                
                // Call informModuleToken with the core auth token, module name, and module token
                bool success = coreAPI->getClient("capability_module")->informModuleToken(capabilityModuleToken, pluginName, authTokenString);
                if (success) {
                    qDebug() << "Successfully informed capability module about token for:" << pluginName;
                } else {
                    qWarning() << "Failed to inform capability module about token for:" << pluginName;
                }
            } else {
                qWarning() << "Failed to connect to capability module for token notification";
            }
            
            // Clean up
            coreAPI->deleteLater();
            informTimer->deleteLater();
        });
        
        informTimer->start();
    } else {
        qDebug() << "Capability module not loaded, skipping token notification";
    }

    //

    // Connect to process finished signal for cleanup
    QObject::connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
                     [pluginName, process](int exitCode, QProcess::ExitStatus exitStatus) {
                         qDebug() << "Plugin process finished:" << pluginName 
                                  << "Exit code:" << exitCode 
                                  << "Exit status:" << exitStatus;
                         
                         // TODO: This is temporary and later needs a mechanism to restart the process
                         if (exitStatus == QProcess::CrashExit) {
                             qCritical() << "Plugin process crashed:" << pluginName << "- terminating core with error";
                             exit(1);
                         }
                         
                         // Remove from our tracking lists
                         g_plugin_processes.remove(pluginName);
                         g_loaded_plugins.removeAll(pluginName);
                         
                         // Clean up the process object
                         process->deleteLater();
                     });

    // Connect to error signal
    QObject::connect(process, &QProcess::errorOccurred,
                     [pluginName](QProcess::ProcessError error) {
                         qCritical() << "Plugin process error for" << pluginName << ":" << error;
                         
                         // TODO: This is temporary and later needs a mechanism to restart the process
                         if (error == QProcess::Crashed) {
                             qCritical() << "Plugin process crashed:" << pluginName << "- terminating core with error";
                             exit(1);
                         }
                     });

    // Connect to output signals to forward logs from logos_host to main process
    QObject::connect(process, &QProcess::readyReadStandardOutput,
                     [pluginName, process]() {
                         QByteArray output = process->readAllStandardOutput();
                         if (!output.isEmpty()) {
                             // Forward logs to main process with plugin prefix
                             QString logLine = QString::fromUtf8(output).trimmed();
                             QStringList lines = logLine.split('\n', Qt::SkipEmptyParts);
                             for (const QString &line : lines) {
                                 // Parse the Qt log level from the line and forward appropriately
                                 if (line.contains("qrc:") || line.contains("Warning:") || line.contains("WARNING:")) {
                                     qWarning() << "[LOGOS_HOST" << pluginName << "]:" << line;
                                 } else if (line.contains("Critical:") || line.contains("FAILED:") || line.contains("ERROR:")) {
                                     qCritical() << "[LOGOS_HOST" << pluginName << "]:" << line;
                                 } else {
                                     qDebug() << "[LOGOS_HOST" << pluginName << "]:" << line;
                                 }
                             }
                         }
                     });

    // Connect to stderr output (in case channel mode changes)
    QObject::connect(process, &QProcess::readyReadStandardError,
                     [pluginName, process]() {
                         QByteArray output = process->readAllStandardError();
                         if (!output.isEmpty()) {
                             // Forward error logs to main process with plugin prefix
                             QString logLine = QString::fromUtf8(output).trimmed();
                             QStringList lines = logLine.split('\n', Qt::SkipEmptyParts);
                             for (const QString &line : lines) {
                                 qCritical() << "[LOGOS_HOST" << pluginName << "] STDERR:" << line;
                             }
                         }
                     });

    qDebug() << "Plugin" << pluginName << "is now running in separate process";
    qDebug() << "Remote registry URL for this plugin: local:logos_" << pluginName;
    
    return true;
}

// Helper function to load and process a plugin
static void loadAndProcessPlugin(const QString &pluginPath)
{
    // First process the plugin to get its metadata
    QString pluginName = processPlugin(pluginPath);
    
    // If we found the name, load the plugin
    if (!pluginName.isEmpty()) {
        loadPlugin(pluginName);
    } else {
        qWarning() << "Failed to process plugin:" << pluginPath;
    }
}

// Helper function to find and load all plugins in a directory
static QStringList findPlugins(const QString &pluginsDir)
{
    QDir dir(pluginsDir);
    QStringList plugins;
    
    qDebug() << "Searching for plugins in:" << dir.absolutePath();
    
    if (!dir.exists()) {
        qWarning() << "Plugins directory does not exist:" << dir.absolutePath();
        return plugins;
    }
    
    // Get all files in the directory
    QStringList entries = dir.entryList(QDir::Files);
    qDebug() << "Files found:" << entries;
    
    // Filter for plugin files based on platform
    QStringList nameFilters;
#ifdef Q_OS_WIN
    nameFilters << "*.dll";
#elif defined(Q_OS_MAC)
    nameFilters << "*.dylib";
#else
    nameFilters << "*.so";
#endif
    
    dir.setNameFilters(nameFilters);
    QStringList pluginFiles = dir.entryList(QDir::Files);
    
    for (const QString &fileName : pluginFiles) {
        QString filePath = dir.absoluteFilePath(fileName);
        plugins.append(filePath);
        qDebug() << "Found plugin:" << filePath;
    }
    
    return plugins;
}

static bool initializeCapabilityModule()
{
    qDebug() << "\n=== Initializing Capability Module ===";

    // Check if capability_module is available in known plugins
    if (!g_known_plugins.contains("capability_module")) {
        qDebug() << "Capability module not found in known plugins, skipping initialization";
        return false;
    }

    qDebug() << "Capability module found, attempting to load...";

    // Load the capability module
    bool success = loadPlugin("capability_module");
    if (success) {
        qDebug() << "Capability module loaded successfully";
        
        // Inform capability module about core_manager token
        if (g_loaded_plugins.contains("core_manager")) {
            qDebug() << "Informing capability module about core_manager token";
            
            // Get the core_manager token from TokenManager
            TokenManager& tokenManager = TokenManager::instance();
            QString coreManagerToken = tokenManager.getToken("core_manager");
            
            if (!coreManagerToken.isEmpty()) {
                // Create LogosAPI instance to connect to the capability module
                LogosAPI* coreAPI = new LogosAPI("core");
                
                // Use a timer to ensure the capability module is ready
                QTimer* informTimer = new QTimer();
                informTimer->setSingleShot(true);
                informTimer->setInterval(1000); // 1 second delay to ensure connection is ready
                
                // get token for capability_module
                QString capabilityModuleToken = tokenManager.getToken("capability_module");
                qDebug() << "Capability module token:" << capabilityModuleToken;

                // get token for core_manager
                QString coreManagerToken = tokenManager.getToken("core_manager");
                qDebug() << "Core manager token:" << coreManagerToken;
                
                QObject::connect(informTimer, &QTimer::timeout, [=]() {
                    if (coreAPI->getClient("capability_module")->isConnected()) {
                        qDebug() << "Calling informModuleToken on capability module for core_manager";
                        
                        // Call informModuleToken with the core auth token, module name, and module token
                        bool success = coreAPI->getClient("capability_module")->informModuleToken(capabilityModuleToken, "core_manager", coreManagerToken);
                        if (success) {
                            qDebug() << "Successfully informed capability module about core_manager token";
                        } else {
                            qWarning() << "Failed to inform capability module about core_manager token";
                        }
                    } else {
                        qWarning() << "Failed to connect to capability module for core_manager token notification";
                    }
                    
                    // Clean up
                    coreAPI->deleteLater();
                    informTimer->deleteLater();
                });
                
                informTimer->start();
            } else {
                qWarning() << "No token found for core_manager, skipping capability module notification";
            }
        } else {
            qDebug() << "Core manager not loaded, skipping token notification";
        }
        
        return true;
    } else {
        qWarning() << "Failed to load capability module";
        return false;
    }
}

// Helper function to initialize core manager
static bool initializeCoreManager()
{
    qDebug() << "\n=== Initializing Core Manager ===";
    
    // Create the core manager instance directly
    CoreManagerPlugin* coreManager = new CoreManagerPlugin();
    
    // Create LogosAPI instance for core manager registration
    LogosAPI* coreAPI = new LogosAPI("core_manager");
    
    // Register the core manager using the new API (which will wrap it with ModuleProxy)
    bool success = coreAPI->getProvider()->registerObject(coreManager->name(), coreManager);
    if (success) {
        qDebug() << "Core manager registered using new API with name:" << coreManager->name();
        // generate token
        QUuid coreManagerToken = QUuid::createUuid();
        QString coreManagerTokenString = coreManagerToken.toString(QUuid::WithoutBraces);
        qDebug() << "Generated core manager token:" << coreManagerTokenString;
        // TODO: replace this, using test token
        coreAPI->getTokenManager()->saveToken("core_manager", coreManagerTokenString);
        qDebug() << "Test token saved for core access";
    } else {
        qWarning() << "Failed to register core manager using new API";
        delete coreAPI;
        return false;
    }
    
    // Add to loaded plugins list
    g_loaded_plugins.append(coreManager->name());
    
    qDebug() << "Core manager initialized successfully";
    return true;
}

void logos_core_init(int argc, char *argv[])
{
    // Create the application instance
    g_app = new QCoreApplication(argc, argv);
    
    // Register QObject* as a metatype
    qRegisterMetaType<QObject*>("QObject*");
}

void logos_core_set_plugins_dir(const char* plugins_dir)
{
    if (plugins_dir) {
        g_plugins_dir = QString(plugins_dir);
        qDebug() << "Custom plugins directory set to:" << g_plugins_dir;
    }
}

void logos_core_start()
{
    qDebug() << "Simple Plugin Example";
    qDebug() << "Current directory:" << QDir::currentPath();
    
    // Clear the list of loaded plugins before loading new ones
    g_loaded_plugins.clear();
    
    // Initialize Qt Remote Object registry host
    if (!g_registry_host) {
        g_registry_host = new QRemoteObjectRegistryHost(QUrl(QStringLiteral("local:logos_core_manager")));
        qDebug() << "Qt Remote Object registry host initialized at: local:logos_core_manager";
    }
    
    // First initialize the core manager
    if (!initializeCoreManager()) {
        qWarning() << "Failed to initialize core manager, continuing with other modules...";
    }
    
    // Define the plugins directory path
    QString pluginsDir;
    if (!g_plugins_dir.isEmpty()) {
        // Use the custom plugins directory if set
        pluginsDir = g_plugins_dir;
    } else {
        // Use the default plugins directory
        pluginsDir = QDir::cleanPath(QCoreApplication::applicationDirPath() + "/../modules");
    }
    qDebug() << "Looking for modules in:" << pluginsDir;
    
    // Find and process all plugins in the directory to populate g_known_plugins
    QStringList pluginPaths = findPlugins(pluginsDir);
    
    if (pluginPaths.isEmpty()) {
        qWarning() << "No modules found in:" << pluginsDir;
    } else {
        qDebug() << "Found" << pluginPaths.size() << "modules";
        
        // Process each plugin to add to known plugins list
        for (const QString &pluginPath : pluginPaths) {
            processPlugin(pluginPath);
        }

        // Initialize capability module if available (after plugin discovery)
        initializeCapabilityModule();
    }
}

int logos_core_exec()
{
    if (g_app) {
        return g_app->exec();
    }
    return -1;
}

void logos_core_cleanup()
{
    // Terminate all plugin processes
    qDebug() << "Terminating all plugin processes...";
    for (auto it = g_plugin_processes.begin(); it != g_plugin_processes.end(); ++it) {
        QProcess* process = it.value();
        QString pluginName = it.key();
        
        qDebug() << "Terminating plugin process:" << pluginName;
        process->terminate();
        
        if (!process->waitForFinished(3000)) {
            qWarning() << "Process did not terminate gracefully, killing it:" << pluginName;
            process->kill();
            process->waitForFinished(1000);
        }
        
        delete process;
    }
    g_plugin_processes.clear();
    g_loaded_plugins.clear();
    
    // Clean up Qt Remote Object registry host
    if (g_registry_host) {
        delete g_registry_host;
        g_registry_host = nullptr;
        qDebug() << "Qt Remote Object registry host cleaned up";
    }
    
    delete g_app;
    g_app = nullptr;
}

// Implementation of the function to get loaded plugins
char** logos_core_get_loaded_plugins()
{
    int count = g_loaded_plugins.size();
    
    if (count == 0) {
        // Return an array with just a NULL terminator
        char** result = new char*[1];
        result[0] = nullptr;
        return result;
    }
    
    // Allocate memory for the array of strings
    char** result = new char*[count + 1];  // +1 for null terminator
    
    // Copy each plugin name
    for (int i = 0; i < count; ++i) {
        QByteArray utf8Data = g_loaded_plugins[i].toUtf8();
        result[i] = new char[utf8Data.size() + 1];
        strcpy(result[i], utf8Data.constData());
    }
    
    // Null-terminate the array
    result[count] = nullptr;
    
    return result;
}

// Implementation of the function to get known plugins
char** logos_core_get_known_plugins()
{
    // Get the keys from the hash (plugin names)
    QStringList knownPlugins = g_known_plugins.keys();
    int count = knownPlugins.size();
    
    if (count == 0) {
        // Return an array with just a NULL terminator
        char** result = new char*[1];
        result[0] = nullptr;
        return result;
    }
    
    // Allocate memory for the array of strings
    char** result = new char*[count + 1];  // +1 for null terminator
    
    // Copy each plugin name
    for (int i = 0; i < count; ++i) {
        QByteArray utf8Data = knownPlugins[i].toUtf8();
        result[i] = new char[utf8Data.size() + 1];
        strcpy(result[i], utf8Data.constData());
    }
    
    // Null-terminate the array
    result[count] = nullptr;
    
    return result;
}

// Implementation of the function to load a plugin by name
int logos_core_load_plugin(const char* plugin_name)
{
    if (!plugin_name) {
        qWarning() << "Cannot load plugin: name is null";
        return 0;
    }
    
    QString name = QString::fromUtf8(plugin_name);
    qDebug() << "Attempting to load plugin by name:" << name;
    
    // Check if plugin exists in known plugins
    if (!g_known_plugins.contains(name)) {
        qWarning() << "Plugin not found among known plugins:" << name;
        return 0;
    }
    
    // Use our internal loadPlugin function
    bool success = loadPlugin(name);
    return success ? 1 : 0;
}

// Implementation of the function to unload a plugin by name
int logos_core_unload_plugin(const char* plugin_name)
{
    if (!plugin_name) {
        qWarning() << "Cannot unload plugin: name is null";
        return 0;
    }

    QString name = QString::fromUtf8(plugin_name);
    qDebug() << "Attempting to unload plugin by name:" << name;

    // Check if plugin is loaded
    if (!g_loaded_plugins.contains(name)) {
        qWarning() << "Plugin not loaded, cannot unload:" << name;
        qDebug() << "Loaded plugins:" << g_loaded_plugins;
        return 0;
    }

    // Check if we have a process for this plugin
    if (!g_plugin_processes.contains(name)) {
        qWarning() << "No process found for plugin:" << name;
        return 0;
    }

    // Get the process
    QProcess* process = g_plugin_processes.value(name);
    
    qDebug() << "Terminating plugin process for:" << name;
    
    // Terminate the process gracefully
    process->terminate();
    
    // Wait for the process to finish, with a timeout
    if (!process->waitForFinished(5000)) {
        qWarning() << "Process did not terminate gracefully, killing it";
        process->kill();
        process->waitForFinished(2000);
    }

    // Remove from our tracking structures
    g_plugin_processes.remove(name);
    g_loaded_plugins.removeAll(name);
    
    // The process will be cleaned up by the signal handler
    qDebug() << "Successfully unloaded plugin:" << name;
    return 1;
}

// TODO: this function can probably go to the core manager instead
char* logos_core_process_plugin(const char* plugin_path)
{
    if (!plugin_path) {
        qWarning() << "Cannot process plugin: path is null";
        return nullptr;
    }

    QString path = QString::fromUtf8(plugin_path);
    qDebug() << "Processing plugin file:" << path;

    QString pluginName = processPlugin(path);
    if (pluginName.isEmpty()) {
        qWarning() << "Failed to process plugin file:" << path;
        return nullptr;
    }

    // Convert to C string that must be freed by the caller
    QByteArray utf8Data = pluginName.toUtf8();
    char* result = new char[utf8Data.size() + 1];
    strcpy(result, utf8Data.constData());

    return result;
}

// Implementation of the function to get a token by key
char* logos_core_get_token(const char* key)
{
    if (!key) {
        qWarning() << "Cannot get token: key is null";
        return nullptr;
    }

    QString keyStr = QString::fromUtf8(key);
    qDebug() << "Getting token for key:" << keyStr;

    // Get the token from the TokenManager singleton
    TokenManager& tokenManager = TokenManager::instance();
    QString token = tokenManager.getToken(keyStr);

    if (token.isEmpty()) {
        qDebug() << "No token found for key:" << keyStr;
        return nullptr;
    }

    qDebug() << "Token found for key:" << keyStr;

    // Convert to C string that must be freed by the caller
    QByteArray utf8Data = token.toUtf8();
    char* result = new char[utf8Data.size() + 1];
    strcpy(result, utf8Data.constData());

    return result;
}

// === Async Callback API Implementation ===

void logos_core_async_operation(const char* data, AsyncCallback callback, void* user_data)
{
    if (!callback) {
        qWarning() << "logos_core_async_operation: callback is null";
        return;
    }
    
    QString inputData = data ? QString::fromUtf8(data) : QString("no data");
    qDebug() << "Starting async operation with data:" << inputData;
    
    // Create a timer to simulate async work
    QTimer* timer = new QTimer();
    timer->setSingleShot(true);
    timer->setInterval(2000); // 2 second delay
    
    // Connect the timer to execute the callback
    QObject::connect(timer, &QTimer::timeout, [=]() {
        qDebug() << "Async operation completed for data:" << inputData;
        
        // Create the result message
        QString resultMessage = QString("Async operation completed successfully for: %1").arg(inputData);
        QByteArray messageBytes = resultMessage.toUtf8();
        
        // Call the callback with success (1) and message
        callback(1, messageBytes.constData(), user_data);
        
        // Clean up the timer
        timer->deleteLater();
    });
    
    timer->start();
    qDebug() << "Async operation timer started, will complete in 2 seconds";
}

void logos_core_load_plugin_async(const char* plugin_name, AsyncCallback callback, void* user_data)
{
    if (!callback) {
        qWarning() << "logos_core_load_plugin_async: callback is null";
        return;
    }
    
    if (!plugin_name) {
        qWarning() << "logos_core_load_plugin_async: plugin_name is null";
        callback(0, "Plugin name is null", user_data);
        return;
    }
    
    QString name = QString::fromUtf8(plugin_name);
    qDebug() << "Starting async plugin load for:" << name;
    
    // Check if plugin exists in known plugins
    if (!g_known_plugins.contains(name)) {
        QString errorMsg = QString("Plugin not found among known plugins: %1").arg(name);
        QByteArray errorBytes = errorMsg.toUtf8();
        callback(0, errorBytes.constData(), user_data);
        return;
    }
    
    // Create a timer to simulate async plugin loading
    QTimer* timer = new QTimer();
    timer->setSingleShot(true);
    timer->setInterval(1000); // 1 second delay to simulate work
    
    // Connect the timer to execute the actual plugin loading
    QObject::connect(timer, &QTimer::timeout, [=]() {
        qDebug() << "Executing async plugin load for:" << name;
        
        // Use our existing synchronous loadPlugin function
        bool success = loadPlugin(name);
        
        QString resultMessage;
        if (success) {
            resultMessage = QString("Plugin '%1' loaded successfully").arg(name);
        } else {
            resultMessage = QString("Failed to load plugin '%1'").arg(name);
        }
        
        QByteArray messageBytes = resultMessage.toUtf8();
        
        // Call the callback with the result
        callback(success ? 1 : 0, messageBytes.constData(), user_data);
        
        // Clean up the timer
        timer->deleteLater();
    });
    
    timer->start();
    qDebug() << "Async plugin load timer started for:" << name;
}

// Helper function to convert JSON parameter to QVariant
static QVariant jsonParamToQVariant(const QJsonObject& param) {
    QString name = param.value("name").toString();
    QString value = param.value("value").toString();
    QString type = param.value("type").toString();
    
    qDebug() << "Converting param:" << name << "value:" << value << "type:" << type;
    
    if (type == "string" || type == "QString") {
        return QVariant(value);
    } else if (type == "int" || type == "integer") {
        bool ok;
        int intValue = value.toInt(&ok);
        return ok ? QVariant(intValue) : QVariant();
    } else if (type == "bool" || type == "boolean") {
        if (value.toLower() == "true" || value == "1") {
            return QVariant(true);
        } else if (value.toLower() == "false" || value == "0") {
            return QVariant(false);
        }
        return QVariant();
    } else if (type == "double" || type == "float") {
        bool ok;
        double doubleValue = value.toDouble(&ok);
        return ok ? QVariant(doubleValue) : QVariant();
    } else {
        // Default to string for unknown types
        qWarning() << "Unknown parameter type:" << type << "- treating as string";
        return QVariant(value);
    }
}

void logos_core_call_plugin_method_async(
    const char* plugin_name, 
    const char* method_name, 
    const char* params_json, 
    AsyncCallback callback, 
    void* user_data
) {
    if (!callback) {
        qWarning() << "logos_core_call_plugin_method_async: callback is null";
        return;
    }
    
    if (!plugin_name || !method_name) {
        qWarning() << "logos_core_call_plugin_method_async: plugin_name or method_name is null";
        callback(0, "Plugin name or method name is null", user_data);
        return;
    }
    
    QString pluginNameStr = QString::fromUtf8(plugin_name);
    QString methodNameStr = QString::fromUtf8(method_name);
    QString paramsJsonStr = params_json ? QString::fromUtf8(params_json) : QString("[]");
    
    qDebug() << "Starting async method call for plugin:" << pluginNameStr 
             << "method:" << methodNameStr 
             << "params:" << paramsJsonStr;
    
    // Check if plugin is loaded
    if (!g_loaded_plugins.contains(pluginNameStr)) {
        QString errorMsg = QString("Plugin not loaded: %1").arg(pluginNameStr);
        QByteArray errorBytes = errorMsg.toUtf8();
        callback(0, errorBytes.constData(), user_data);
        return;
    }
    
    // Create a timer to simulate async method call
    // Create timer with QCoreApplication as parent to ensure it's in the main thread
    QTimer* timer = new QTimer(QCoreApplication::instance());
    timer->setSingleShot(true);
    timer->setInterval(500); // 0.5 second delay to simulate async work
        
        // Connect the timer to execute the actual method call
        QObject::connect(timer, &QTimer::timeout, [=]() {
        qDebug() << "Executing async method call for:" << pluginNameStr << "::" << methodNameStr;
        
        try {
            // Parse the JSON parameters
            QJsonParseError parseError;
            QJsonDocument jsonDoc = QJsonDocument::fromJson(paramsJsonStr.toUtf8(), &parseError);
            
            if (parseError.error != QJsonParseError::NoError) {
                QString errorMsg = QString("JSON parse error: %1").arg(parseError.errorString());
                QByteArray errorBytes = errorMsg.toUtf8();
                callback(0, errorBytes.constData(), user_data);
                timer->deleteLater();
                return;
            }
            
            QJsonArray paramsArray = jsonDoc.array();
            QVariantList args;
            
            // Convert JSON parameters to QVariantList
            for (const QJsonValue& paramValue : paramsArray) {
                if (paramValue.isObject()) {
                    QJsonObject paramObj = paramValue.toObject();
                    QVariant variant = jsonParamToQVariant(paramObj);
                    if (variant.isValid()) {
                        args.append(variant);
                    } else {
                        QString errorMsg = QString("Invalid parameter: %1").arg(paramObj.value("name").toString());
                        QByteArray errorBytes = errorMsg.toUtf8();
                        callback(0, errorBytes.constData(), user_data);
                        timer->deleteLater();
                        return;
                    }
                }
            }
            
            qDebug() << "Converted parameters to QVariantList, count:" << args.size();
            
            // Create LogosAPI instance to make the remote call
            LogosAPI* logosAPI = new LogosAPI("core");
            
            // Use a longer delay to ensure connection is established
            QTimer* connectionTimer = new QTimer();
            connectionTimer->setSingleShot(true);
            connectionTimer->setInterval(2000); // 2 second delay
            
            QObject::connect(connectionTimer, &QTimer::timeout, [=]() {
                if (logosAPI->getClient(pluginNameStr)->isConnected()) {
                    qDebug() << "LogosAPI connected, making remote method call";
                    
                    // Make the remote method call
                    QVariant result = logosAPI->getClient(pluginNameStr)->invokeRemoteMethod(pluginNameStr, methodNameStr, args);
                    
                    QString resultMessage;
                    if (result.isValid()) {
                        // Convert result to string representation
                        QString resultStr;
                        if (result.canConvert<QString>()) {
                            resultStr = result.toString();
                        } else {
                            resultStr = QString("Result of type: %1").arg(result.typeName());
                        }
                        resultMessage = QString("Method call successful. Result: %1").arg(resultStr);
                        
                        QByteArray messageBytes = resultMessage.toUtf8();
                        callback(1, messageBytes.constData(), user_data);
                    } else {
                        resultMessage = QString("Method call returned invalid result");
                        QByteArray messageBytes = resultMessage.toUtf8();
                        callback(0, messageBytes.constData(), user_data);
                    }
                } else {
                    QString errorMsg = QString("Failed to connect to plugin: %1").arg(pluginNameStr);
                    QByteArray errorBytes = errorMsg.toUtf8();
                    callback(0, errorBytes.constData(), user_data);
                }
                
                // Clean up
                logosAPI->deleteLater();
                connectionTimer->deleteLater();
            });
            
            connectionTimer->start();
            
        } catch (const std::exception& e) {
            QString errorMsg = QString("Exception during method call: %1").arg(e.what());
            QByteArray errorBytes = errorMsg.toUtf8();
            callback(0, errorBytes.constData(), user_data);
        } catch (...) {
            QString errorMsg = QString("Unknown exception during method call");
            QByteArray errorBytes = errorMsg.toUtf8();
            callback(0, errorBytes.constData(), user_data);
        }
        
        // Clean up the main timer
        timer->deleteLater();
        });
        
    timer->start();
    qDebug() << "Async method call timer started for:" << pluginNameStr << "::" << methodNameStr;
}

void logos_core_register_event_listener(
    const char* plugin_name,
    const char* event_name, 
    AsyncCallback callback,
    void* user_data
) {
    if (!plugin_name || !event_name || !callback) {
        qWarning() << "logos_core_register_event_listener: Invalid parameters";
        return;
    }
    
    QString pluginNameStr = QString::fromUtf8(plugin_name);
    QString eventNameStr = QString::fromUtf8(event_name);
    
    qDebug() << "Registering event listener for plugin:" << pluginNameStr << "event:" << eventNameStr;
    
    // Check if plugin is loaded
    if (!g_loaded_plugins.contains(pluginNameStr)) {
        qWarning() << "Cannot register event listener: Plugin not loaded:" << pluginNameStr;
        return;
    }
    
    // Create event listener structure
    EventListener listener;
    listener.pluginName = pluginNameStr;
    listener.eventName = eventNameStr;
    listener.callback = callback;
    listener.userData = user_data;
    
    // Add to global list
    g_event_listeners.append(listener);
    
    // Set up the actual Qt Remote Objects event listener
    // Create timer with QCoreApplication as parent to ensure it's in the main thread
    QTimer* setupTimer = new QTimer(QCoreApplication::instance());
    setupTimer->setSingleShot(true);
    setupTimer->setInterval(1000); // Give plugin time to be ready
        
        QObject::connect(setupTimer, &QTimer::timeout, [=]() {
        // Create LogosAPI instance to connect to the plugin
        LogosAPI* logosAPI = new LogosAPI("core");
        
        // Use a delay to ensure connection is established
        QTimer* connectionTimer = new QTimer();
        connectionTimer->setSingleShot(true);
        connectionTimer->setInterval(2000); // 2 second delay
        
        QObject::connect(connectionTimer, &QTimer::timeout, [=]() {
            if (logosAPI->getClient(pluginNameStr)->isConnected()) {
                qDebug() << "LogosAPI connected for event listener, setting up event listener for" << eventNameStr;
                
                // Get the replica object to set up event listener
                QObject* replica = logosAPI->getClient(pluginNameStr)->requestObject(pluginNameStr);
                if (replica) {
                    // Set up event listener for the specified event
                    logosAPI->getClient(pluginNameStr)->onEvent(replica, nullptr, eventNameStr, [=](const QString& eventName, const QVariantList& eventData) {
                        qDebug() << "Event listener captured event:" << eventName << "with data:" << eventData;
                        
                        // Format the event data as JSON for the callback
                        QString eventResponse = QString("{\"event\":\"%1\",\"data\":[").arg(eventName);
                        for (int i = 0; i < eventData.size(); ++i) {
                            if (i > 0) eventResponse += ",";
                            eventResponse += QString("\"%1\"").arg(eventData[i].toString());
                        }
                        eventResponse += "]}";
                        
                        QByteArray eventBytes = eventResponse.toUtf8();
                        callback(1, eventBytes.constData(), user_data);
                    });
                    qDebug() << "Event listener successfully registered for" << pluginNameStr << "::" << eventNameStr;
                } else {
                    qWarning() << "Failed to get replica for event listener setup";
                }
            } else {
                qWarning() << "Failed to connect LogosAPI for event listener:" << pluginNameStr;
            }
            
            // Clean up timers but keep LogosAPI alive for the event listener
            connectionTimer->deleteLater();
        });
        
        connectionTimer->start();
        setupTimer->deleteLater();
        });
        
    setupTimer->start();
    qDebug() << "Event listener setup timer started for:" << pluginNameStr << "::" << eventNameStr;
}

void logos_core_process_events()
{
    if (g_app) {
        g_app->processEvents();
    }
} 
