#pragma once

#include <QtCore/QObject>
#include "../../core/interface.h"

class TemplateModuleInterface : public PluginInterface
{
public:
    virtual ~TemplateModuleInterface() {}
    Q_INVOKABLE virtual bool foo(const QString &bar) = 0;

signals:
    // for now this is required for events, later it might not be necessary if using a proxy
    void eventResponse(const QString& eventName, const QVariantList& data);
};

#define TemplateModuleInterface_iid "org.logos.TemplateModuleInterface"
Q_DECLARE_INTERFACE(TemplateModuleInterface, TemplateModuleInterface_iid) 