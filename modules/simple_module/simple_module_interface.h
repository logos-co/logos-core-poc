#pragma once

#include <QtCore/QObject>
#include <QtCore/QJsonArray>
#include <QtCore/QStringList>
#include "../../core/interface.h"

class SimpleModuleInterface : public PluginInterface
{
public:
    virtual ~SimpleModuleInterface() {}
    Q_INVOKABLE virtual bool foo(const QString &bar) = 0;
    Q_INVOKABLE virtual void bar(const QString &message) = 0;
    Q_INVOKABLE virtual bool stringToBool(const QString &boolString) = 0;
    Q_INVOKABLE virtual QJsonArray getJsonArray(const QString &arrayType) = 0;
    Q_INVOKABLE virtual QString combineStrings(const QString &str1, const QString &str2) = 0;
    Q_INVOKABLE virtual QString formatMessage(const QString &prefix, const QString &message, const QString &suffix) = 0;
    Q_INVOKABLE virtual QStringList getStringList(const QString &listType) = 0;
    Q_INVOKABLE virtual QString processData(const QString &title, int value, const QString &unit) = 0;

signals:
    // for now this is required for events, later it might not be necessary if using a proxy
    void eventResponse(const QString& eventName, const QVariantList& data);
};

#define SimpleModuleInterface_iid "org.logos.SimpleModuleInterface"
Q_DECLARE_INTERFACE(SimpleModuleInterface, SimpleModuleInterface_iid)


