#pragma once

#include <QtCore/QObject>
#include <QtCore/QJsonArray>
#include <QtCore/QStringList>
#include "../../core/interface.h"

// Forward declaration to avoid circular dependency
class TokenManager;

class TemplateModuleInterface : public PluginInterface
{
public:
    virtual ~TemplateModuleInterface() {}
    Q_INVOKABLE virtual bool foo(const QString &bar) = 0;
    Q_INVOKABLE virtual void bar(const QString &message) = 0;
    Q_INVOKABLE virtual bool stringToBool(const QString &boolString) = 0;
    Q_INVOKABLE virtual QJsonArray getJsonArray(const QString &arrayType) = 0;
    Q_INVOKABLE virtual QString combineStrings(const QString &str1, const QString &str2) = 0;
    Q_INVOKABLE virtual QString formatMessage(const QString &prefix, const QString &message, const QString &suffix) = 0;
    Q_INVOKABLE virtual QStringList getStringList(const QString &listType) = 0;
    Q_INVOKABLE virtual QString processData(const QString &title, int value, const QString &unit) = 0;
    Q_INVOKABLE virtual void setTokenManager(TokenManager* tokenManager) = 0;

signals:
    // for now this is required for events, later it might not be necessary if using a proxy
    void eventResponse(const QString& eventName, const QVariantList& data);
};

#define TemplateModuleInterface_iid "org.logos.TemplateModuleInterface"
Q_DECLARE_INTERFACE(TemplateModuleInterface, TemplateModuleInterface_iid) 