#include "core_manager.h"
#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QPluginLoader>
#include <QMetaMethod>
#include <QJsonObject>
#include <QJsonArray>
#include "../plugin_registry.h"
#include "logos_core.h"

CoreManagerPlugin::CoreManagerPlugin() {
    qDebug() << "CoreManager plugin created";
}

CoreManagerPlugin::~CoreManagerPlugin() {
    cleanup();
}

void CoreManagerPlugin::initialize(int argc, char* argv[]) {
    qDebug() << "Initializing CoreManager plugin";
    // We don't call logos_core_init here because it creates another QApplication
    // instance and we're already creating one in main.cpp
}

void CoreManagerPlugin::setPluginsDirectory(const QString& directory) {
    m_pluginsDirectory = directory;
    qDebug() << "Setting plugins directory to:" << directory;
    logos_core_set_plugins_dir(directory.toUtf8().constData());
}

void CoreManagerPlugin::start() {
    qDebug() << "Starting CoreManager plugin";
    logos_core_start();

    // Register ourselves in the plugin registry
    PluginRegistry::registerPlugin(this, name());
}

void CoreManagerPlugin::cleanup() {
    qDebug() << "Cleaning up CoreManager plugin";
    logos_core_cleanup();
}

void CoreManagerPlugin::helloWorld() {
    qDebug() << "Hello from CoreManager plugin!";
}

QStringList CoreManagerPlugin::getLoadedPlugins() {
    qDebug() << "\n\n----------> Getting loaded plugins\n\n";
    QStringList pluginList;
    char** plugins = logos_core_get_loaded_plugins();
    
    if (!plugins) {
        return pluginList;
    }
    
    for (char** p = plugins; *p != nullptr; ++p) {
        pluginList << QString::fromUtf8(*p);
        delete[] *p;
    }
    delete[] plugins;
    
    return pluginList;
}

QJsonArray CoreManagerPlugin::getPluginMethods(const QString& pluginName) {
    QJsonArray methodsArray;
    
    // Get the plugin from the registry
    QObject* plugin = PluginRegistry::getPlugin<QObject>(pluginName);
    if (!plugin) {
        qWarning() << "Plugin not found:" << pluginName;
        return methodsArray;
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
    
    return methodsArray;
}
