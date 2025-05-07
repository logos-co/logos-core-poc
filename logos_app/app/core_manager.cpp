#include "core_manager.h"
#include <QCoreApplication>
#include <QDebug>

// Include logos_core header
#include "../../core/host/logos_core.h"

// External C functions from logos_core
extern "C" {
    void logos_core_set_plugins_dir(const char* plugins_dir);
    void logos_core_start();
    void logos_core_cleanup();
    char** logos_core_get_loaded_plugins();
    int logos_core_load_plugin(const char* plugin_name);
}

// Implementation of the custom deleter
void PluginsArrayDeleter::operator()(char** plugins) {
    if (!plugins) return;
    for (char** p = plugins; *p != nullptr; ++p) {
        delete[] *p;
    }
    delete[] plugins;
}

// Singleton instance accessor
CoreManager& CoreManager::instance() {
    static CoreManager instance;
    return instance;
}

void CoreManager::initialize(int argc, char* argv[]) {
    // We don't call logos_core_init here because it creates another QApplication
    // instance and we're already creating one in main.cpp
    qDebug() << "Core Manager initialized";
}

void CoreManager::setPluginsDirectory(const QString& directory) {
    qDebug() << "Setting plugins directory to:" << directory;
    logos_core_set_plugins_dir(directory.toUtf8().constData());
}

void CoreManager::start() {
    qDebug() << "Starting logos_core";
    logos_core_start();
}

void CoreManager::cleanup() {
    qDebug() << "Cleaning up logos_core";
    logos_core_cleanup();
}

QStringList CoreManager::getLoadedPlugins() {
    // Use smart pointer with custom deleter for automatic cleanup
    std::unique_ptr<char*, PluginsArrayDeleter> plugins(logos_core_get_loaded_plugins());
    return convertPluginsToStringList(plugins.get());
}

QStringList CoreManager::convertPluginsToStringList(char** plugins) {
    QStringList pluginList;
    
    if (!plugins) {
        return pluginList;
    }
    
    for (char** p = plugins; *p != nullptr; ++p) {
        pluginList << QString::fromUtf8(*p);
    }
    
    return pluginList;
}

bool CoreManager::loadPlugin(const QString& pluginName) {
    qDebug() << "CoreManager: Loading plugin:" << pluginName;
    int result = logos_core_load_plugin(pluginName.toUtf8().constData());
    return result == 1;
} 