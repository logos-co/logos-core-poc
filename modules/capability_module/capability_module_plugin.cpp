#include "capability_module_plugin.h"
#include <QDebug>
#include <QCoreApplication>
#include <QVariantList>
#include <QDateTime>
#include <QJsonArray>
#include <QJsonObject>

CapabilityModulePlugin::CapabilityModulePlugin() : logosAPI(nullptr)
{
    qDebug() << "CapabilityModulePlugin: Initializing...";
    
    // Initialize the LogosAPIProvider
    logosAPI = new LogosAPIProvider("core_registry", this);
    
    qDebug() << "CapabilityModulePlugin: Initialized successfully";
}

CapabilityModulePlugin::~CapabilityModulePlugin() 
{
    // Clean up resources
    if (logosAPI) {
        delete logosAPI;
        logosAPI = nullptr;
    }
}

QString CapabilityModulePlugin::requestModule(const QString &moduleName)
{
    qDebug() << "CapabilityModulePlugin::requestModule called with:" << moduleName;
    
    // Hardcoded return value as requested
    QString result = "abc";
    
    qDebug() << "CapabilityModulePlugin::requestModule returning:" << result;
    
    // Create event data with the module name parameter
    QVariantList eventData;
    eventData << moduleName; // Add the moduleName parameter to the event data
    eventData << result; // Add the result
    eventData << QDateTime::currentDateTime().toString(Qt::ISODate); // Add timestamp
    
    // Trigger the event using LogosAPI
    if (logosAPI) {
        qDebug() << "CapabilityModulePlugin: Triggering event 'requestModuleTriggered' with data:" << eventData;
        // logosAPI->onEventResponse(this, "requestModuleTriggered", eventData);
        qDebug() << "CapabilityModulePlugin: Event 'requestModuleTriggered' triggered with data:" << eventData;
    } else {
        qWarning() << "CapabilityModulePlugin: LogosAPI not available, cannot trigger event";
    }
    
    return result;
}
