#pragma once

#include <QtCore/QObject>
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
    Q_INVOKABLE bool installPlugin(const QString& pluginPath);
    QString name() const override { return "package_manager"; }
    QString version() const override { return "1.0.0"; }

private:
    QString m_pluginsDirectory;
}; 