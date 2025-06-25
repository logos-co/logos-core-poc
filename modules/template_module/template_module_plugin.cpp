#include "template_module_plugin.h"
#include <QDebug>
#include <QCoreApplication>
#include <QVariantList>
#include <QDateTime>
#include <QJsonArray>
#include <QJsonObject>

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

void TemplateModulePlugin::bar(const QString &message)
{
    qDebug() << "TemplateModulePlugin::bar called with message:" << message;
    qDebug() << "Bar method executing - processing message:" << message;
    qDebug() << "Message length:" << message.length() << "characters";
    qDebug() << "Bar method completed successfully";
}

bool TemplateModulePlugin::stringToBool(const QString &boolString)
{
    qDebug() << "TemplateModulePlugin::stringToBool called with string:" << boolString;
    
    // Convert to lowercase for case-insensitive comparison
    QString lowerString = boolString.toLower().trimmed();
    
    bool result = false;
    if (lowerString == "true") {
        result = true;
        qDebug() << "String 'true' converted to boolean: true";
    } else if (lowerString == "false") {
        result = false;
        qDebug() << "String 'false' converted to boolean: false";
    } else {
        qDebug() << "Invalid boolean string:" << boolString << "- defaulting to false";
        result = false;
    }
    
    qDebug() << "stringToBool returning:" << result;
    return result;
}

QJsonArray TemplateModulePlugin::getJsonArray(const QString &arrayType)
{
    qDebug() << "TemplateModulePlugin::getJsonArray called with arrayType:" << arrayType;
    
    QJsonArray jsonArray;
    
    if (arrayType.toLower() == "numbers") {
        // Return an array of numbers
        jsonArray.append(1);
        jsonArray.append(2);
        jsonArray.append(3);
        jsonArray.append(42);
        jsonArray.append(100);
        qDebug() << "Created numbers array with 5 elements";
        
    } else if (arrayType.toLower() == "strings") {
        // Return an array of strings
        jsonArray.append("hello");
        jsonArray.append("world");
        jsonArray.append("json");
        jsonArray.append("array");
        qDebug() << "Created strings array with 4 elements";
        
    } else if (arrayType.toLower() == "mixed") {
        // Return a mixed array
        jsonArray.append("text");
        jsonArray.append(123);
        jsonArray.append(true);
        jsonArray.append("more text");
        jsonArray.append(45.67);
        qDebug() << "Created mixed array with 5 elements";
        
    } else if (arrayType.toLower() == "objects") {
        // Return an array of JSON objects
        QJsonObject obj1;
        obj1["name"] = "Alice";
        obj1["age"] = 30;
        jsonArray.append(obj1);
        
        QJsonObject obj2;
        obj2["name"] = "Bob";
        obj2["age"] = 25;
        jsonArray.append(obj2);
        
        QJsonObject obj3;
        obj3["name"] = "Charlie";
        obj3["age"] = 35;
        jsonArray.append(obj3);
        
        qDebug() << "Created objects array with 3 person objects";
        
    } else {
        // Default: return empty array
        qDebug() << "Unknown arrayType:" << arrayType << "- returning empty array";
    }
    
    qDebug() << "getJsonArray returning array with" << jsonArray.size() << "elements";
    return jsonArray;
} 
