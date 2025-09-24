#include "simple_module_plugin.h"
#include <QDebug>
#include <QCoreApplication>
#include <QVariantList>
#include <QDateTime>
#include <QJsonArray>
#include <QJsonObject>
#include "../../SDK/cpp/token_manager.h"

SimpleModulePlugin::SimpleModulePlugin()
{
    qDebug() << "SimpleModulePlugin: Initializing...";
    qDebug() << "SimpleModulePlugin: Initialized successfully";
}

SimpleModulePlugin::~SimpleModulePlugin() 
{
    // Clean up resources
    if (logosAPI) {
        delete logosAPI;
        logosAPI = nullptr;
    }
}

bool SimpleModulePlugin::foo(const QString &bar)
{
    qDebug() << "SimpleModulePlugin::foo called with:" << bar;
    
    // Create event data with the bar parameter
    QVariantList eventData;
    eventData << bar; // Add the bar parameter to the event data
    eventData << QDateTime::currentDateTime().toString(Qt::ISODate); // Add timestamp
    
    // This is here for now for testing purposes
    // get token manager from logos api and print the keys
    TokenManager* tokenManager = logosAPI->getTokenManager();
    if (tokenManager) {
        qDebug() << "--------------------------------------------------------";
        qDebug() << "SimpleModulePlugin: Token manager keys:";
        // print the keys and values
        QList<QString> keys = tokenManager->getTokenKeys();
        for (const QString& key : keys) {
            qDebug() << "SimpleModulePlugin: Token key:" << key << "value:" << tokenManager->getToken(key);
        }
        qDebug() << "--------------------------------------------------------";
    } else {
        qWarning() << "SimpleModulePlugin: Token manager not available";
    }
    
    // Trigger the event using LogosAPI client (like chat module does)
    if (logosAPI) {
        // print triggering signal
        qDebug() << "SimpleModulePlugin: Triggering event 'fooTriggered' with data:" << eventData;
        logosAPI->getClient("core_manager")->onEventResponse(this, "fooTriggered", eventData);
        qDebug() << "SimpleModulePlugin: Event 'fooTriggered' triggered with data:" << eventData;
    } else {
        qWarning() << "SimpleModulePlugin: LogosAPI not available, cannot trigger event";
    }
    
    return true;
}

void SimpleModulePlugin::bar(const QString &message)
{
    qDebug() << "SimpleModulePlugin::bar called with message:" << message;
    
    // Create event data with the message parameter
    QVariantList eventData;
    eventData << message;
    eventData << QDateTime::currentDateTime().toString(Qt::ISODate);
    
    // Trigger the event using LogosAPI client
    if (logosAPI) {
        qDebug() << "SimpleModulePlugin: Triggering event 'barTriggered' with data:" << eventData;
        logosAPI->getClient("core_manager")->onEventResponse(this, "barTriggered", eventData);
        qDebug() << "SimpleModulePlugin: Event 'barTriggered' triggered with data:" << eventData;
    } else {
        qWarning() << "SimpleModulePlugin: LogosAPI not available, cannot trigger event";
    }
}

bool SimpleModulePlugin::stringToBool(const QString &boolString)
{
    qDebug() << "SimpleModulePlugin::stringToBool called with:" << boolString;
    
    QString lowerStr = boolString.toLower().trimmed();
    bool result = (lowerStr == "true" || lowerStr == "1" || lowerStr == "yes" || lowerStr == "on");
    
    qDebug() << "SimpleModulePlugin::stringToBool result:" << result;
    
    // Trigger event with the conversion result
    if (logosAPI) {
        QVariantList eventData;
        eventData << boolString << result;
        logosAPI->getClient("core_manager")->onEventResponse(this, "stringToBoolTriggered", eventData);
    }
    
    return result;
}

QJsonArray SimpleModulePlugin::getJsonArray(const QString &arrayType)
{
    qDebug() << "SimpleModulePlugin::getJsonArray called with arrayType:" << arrayType;
    
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
        qWarning() << "SimpleModulePlugin::getJsonArray: Unknown array type:" << arrayType;
    }
    
    qDebug() << "SimpleModulePlugin::getJsonArray result:" << result;
    
    // Trigger event with the array result
    if (logosAPI) {
        QVariantList eventData;
        eventData << arrayType << QVariant::fromValue(result);
        logosAPI->getClient("core_manager")->onEventResponse(this, "getJsonArrayTriggered", eventData);
    }
    
    return result;
}

QString SimpleModulePlugin::combineStrings(const QString &str1, const QString &str2)
{
    qDebug() << "SimpleModulePlugin::combineStrings called with str1:" << str1 << "str2:" << str2;
    
    QString result = str1 + " + " + str2;
    
    qDebug() << "SimpleModulePlugin::combineStrings result:" << result;
    
    // Trigger event with the combination result
    if (logosAPI) {
        QVariantList eventData;
        eventData << str1 << str2 << result;
        logosAPI->getClient("core_manager")->onEventResponse(this, "combineStringsTriggered", eventData);
    }
    
    return result;
}

QString SimpleModulePlugin::formatMessage(const QString &prefix, const QString &message, const QString &suffix)
{
    qDebug() << "SimpleModulePlugin::formatMessage called with prefix:" << prefix << "message:" << message << "suffix:" << suffix;
    
    QString result = QString("[%1] %2 [%3]").arg(prefix).arg(message).arg(suffix);
    
    qDebug() << "SimpleModulePlugin::formatMessage result:" << result;
    
    // Trigger event with the formatting result
    if (logosAPI) {
        QVariantList eventData;
        eventData << prefix << message << suffix << result;
        logosAPI->getClient("core_manager")->onEventResponse(this, "formatMessageTriggered", eventData);
    }
    
    return result;
}

QStringList SimpleModulePlugin::getStringList(const QString &listType)
{
    qDebug() << "SimpleModulePlugin::getStringList called with listType:" << listType;
    
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
        qWarning() << "SimpleModulePlugin::getStringList: Unknown list type:" << listType;
    }
    
    qDebug() << "SimpleModulePlugin::getStringList result:" << result;
    
    // Trigger event with the list result
    if (logosAPI) {
        QVariantList eventData;
        eventData << listType << QVariant::fromValue(result);
        logosAPI->getClient("core_manager")->onEventResponse(this, "getStringListTriggered", eventData);
    }
    
    return result;
}

QString SimpleModulePlugin::processData(const QString &title, int value, const QString &unit)
{
    qDebug() << "SimpleModulePlugin::processData called with title:" << title << "value:" << value << "unit:" << unit;
    
    QString result = QString("%1: %2 %3").arg(title).arg(value).arg(unit);
    
    qDebug() << "SimpleModulePlugin::processData result:" << result;
    
    // Trigger event with the processing result
    if (logosAPI) {
        QVariantList eventData;
        eventData << title << value << unit << result;
        logosAPI->getClient("core_manager")->onEventResponse(this, "processDataTriggered", eventData);
    }
    
    return result;
}

void SimpleModulePlugin::initLogos(LogosAPI* logosAPIInstance) {
    logosAPI = logosAPIInstance;
} 


