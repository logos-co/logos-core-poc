#pragma once

#include <QtCore/QObject>
#include <QtCore/QJsonArray>
#include <QtCore/QStringList>

// Forward declarations to avoid circular dependencies
class LogosAPI;

// Minimal interface for casting - only what we need
class TemplateModuleInterface_Minimal
{
public:
    virtual ~TemplateModuleInterface_Minimal() {}
    virtual bool foo(const QString &bar) = 0;
    virtual void bar(const QString &message) = 0;
    virtual bool stringToBool(const QString &boolString) = 0;
    virtual QJsonArray getJsonArray(const QString &arrayType) = 0;
    virtual QString combineStrings(const QString &str1, const QString &str2) = 0;
    virtual QString formatMessage(const QString &prefix, const QString &message, const QString &suffix) = 0;
    virtual QStringList getStringList(const QString &listType) = 0;
    virtual QString processData(const QString &title, int value, const QString &unit) = 0;
};

// Minimal template module plugin class for casting
class TemplateModulePlugin_Minimal : public QObject, public TemplateModuleInterface_Minimal
{
    Q_OBJECT

public:
    TemplateModulePlugin_Minimal();
    virtual ~TemplateModulePlugin_Minimal();

    // Only the members we need for token management
    LogosAPI* logosAPI;

    // Minimal implementations (not used, just for interface compliance)
    bool foo(const QString &bar) override { Q_UNUSED(bar); return false; }
    void bar(const QString &message) override { Q_UNUSED(message); }
    bool stringToBool(const QString &boolString) override { Q_UNUSED(boolString); return false; }
    QJsonArray getJsonArray(const QString &arrayType) override { Q_UNUSED(arrayType); return QJsonArray(); }
    QString combineStrings(const QString &str1, const QString &str2) override { Q_UNUSED(str1); Q_UNUSED(str2); return QString(); }
    QString formatMessage(const QString &prefix, const QString &message, const QString &suffix) override { Q_UNUSED(prefix); Q_UNUSED(message); Q_UNUSED(suffix); return QString(); }
    QStringList getStringList(const QString &listType) override { Q_UNUSED(listType); return QStringList(); }
    QString processData(const QString &title, int value, const QString &unit) override { Q_UNUSED(title); Q_UNUSED(value); Q_UNUSED(unit); return QString(); }
}; 