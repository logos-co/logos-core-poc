#include "template_module_plugin.h"
#include <QDebug>
#include <QCoreApplication>
#include <QVariantList>
#include <QDateTime>
#include <QJsonArray>
#include <QJsonObject>
#include "../../SDK/cpp/token_manager.h"

TemplateModulePlugin::TemplateModulePlugin()
{
    qDebug() << "TemplateModulePlugin: Initializing...";
    qDebug() << "TemplateModulePlugin: Initialized successfully";
}

TemplateModulePlugin::~TemplateModulePlugin() 
{
    // Clean up resources
    //if (logosAPI) {
    //    delete logosAPI;
    //    logosAPI = nullptr;
    //}
}

bool TemplateModulePlugin::foo(const QString &bar)
{
    qDebug() << "TemplateModulePlugin::foo called with:" << bar;
    
    // Create event data with the bar parameter
    QVariantList eventData;
    eventData << bar; // Add the bar parameter to the event data
    eventData << QDateTime::currentDateTime().toString(Qt::ISODate); // Add timestamp
    
    // This is here for now for testing purposes
    // get token manager from logos api and print the keys
    TokenManager* tokenManager = logosAPI->getTokenManager();
    if (tokenManager) {
        qDebug() << "--------------------------------------------------------";
        qDebug() << "TemplateModulePlugin: Token manager keys:";
        // print the keys and values
        QList<QString> keys = tokenManager->getTokenKeys();
        for (const QString& key : keys) {
            qDebug() << "TemplateModulePlugin: Token key:" << key << "value:" << tokenManager->getToken(key);
        }
        qDebug() << "--------------------------------------------------------";
    } else {
        qWarning() << "TemplateModulePlugin: Token manager not available";
    }
    
    // Trigger the event using LogosAPI client (like chat module does)
    if (logosAPI) {
        // print triggering signal
        qDebug() << "TemplateModulePlugin: Triggering event 'fooTriggered' with data:" << eventData;
        logosAPI->getClient("core_manager")->onEventResponse(this, "fooTriggered", eventData);
        qDebug() << "TemplateModulePlugin: Event 'fooTriggered' triggered with data:" << eventData;
    } else {
        qWarning() << "TemplateModulePlugin: LogosAPI not available, cannot trigger event";
    }
    
    return true;
}

void TemplateModulePlugin::bar(const QString &message)
{
    qDebug() << "TemplateModulePlugin::bar called with message:" << message;
    
    // Create event data with the message parameter
    QVariantList eventData;
    eventData << message;
    eventData << QDateTime::currentDateTime().toString(Qt::ISODate);
    
    // Trigger the event using LogosAPI client
    if (logosAPI) {
        qDebug() << "TemplateModulePlugin: Triggering event 'barTriggered' with data:" << eventData;
        logosAPI->getClient("core_manager")->onEventResponse(this, "barTriggered", eventData);
        qDebug() << "TemplateModulePlugin: Event 'barTriggered' triggered with data:" << eventData;
    } else {
        qWarning() << "TemplateModulePlugin: LogosAPI not available, cannot trigger event";
    }
}

bool TemplateModulePlugin::stringToBool(const QString &boolString)
{
    qDebug() << "TemplateModulePlugin::stringToBool called with:" << boolString;
    
    QString lowerStr = boolString.toLower().trimmed();
    bool result = (lowerStr == "true" || lowerStr == "1" || lowerStr == "yes" || lowerStr == "on");
    
    qDebug() << "TemplateModulePlugin::stringToBool result:" << result;
    
    // Trigger event with the conversion result
    if (logosAPI) {
        QVariantList eventData;
        eventData << boolString << result;
        logosAPI->getClient("core_manager")->onEventResponse(this, "stringToBoolTriggered", eventData);
    }
    
    return result;
}

