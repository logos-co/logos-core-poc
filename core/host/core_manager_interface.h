#ifndef CORE_MANAGER_INTERFACE_H
#define CORE_MANAGER_INTERFACE_H

#include <QString>
#include <QStringList>
#include "../interface.h"

// Core manager specific methods interface
class CoreManagerInterface {
public:
    virtual ~CoreManagerInterface() {}

    virtual void initialize(int argc, char* argv[]) = 0;
    virtual void setPluginsDirectory(const QString& directory) = 0;
    virtual void start() = 0;
    virtual void cleanup() = 0;
    virtual QStringList getLoadedPlugins() = 0;
    virtual bool loadPlugin(const QString& pluginName) = 0;
};

#endif // CORE_MANAGER_INTERFACE_H 