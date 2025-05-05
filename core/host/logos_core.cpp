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
        qWarning() << "Failed to initialize core manager, continuing with other plugins...";
    }
    
    // Define the plugins directory path
    QString pluginsDir;
    if (!g_plugins_dir.isEmpty()) {
        // Use the custom plugins directory if set
        pluginsDir = g_plugins_dir;
    } else {
        // Use the default plugins directory
        pluginsDir = QDir::cleanPath(QCoreApplication::applicationDirPath() + "/../plugins");
    }
    qDebug() << "Looking for plugins in:" << pluginsDir;
    
    // Find and load all plugins in the directory
    QStringList pluginPaths = findPlugins(pluginsDir);
    
    if (pluginPaths.isEmpty()) {
        qWarning() << "No plugins found in:" << pluginsDir;
    } else {
        qDebug() << "Found" << pluginPaths.size() << "plugins";
        
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

// Implementation of the function to get a plugin's methods
char* logos_core_get_plugin_methods(const char* plugin_name)
{
    if (!plugin_name) {
        qWarning() << "Cannot get plugin methods: name is null";
        return nullptr;
    }

    QString name = QString::fromUtf8(plugin_name);
    qDebug() << "Getting methods for plugin:" << name;

    // Create a JSON array to store method information
    QJsonArray methodsArray;

    // Get the plugin from the registry (convert to registry key format)
    QString registryKey = name.toLower().replace(" ", "_");
    QObject* plugin = PluginRegistry::getPlugin<QObject>(registryKey);
    
    if (!plugin) {
        qWarning() << "Plugin not found in registry:" << name << "with key" << registryKey;
        // Return empty JSON array
        QJsonDocument emptyDoc(methodsArray);
        QByteArray jsonData = emptyDoc.toJson(QJsonDocument::Compact);
        char* result = new char[jsonData.size() + 1];
        strcpy(result, jsonData.constData());
        return result;
    }

    // Use QMetaObject for runtime introspection
    const QMetaObject* metaObject = plugin->metaObject();

    // Iterate through methods and add to the JSON array
    for (int i = 0; i < metaObject->methodCount(); ++i) {
        QMetaMethod method = metaObject->method(i);

        // Skip methods from QObject and other base classes
        if (method.enclosingMetaObject() != metaObject) {
            continue;
        }

        // Create a JSON object for each method
        QJsonObject methodObj;
        methodObj["signature"] = QString::fromUtf8(method.methodSignature());
        methodObj["name"] = QString::fromUtf8(method.name());
        methodObj["returnType"] = QString::fromUtf8(method.typeName());

        // Check if the method is invokable via QMetaObject::invokeMethod
        methodObj["isInvokable"] = method.isValid() && (method.methodType() == QMetaMethod::Method || 
                                  method.methodType() == QMetaMethod::Slot);

        // Add parameter information if available
        if (method.parameterCount() > 0) {
            QJsonArray params;
            for (int p = 0; p < method.parameterCount(); ++p) {
                QJsonObject paramObj;
                paramObj["type"] = QString::fromUtf8(method.parameterTypeName(p));

                // Try to get parameter name if available
                QByteArrayList paramNames = method.parameterNames();
                if (p < paramNames.size() && !paramNames.at(p).isEmpty()) {
                    paramObj["name"] = QString::fromUtf8(paramNames.at(p));
                } else {
                    paramObj["name"] = "param" + QString::number(p);
                }

                params.append(paramObj);
            }
            methodObj["parameters"] = params;
        }

        methodsArray.append(methodObj);
    }

    // Convert JSON array to string
    QJsonDocument doc(methodsArray);
    QByteArray jsonData = doc.toJson(QJsonDocument::Compact);
    
    // Convert to C string that must be freed by the caller
    char* result = new char[jsonData.size() + 1];
    strcpy(result, jsonData.constData());

    return result;
}

// Implementation of the function to call a plugin method with parameters
char* logos_core_call_plugin_method(const char* plugin_name, const char* method_name, const char* params_json, const char* return_type_hint)
{
    if (!plugin_name || !method_name) {
        qWarning() << "Cannot call plugin method: plugin_name or method_name is null";
        return nullptr;
    }

    QString name = QString::fromUtf8(plugin_name);
    QString methodName = QString::fromUtf8(method_name);
    QString returnTypeHint = return_type_hint ? QString::fromUtf8(return_type_hint) : QString();
    
    qDebug() << "Attempting to call method" << methodName << "on plugin:" << name;
    if (!returnTypeHint.isEmpty()) {
        qDebug() << "Using provided return type hint:" << returnTypeHint;
    }

    // Get the plugin from the registry (convert to registry key format)
    QString registryKey = name.toLower().replace(" ", "_");
    QObject* plugin = PluginRegistry::getPlugin<QObject>(registryKey);
    
    if (!plugin) {
        qWarning() << "Plugin not found in registry:" << name << "with key" << registryKey;
        return nullptr;
    }

    // Parse the JSON parameters
    QJsonParseError jsonError;
    QJsonDocument paramsDoc = QJsonDocument::fromJson(params_json ? QByteArray(params_json) : QByteArray("[]"), &jsonError);
    
    if (jsonError.error != QJsonParseError::NoError) {
        qWarning() << "Failed to parse parameters JSON:" << jsonError.errorString();
        return nullptr;
    }
    
    if (!paramsDoc.isArray()) {
        qWarning() << "Parameters JSON must be an array";
        return nullptr;
    }
    
    QJsonArray paramsArray = paramsDoc.array();
    
    // Get the meta-object for the plugin to find the method
    const QMetaObject* metaObj = plugin->metaObject();
    
    // Find the method in the meta-object system
    // We need to build a signature to find the method
    QString methodSignature = methodName + "(";
    QStringList paramTypes;
    
    for (int i = 0; i < paramsArray.size(); ++i) {
        QJsonObject paramObj = paramsArray[i].toObject();
        QString paramType = paramObj["type"].toString();
        paramTypes.append(paramType);
    }
    
    methodSignature += paramTypes.join(",") + ")";
    qDebug() << "Looking for method with signature:" << methodSignature;
    
    // Find the method by name and signature
    int methodIndex = -1;
    for (int i = 0; i < metaObj->methodCount(); ++i) {
        QMetaMethod method = metaObj->method(i);
        QString signature = QString::fromUtf8(method.methodSignature());
        qDebug() << "Checking method:" << signature;
        
        if (signature.startsWith(methodName + "(")) {
            // Found a method with the right name
            methodIndex = i;
            break;
        }
    }
    
    if (methodIndex == -1) {
        qWarning() << "Method not found:" << methodName;
        return nullptr;
    }
    
    QMetaMethod method = metaObj->method(methodIndex);
    
    // Validate parameter count
    if (method.parameterCount() != paramsArray.size()) {
        qWarning() << "Parameter count mismatch. Method expects" << method.parameterCount() 
                   << "but" << paramsArray.size() << "were provided";
        return nullptr;
    }
    
    // Create QGenericArgument array for the parameters
    QGenericArgument args[10] = { QGenericArgument() }; // Max 10 arguments
    
    // This vector holds our converted values so they stay in scope during the call
    QVector<QVariant> argValues;
    argValues.resize(paramsArray.size());
    
    // Track pointers that need to be freed
    QVector<void*> pointersToFree;
    
    // Process each parameter
    for (int i = 0; i < paramsArray.size() && i < 10; ++i) {
        QJsonObject paramObj = paramsArray[i].toObject();
        QString paramName = paramObj["name"].toString();
        QString paramType = paramObj["type"].toString();
        QJsonValue paramValue = paramObj["value"];
        
        qDebug() << "Processing parameter" << i << ":" << paramName << "of type" << paramType;
        
        // Convert JSON value to QVariant based on the type
        QVariant variant;
        
        if (paramType == "QString" || paramType == "std::string" || paramType == "string") {
            variant = paramValue.toString();
        } 
        else if (paramType == "int" || paramType == "qint32" || paramType == "int32_t") {
            // Make sure we explicitly convert to int and set the correct type
            variant = QVariant(paramValue.toInt());
            // Force the variant to be recognized as an int
            variant.convert(QMetaType::Int);
        }
        else if (paramType == "double" || paramType == "qreal" || paramType == "float") {
            variant = QVariant(paramValue.toDouble());
            variant.convert(QMetaType::Double);
        }
        else if (paramType == "bool") {
            variant = QVariant(paramValue.toBool());
            variant.convert(QMetaType::Bool);
        }
        else if (paramType == "QJsonObject" || paramType == "QVariantMap") {
            variant = paramValue.toObject().toVariantMap();
        }
        else if (paramType == "QJsonArray" || paramType == "QVariantList") {
            variant = paramValue.toArray().toVariantList();
        }
        else {
            qWarning() << "Unsupported parameter type:" << paramType;
            return nullptr;
        }
        
        qDebug() << "Parameter value type in QVariant:" << variant.typeName();
        
        // Store the converted value
        argValues[i] = variant;
        
        // Create the QGenericArgument
        if (paramType == "int") {
            // For int type, we need to make sure it's handled correctly
            int *intPtr = new int(argValues[i].toInt());
            pointersToFree.append(intPtr);
            args[i] = QGenericArgument("int", intPtr);
        } else if (paramType == "double" || paramType == "qreal" || paramType == "float") {
            double *doublePtr = new double(argValues[i].toDouble());
            pointersToFree.append(doublePtr);
            args[i] = QGenericArgument(paramType.toUtf8().constData(), doublePtr);
        } else if (paramType == "bool") {
            bool *boolPtr = new bool(argValues[i].toBool());
            pointersToFree.append(boolPtr);
            args[i] = QGenericArgument("bool", boolPtr);
        } else {
            // For other types, use the variant data directly
            args[i] = QGenericArgument(QMetaType::typeName(argValues[i].userType()), argValues[i].data());
        }
    }
    
    // Prepare return value storage based on the method's return type
    QVariant returnValue;
    QGenericReturnArgument returnArg;
    
    QString returnType = QString::fromUtf8(method.typeName());
    qDebug() << "Method's declared return type:" << returnType;
    
    // If a return type hint was provided, use it instead
    if (!returnTypeHint.isEmpty()) {
        qDebug() << "Using provided return type hint instead:" << returnTypeHint;
        returnType = returnTypeHint;
    }
    
    // Set up the return value based on type
    if (returnType == "void" || returnType.isEmpty()) {
        // No return value needed
        returnArg = QGenericReturnArgument();
    } 
    else if (returnType == "int") {
        int* result = new int(0);
        pointersToFree.append(result);
        returnArg = QGenericReturnArgument("int", result);
    }
    else if (returnType == "bool") {
        bool* result = new bool(false);
        pointersToFree.append(result);
        returnArg = QGenericReturnArgument("bool", result);
    }
    else if (returnType == "double" || returnType == "float" || returnType == "qreal") {
        double* result = new double(0.0);
        pointersToFree.append(result);
        returnArg = QGenericReturnArgument(returnType.toUtf8().constData(), result);
    }
    else if (returnType == "QString") {
        QString* result = new QString();
        pointersToFree.append(result);
        returnArg = QGenericReturnArgument("QString", result);
    }
    else {
        qWarning() << "Unsupported return type:" << returnType;
        
        // Free allocated memory before returning
        for (void* ptr : pointersToFree) {
            delete ptr;
        }
        
        return nullptr;
    }
    
    // Attempt to invoke the method
    bool success = false;
    
    if (returnType == "void" || returnType.isEmpty()) {
        success = QMetaObject::invokeMethod(
            plugin, 
            methodName.toUtf8().constData(),
            Qt::DirectConnection,
            args[0], args[1], args[2], args[3], args[4], 
            args[5], args[6], args[7], args[8], args[9]
        );
    } else {
        success = QMetaObject::invokeMethod(
            plugin, 
            methodName.toUtf8().constData(),
            Qt::DirectConnection,
            returnArg,
            args[0], args[1], args[2], args[3], args[4], 
            args[5], args[6], args[7], args[8], args[9]
        );
    }
    
    // Prepare result JSON
    QJsonObject resultJson;
    resultJson["success"] = success;
    
    if (success) {
        resultJson["message"] = "Method called successfully";
        
        // Extract return value if method returned something
        if (returnType != "void" && !returnType.isEmpty()) {
            if (returnType == "int") {
                resultJson["returnValue"] = *static_cast<int*>(returnArg.data());
            }
            else if (returnType == "bool") {
                resultJson["returnValue"] = *static_cast<bool*>(returnArg.data());
            }
            else if (returnType == "double" || returnType == "float" || returnType == "qreal") {
                resultJson["returnValue"] = *static_cast<double*>(returnArg.data());
            }
            else if (returnType == "QString") {
                resultJson["returnValue"] = *static_cast<QString*>(returnArg.data());
            }
        }
    } else {
        resultJson["message"] = "Failed to invoke method";
    }
    
    // Free allocated memory
    for (void* ptr : pointersToFree) {
        delete ptr;
    }
    
    // Convert result to JSON string
    QJsonDocument resultDoc(resultJson);
    QByteArray resultBytes = resultDoc.toJson(QJsonDocument::Compact);
    
    // Create a C string to return
    char* resultStr = new char[resultBytes.size() + 1];
    strcpy(resultStr, resultBytes.constData());
    
    if (success) {
        qDebug() << "Successfully called method:" << methodName << "on plugin:" << name;
    } else {
        qDebug() << "Failed to call method:" << methodName << "on plugin:" << name;
    }
    
    return resultStr;
} 
