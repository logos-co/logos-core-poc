#include "template_module_plugin.h"
#include <QDebug>
#include <QCoreApplication>
#include "../../core/plugin_registry.h"

TemplateModulePlugin::TemplateModulePlugin()
{
    qDebug() << "hello world";
}

TemplateModulePlugin::~TemplateModulePlugin() {}

bool TemplateModulePlugin::foo(const QString &bar)
{
    qDebug() << "foo called with:" << bar;
    return true;
} 