QJsonArray TemplateModulePlugin::getJsonArray(const QString &arrayType)
{
    qDebug() << "TemplateModulePlugin::getJsonArray called with arrayType:" << arrayType;
    
    QJsonArray result;
    
    if (arrayType.toLower() == "numbers") {
        result.append(1);
        result.append(2);
        result.append(3);
        result.append(4);
        result.append(5);
    } else if (arrayType.toLower() == "strings") {
        result.append("apple");
        result.append("banana");
        result.append("cherry");
        result.append("date");
    } else if (arrayType.toLower() == "mixed") {
        result.append("hello");
        result.append(42);
        result.append(true);
        result.append(3.14);
    } else if (arrayType.toLower() == "objects") {
        QJsonObject obj1;
        obj1["name"] = "John";
        obj1["age"] = 30;
        result.append(obj1);
        
        QJsonObject obj2;
        obj2["name"] = "Jane";
        obj2["age"] = 25;
        result.append(obj2);
    } else {
        // Default empty array for unknown types
        qWarning() << "TemplateModulePlugin::getJsonArray: Unknown array type:" << arrayType;
    }
    
    qDebug() << "TemplateModulePlugin::getJsonArray result:" << result;
    
    // Trigger event with the array result
    if (logosAPI) {
        QVariantList eventData;
        eventData << arrayType << QVariant::fromValue(result);
        logosAPI->getClient("core_manager")->onEventResponse(this, "getJsonArrayTriggered", eventData);
    }
    
    return result;
}

QString TemplateModulePlugin::combineStrings(const QString &str1, const QString &str2)
{
    qDebug() << "TemplateModulePlugin::combineStrings called with str1:" << str1 << "str2:" << str2;
    
    QString result = str1 + " + " + str2;
    
    qDebug() << "TemplateModulePlugin::combineStrings result:" << result;
    
    // Trigger event with the combination result
    if (logosAPI) {
        QVariantList eventData;
        eventData << str1 << str2 << result;
        logosAPI->getClient("core_manager")->onEventResponse(this, "combineStringsTriggered", eventData);
    }
    
    return result;
}

QString TemplateModulePlugin::formatMessage(const QString &prefix, const QString &message, const QString &suffix)
{
    qDebug() << "TemplateModulePlugin::formatMessage called with prefix:" << prefix << "message:" << message << "suffix:" << suffix;
    
    QString result = QString("[%1] %2 [%3]").arg(prefix).arg(message).arg(suffix);
    
    qDebug() << "TemplateModulePlugin::formatMessage result:" << result;
    
    // Trigger event with the formatting result
    if (logosAPI) {
        QVariantList eventData;
        eventData << prefix << message << suffix << result;
        logosAPI->getClient("core_manager")->onEventResponse(this, "formatMessageTriggered", eventData);
    }
    
    return result;
}

QStringList TemplateModulePlugin::getStringList(const QString &listType)
{
    qDebug() << "TemplateModulePlugin::getStringList called with listType:" << listType;
    
    QStringList result;
    
    if (listType.toLower() == "fruits") {
        result << "apple" << "banana" << "cherry" << "date" << "elderberry";
    } else if (listType.toLower() == "colors") {
        result << "red" << "green" << "blue" << "yellow";
    } else if (listType.toLower() == "countries") {
        result << "USA" << "Canada" << "Mexico" << "Brazil";
    } else if (listType.toLower() == "programming") {
        result << "C++" << "JavaScript" << "Python" << "Rust" << "Go";
    } else if (listType.toLower() == "numbers") {
        result << "one" << "two" << "three" << "four" << "five";
    } else {
        // Default empty list for unknown types
        qWarning() << "TemplateModulePlugin::getStringList: Unknown list type:" << listType;
    }
    
    qDebug() << "TemplateModulePlugin::getStringList result:" << result;
    
    // Trigger event with the list result
    if (logosAPI) {
        QVariantList eventData;
        eventData << listType << QVariant::fromValue(result);
        logosAPI->getClient("core_manager")->onEventResponse(this, "getStringListTriggered", eventData);
    }
    
    return result;
}

QString TemplateModulePlugin::processData(const QString &title, int value, const QString &unit)
{
    qDebug() << "TemplateModulePlugin::processData called with title:" << title << "value:" << value << "unit:" << unit;
    
    QString result = QString("%1: %2 %3").arg(title).arg(value).arg(unit);
    
    qDebug() << "TemplateModulePlugin::processData result:" << result;
    
    // Trigger event with the processing result
    if (logosAPI) {
        QVariantList eventData;
        eventData << title << value << unit << result;
        logosAPI->getClient("core_manager")->onEventResponse(this, "processDataTriggered", eventData);
    }
    
    return result;
}

void TemplateModulePlugin::initLogos(LogosAPI* logosAPIInstance) {
    logosAPI = logosAPIInstance;
} 
