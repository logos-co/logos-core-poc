#pragma once

#include <QtCore/QObject>
#include <QtCore/QString>
#include "capability_module_interface.h"
#include "../../logos-cpp-sdk/cpp/logos_api.h"
#include "../../logos-cpp-sdk/cpp/logos_api_client.h"

class CapabilityModulePlugin : public QObject, public CapabilityModuleInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID CapabilityModuleInterface_iid FILE "metadata.json")
    Q_INTERFACES(CapabilityModuleInterface PluginInterface)

public:
    CapabilityModulePlugin();
    ~CapabilityModulePlugin();

    Q_INVOKABLE QString requestModule(const QString &fromModuleName, const QString &moduleName) override;
    QString name() const override { return "capability_module"; }
    QString version() const override { return "1.0.0"; }

    // LogosAPI initialization
    Q_INVOKABLE void initLogos(LogosAPI* logosAPIInstance);

signals:
    // for now this is required for events, later it might not be necessary if using a proxy
    void eventResponse(const QString& eventName, const QVariantList& data);

// note: this should be defined in interface.h but if not defined here it breaks, unclear why at the moment
private:
   LogosAPI* logosAPI = nullptr;
}; 