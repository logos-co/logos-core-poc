#pragma once

#include <QtCore/QObject>
#include <QtCore/QString>
#include "../../logos-liblogos/interface.h"

class CapabilityModuleInterface : public PluginInterface
{
public:
    virtual ~CapabilityModuleInterface() {}
    Q_INVOKABLE virtual QString requestModule(const QString &fromModuleName, const QString &moduleName) = 0;

signals:
    // for now this is required for events, later it might not be necessary if using a proxy
    void eventResponse(const QString& eventName, const QVariantList& data);
};

#define CapabilityModuleInterface_iid "org.logos.CapabilityModuleInterface"
Q_DECLARE_INTERFACE(CapabilityModuleInterface, CapabilityModuleInterface_iid) 