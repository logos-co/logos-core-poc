#pragma once

#include <QtCore/QObject>
#include "wallet_module_interface.h"
#include "../../logos-cpp-sdk/cpp/logos_api.h"
#include "../../logos-cpp-sdk/cpp/logos_api_client.h"

extern "C" {
    #include "lib/libgowalletsdk.h"
}

class WalletModulePlugin : public QObject, public WalletModuleInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID WalletModuleInterface_iid FILE "metadata.json")
    Q_INTERFACES(WalletModuleInterface PluginInterface)

public:
    WalletModulePlugin();
    ~WalletModulePlugin();

    Q_INVOKABLE bool initWallet(const QString &configJson) override;
    Q_INVOKABLE QString chainId(const QString &rpcUrl) override;
    Q_INVOKABLE QString getEthBalance(const QString &rpcUrl, const QString &address) override;
    Q_INVOKABLE QString getErc20Balances(const QString &rpcUrl, const QString &address, const QStringList &tokenAddresses) override;
    QString name() const override { return "wallet_module"; }
    QString version() const override { return "1.0.0"; }

    Q_INVOKABLE void initLogos(LogosAPI* logosAPIInstance);

signals:
    void eventResponse(const QString& eventName, const QVariantList& data);

private:
    unsigned long long walletHandle;

    static void simple_callback(int callerRet, const char* msg, size_t len, void* userData);
};


