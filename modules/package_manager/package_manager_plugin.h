#pragma once

#include <QtCore/QObject>
#include <QJsonArray>
#include "package_manager_interface.h"

class PackageManagerPlugin : public QObject, public PackageManagerInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID PackageManagerInterface_iid FILE "metadata.json")
    Q_INTERFACES(PackageManagerInterface PluginInterface)

public:
    PackageManagerPlugin();
    ~PackageManagerPlugin();

    // Implementation of PackageManagerInterface
    Q_INVOKABLE bool installPlugin(const QString& pluginPath) override;
    QString name() const override { return "package_manager"; }
    QString version() const override { return "1.0.0"; }
    Q_INVOKABLE QJsonArray getPackages();

private:
    QString m_pluginsDirectory;
}; 