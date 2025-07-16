#pragma once

#include <QtCore/QObject>
#include <QtCore/QJsonArray>
#include <QtCore/QStringList>
#include "template_module_interface.h"
#include "../../SDK/cpp/logos_api_client.h"

class TemplateModulePlugin : public QObject, public TemplateModuleInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID TemplateModuleInterface_iid FILE "metadata.json")
    Q_INTERFACES(TemplateModuleInterface PluginInterface)

public:
    TemplateModulePlugin();
    ~TemplateModulePlugin();

    Q_INVOKABLE bool foo(const QString &bar) override;
    Q_INVOKABLE void bar(const QString &message) override;
    Q_INVOKABLE bool stringToBool(const QString &boolString) override;
    Q_INVOKABLE QJsonArray getJsonArray(const QString &arrayType) override;
    Q_INVOKABLE QString combineStrings(const QString &str1, const QString &str2) override;
    Q_INVOKABLE QString formatMessage(const QString &prefix, const QString &message, const QString &suffix) override;
    Q_INVOKABLE QStringList getStringList(const QString &listType) override;
    Q_INVOKABLE QString processData(const QString &title, int value, const QString &unit) override;
    QString name() const override { return "template_module"; }
    QString version() const override { return "1.0.0"; }

signals:
    // for now this is required for events, later it might not be necessary if using a proxy
    void eventResponse(const QString& eventName, const QVariantList& data);

private:
    LogosAPIClient* logosAPI;
}; 