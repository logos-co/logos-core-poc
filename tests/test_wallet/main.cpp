#include <iostream>
#include <QDir>
#include <QString>
#include <QStringList>
#include <QCoreApplication>
#include <QDebug>
#include <QJsonArray>
#include <QJsonObject>
#include "../../core/src/logos_core.h"
#include "../../SDK/cpp/logos_api.h"
#include "../../SDK/cpp/logos_api_client.h"
#include "logos_sdk.h"

int main(int argc, char *argv[])
{
    std::cout << "Test Wallet Application Starting..." << std::endl;

    // Initialize the logos core library
    logos_core_init(argc, argv);

    // Set the plugins directory to ./modules relative to the test binary
    QString pluginsDir = QDir::cleanPath(QDir::currentPath() + "/modules");
    std::cout << "Setting plugins directory to: " << pluginsDir.toStdString() << std::endl;
    logos_core_set_plugins_dir(pluginsDir.toUtf8().constData());

    // Start the logos core
    logos_core_start();

    // Load wallet_module plugin
    if (!logos_core_load_plugin("wallet_module")) {
        qCritical() << "Failed to load wallet_module plugin";
        return 1;
    }
    qDebug() << "wallet_module loaded";

    // Create a LogosAPI client and typed module wrappers
    LogosAPI api("core");
    LogosModules logos(&api);
    auto& coreManager = logos.core_manager;
    auto& wallet = logos.wallet_module;

    // Query the wallet plugin methods via core manager
    QJsonArray methods = coreManager.getPluginMethods("wallet_module");
    qDebug() << "wallet_module methods (" << methods.size() << ")";
    for (const QJsonValue &val : methods) {
        QJsonObject obj = val.toObject();
        qDebug() << "-" << obj.value("name").toString()
                 << "|" << obj.value("signature").toString()
                 << "| returns" << obj.value("returnType").toString();
        if (obj.contains("parameters")) {
            QStringList paramStrs;
            for (const QJsonValue &pv : obj.value("parameters").toArray()) {
                QJsonObject pObj = pv.toObject();
                paramStrs << (pObj.value("type").toString() + " " + pObj.value("name").toString());
            }
            if (!paramStrs.isEmpty()) {
                qDebug() << "  params:" << paramStrs.join(", ");
            }
        }
    }

    // Call chainId()
    const QString rpcUrl = QStringLiteral("https://ethereum-rpc.publicnode.com");
    QString chainId = wallet.chainId(rpcUrl);
    if (chainId.isEmpty()) {
        qCritical() << "chainId() call returned empty result";
        return 1;
    }
    qDebug() << "chainId:" << chainId;

    // Call getEthBalance() for zero address as in example
    QString balance = wallet.getEthBalance(rpcUrl, QStringLiteral("0x0000000000000000000000000000000000000000"));
    if (balance.isEmpty()) {
        qCritical() << "getEthBalance() call returned empty result";
        return 1;
    }
    qDebug() << "balance (wei):" << balance;

    std::cout << "Test Wallet Completed" << std::endl;
    return 0;
}
