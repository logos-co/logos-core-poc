#include "template_module_plugin.h"
#include <QDebug>
#include <QCoreApplication>
#include <QVariantList>
#include <QDateTime>

TemplateModulePlugin::TemplateModulePlugin() : logosAPI(nullptr)
{
    qDebug() << "TemplateModulePlugin: Initializing...";
    
    // Initialize the Logos API
    logosAPI = new LogosAPI("core_registry", this);
    
    qDebug() << "TemplateModulePlugin: Initialized successfully";
}

TemplateModulePlugin::~TemplateModulePlugin() 
{
    // Clean up resources
    if (logosAPI) {
        delete logosAPI;
        logosAPI = nullptr;
    }
}

bool TemplateModulePlugin::foo(const QString &bar)
{
    qDebug() << "TemplateModulePlugin::foo called with:" << bar;
    
    // Create event data with the bar parameter
    QVariantList eventData;
    eventData << bar; // Add the bar parameter to the event data
    eventData << QDateTime::currentDateTime().toString(Qt::ISODate); // Add timestamp
    
    // Trigger the event using LogosAPI (like chat module does)
    if (logosAPI) {
        // print triggering signal
        qDebug() << "TemplateModulePlugin: Triggering event 'fooTriggered' with data:" << eventData;
        logosAPI->onEventResponse(this, "fooTriggered", eventData);
        qDebug() << "TemplateModulePlugin: Event 'fooTriggered' triggered with data:" << eventData;
    } else {
        qWarning() << "TemplateModulePlugin: LogosAPI not available, cannot trigger event";
    }
    
    return true;
} 