#pragma once

#include "../../logos-liblogos/interface.h"

class PackageManagerInterface : public PluginInterface
{
public:
    virtual ~PackageManagerInterface() {}
    // Expose only the InstallPlugin API
    Q_INVOKABLE virtual bool installPlugin(const QString &pluginPath) = 0;
};

#define PackageManagerInterface_iid "org.logos.PackageManagerInterface"
Q_DECLARE_INTERFACE(PackageManagerInterface, PackageManagerInterface_iid) 