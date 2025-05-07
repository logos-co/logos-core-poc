#include "logos_core.h"
#include <QCoreApplication>
#include <QPluginLoader>
#include <QObject>
#include <QDebug>
#include <QDir>
#include <QMetaProperty>
#include <QMetaMethod>
#include <QTimer>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QHash>
#include "../interface.h"
#include "../plugin_registry.h"
#include "core_manager.h"

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

// Helper function to load a plugin by name
static bool loadPlugin(const QString &pluginName)
{
    if (!g_known_plugins.contains(pluginName)) {
        qWarning() << "Cannot load unknown plugin:" << pluginName;
        return false;
    }

    QString pluginPath = g_known_plugins.value(pluginName);
    qDebug() << "Loading plugin:" << pluginName << "from path:" << pluginPath;

    // Load the plugin
    QPluginLoader loader(pluginPath);
    QObject *plugin = loader.instance();

    if (!plugin) {
        qWarning() << "Failed to load plugin:" << loader.errorString();
        return false;
    }

    qDebug() << "Plugin loaded successfully.";

    // Cast to the base PluginInterface
    PluginInterface *basePlugin = qobject_cast<PluginInterface *>(plugin);
    qDebug() << "Plugin casted to PluginInterface";
    if (!basePlugin) {
        qWarning() << "Plugin does not implement the PluginInterface";
        return false;
    }

    // Verify that the plugin name matches the metadata
    if (pluginName != basePlugin->name()) {
        qWarning() << "Plugin name mismatch! Expected:" << pluginName << "Actual:" << basePlugin->name();
    }

    qDebug() << "Plugin name:" << basePlugin->name();
    qDebug() << "Plugin version:" << basePlugin->version();

    // Add the plugin name to our loaded plugins list
    g_loaded_plugins.append(basePlugin->name());

    // Register the plugin using the PluginRegistry namespace function
    PluginRegistry::registerPlugin(plugin, basePlugin->name());
    qDebug() << "Registered plugin with key:" << basePlugin->name().toLower().replace(" ", "_");

    // Use QObject reflection (QMetaObject) for runtime inspection
    const QMetaObject *metaObject = plugin->metaObject();
    qDebug() << "\nPlugin class name:" << metaObject->className();

    // List properties
    qDebug() << "\nProperties:";
    for (int i = 0; i < metaObject->propertyCount(); ++i) {
        QMetaProperty property = metaObject->property(i);
        qDebug() << " -" << property.name() << "=" << plugin->property(property.name());
    }

    // List methods
    qDebug() << "\nMethods:";
    for (int i = 0; i < metaObject->methodCount(); ++i) {
        QMetaMethod method = metaObject->method(i);
        qDebug() << " -" << method.methodSignature();
        
        // List parameter types for more complex methods
        if (method.parameterCount() > 0) {
            QStringList paramDetails;
            for (int p = 0; p < method.parameterCount(); ++p) {
                QString paramType = method.parameterTypeName(p);
                QString paramName = method.parameterNames().at(p);
                
                // Add extra info for known callback types
                if (paramType == "WakuInitCallback") {
                    paramDetails << QString("  - Parameter %1: %2 (std::function<void(bool success, const QString &message)>)").arg(p).arg(paramType);
                } else if (paramType == "WakuVersionCallback") {
                    paramDetails << QString("  - Parameter %1: %2 (std::function<void(const QString &version)>)").arg(p).arg(paramType);
                } else if (!paramType.isEmpty()) {
                    paramDetails << QString("  - Parameter %1: %2").arg(p).arg(paramType);
                }
            }
            
            if (!paramDetails.isEmpty()) {
                qDebug() << "   Parameters:";
                for (const QString &detail : paramDetails) {
                    qDebug() << detail;
                }
            }
        }
    }
    
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
    
    // Register it in the plugin registry
    PluginRegistry::registerPlugin(coreManager, coreManager->name());
    
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

    // Converting to registry key format 
    QString registryKey = name.toLower().replace(" ", "_");
    qDebug() << "Looking for plugin in registry with key:" << registryKey;

    // Get the plugin object from the registry
    QObject* plugin = nullptr;

    // First try to get it directly from registry
    plugin = PluginRegistry::getPlugin<QObject>(registryKey);

    if (plugin) {
        bool removed = PluginRegistry::unregisterPlugin(registryKey);

        g_loaded_plugins.removeAll(name);

        delete plugin;
        qDebug() << "Successfully deleted plugin object";
    }

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
