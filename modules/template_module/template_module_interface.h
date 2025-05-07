#pragma once

#include "../../core/interface.h"

class TemplateModuleInterface : public PluginInterface
{
public:
    virtual ~TemplateModuleInterface() {}
    Q_INVOKABLE virtual bool foo(const QString &bar) = 0;
};

#define TemplateModuleInterface_iid "org.logos.TemplateModuleInterface"
Q_DECLARE_INTERFACE(TemplateModuleInterface, TemplateModuleInterface_iid) 