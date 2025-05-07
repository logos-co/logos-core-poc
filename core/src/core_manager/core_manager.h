#ifndef CORE_MANAGER_PLUGIN_H
#define CORE_MANAGER_PLUGIN_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QJsonArray>
#include "../interface.h"

class CoreManagerPlugin : public QObject, public PluginInterface {
    Q_OBJECT
    Q_PLUGIN_METADATA(IID PluginInterface_iid FILE "metadata.json")
    Q_INTERFACES(PluginInterface)

public:
    CoreManagerPlugin();
    ~CoreManagerPlugin();

    // PluginInterface implementation
    QString name() const override { return "core_manager"; }
    QString version() const override { return "0.1.0"; }

    // Core manager functionality
    Q_INVOKABLE void initialize(int argc, char* argv[]);
    Q_INVOKABLE void setPluginsDirectory(const QString& directory);
    Q_INVOKABLE void start();
    Q_INVOKABLE void cleanup();
    Q_INVOKABLE QStringList getLoadedPlugins();
    Q_INVOKABLE QJsonArray getKnownPlugins();
    Q_INVOKABLE QJsonArray getPluginMethods(const QString& pluginName);
    Q_INVOKABLE void helloWorld();
    Q_INVOKABLE bool loadPlugin(const QString& pluginName);
    Q_INVOKABLE bool unloadPlugin(const QString& pluginName);
    Q_INVOKABLE QString processPlugin(const QString& filePath);

private:
    QString m_pluginsDirectory;
    QStringList m_loadedPlugins;
};

#endif // CORE_MANAGER_PLUGIN_H 