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
#include "../interface.h"
#include "core_manager/core_manager.h"

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

// Helper function to load a plugin by name using module_host in a separate process
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

    // Find the module_host executable
    QString moduleHostPath = QDir::cleanPath(QCoreApplication::applicationDirPath() + "/module_host");
#ifdef Q_OS_WIN
    moduleHostPath += ".exe";
#endif

    qDebug() << "Module host path:" << moduleHostPath;

    // Check if module_host exists
    if (!QFile::exists(moduleHostPath)) {
        qCritical() << "module_host executable not found at:" << moduleHostPath;
        return false;
    }

    // Create a new process for the plugin
    QProcess* process = new QProcess();
    
    // Set up the process to capture output (merge stdout and stderr)
    process->setProcessChannelMode(QProcess::MergedChannels);
    
    // Set up arguments for module_host
    QStringList arguments;
    arguments << "--name" << pluginName;
    arguments << "--path" << pluginPath;

    qDebug() << "Starting module_host with arguments:" << arguments;

    // Start the process
    process->start(moduleHostPath, arguments);

    if (!process->waitForStarted(5000)) { // Wait up to 5 seconds for the process to start
        qCritical() << "Failed to start module_host process:" << process->errorString();
        delete process;
        return false;
    }

    qDebug() << "Module host process started successfully for plugin:" << pluginName;
    qDebug() << "Process ID:" << process->processId();

    // Store the process
    g_plugin_processes.insert(pluginName, process);

    // Add the plugin name to our loaded plugins list
    g_loaded_plugins.append(pluginName);

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

    // Connect to output signals to forward logs from module_host to main process
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
                                     qWarning() << "[MODULE_HOST" << pluginName << "]:" << line;
                                 } else if (line.contains("Critical:") || line.contains("FAILED:") || line.contains("ERROR:")) {
                                     qCritical() << "[MODULE_HOST" << pluginName << "]:" << line;
                                 } else {
                                     qDebug() << "[MODULE_HOST" << pluginName << "]:" << line;
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
                                 qCritical() << "[MODULE_HOST" << pluginName << "] STDERR:" << line;
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

// Helper function to initialize core manager
static bool initializeCoreManager()
{
    qDebug() << "\n=== Initializing Core Manager ===";
    
    // Create the core manager instance directly
    CoreManagerPlugin* coreManager = new CoreManagerPlugin();
    
    // Enable remote access for the core manager
    if (g_registry_host) {
        bool success = g_registry_host->enableRemoting(coreManager, coreManager->name());
        if (success) {
            qDebug() << "Core manager enabled for remote access with name:" << coreManager->name();
        } else {
            qWarning() << "Failed to enable remote access for core manager";
        }
    } else {
        qWarning() << "Registry host not initialized, cannot enable remote access for core manager";
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
        g_registry_host = new QRemoteObjectRegistryHost(QUrl(QStringLiteral("local:logos_core_registry")));
        qDebug() << "Qt Remote Object registry host initialized at: local:logos_core_registry";
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
    
    // Find and load all plugins in the directory
    QStringList pluginPaths = findPlugins(pluginsDir);
    
    if (pluginPaths.isEmpty()) {
        qWarning() << "No modules found in:" << pluginsDir;
    } else {
        qDebug() << "Found" << pluginPaths.size() << "modules";
        
        // Load and process each plugin
        for (const QString &pluginPath : pluginPaths) {
            // loadAndProcessPlugin(pluginPath);
            processPlugin(pluginPath);
        }
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
