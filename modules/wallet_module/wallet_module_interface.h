#pragma once

#include <QtCore/QObject>
#include "../../logos-liblogos/interface.h"

class WalletModuleInterface : public PluginInterface
{
public:
    virtual ~WalletModuleInterface() {}

    // Example operations; will be expanded as we integrate SDK
    Q_INVOKABLE virtual bool initWallet(const QString &configJson) = 0;
    Q_INVOKABLE virtual QString chainId(const QString &rpcUrl) = 0;
    Q_INVOKABLE virtual QString getEthBalance(const QString &rpcUrl, const QString &address) = 0;
    Q_INVOKABLE virtual QString getErc20Balances(const QString &rpcUrl, const QString &address, const QStringList &tokenAddresses) = 0;

signals:
    void eventResponse(const QString& eventName, const QVariantList& data);
};

#define WalletModuleInterface_iid "org.logos.WalletModuleInterface"
Q_DECLARE_INTERFACE(WalletModuleInterface, WalletModuleInterface_iid)


