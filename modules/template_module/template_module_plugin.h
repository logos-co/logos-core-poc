#pragma once

#include <QtCore/QObject>
#include "template_module_interface.h"

class TemplateModulePlugin : public QObject, public TemplateModuleInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID TemplateModuleInterface_iid FILE "metadata.json")
    Q_INTERFACES(TemplateModuleInterface PluginInterface)

public:
    TemplateModulePlugin();
    ~TemplateModulePlugin();

    Q_INVOKABLE bool foo(const QString &bar) override;
    QString name() const override { return "template_module"; }
    QString version() const override { return "1.0.0"; }
}; 