#pragma once

#include <QtCore/QObject>
#include <QtCore/QJsonArray>
#include <QtCore/QStringList>
#include "capability_module_interface.h"
#include "../../SDK/cpp/logos_api_provider.h"

class CapabilityModulePlugin : public QObject, public CapabilityModuleInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID CapabilityModuleInterface_iid FILE "metadata.json")
    Q_INTERFACES(CapabilityModuleInterface PluginInterface)

public:
    CapabilityModulePlugin();
    ~CapabilityModulePlugin();

    Q_INVOKABLE QString requestModule(const QString &moduleName) override;
    QString name() const override { return "capability_module"; }
    QString version() const override { return "1.0.0"; }

signals:
    // for now this is required for events, later it might not be necessary if using a proxy
    void eventResponse(const QString& eventName, const QVariantList& data);

private:
    LogosAPIProvider* logosAPI;
}; 