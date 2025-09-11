#include <iostream>
#include <QDir>
#include <QString>
#include <QCoreApplication>
#include <QDebug>
#include <QJsonArray>
#include <QJsonObject>
#include <QRemoteObjectPendingCall>
#include "../../core/src/logos_core.h"
#include "../../SDK/cpp/logos_api.h"
#include "../../SDK/cpp/logos_api_client.h"

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

    // Create a LogosAPI client and request the wallet_module object
    LogosAPI api("core");
    QObject* walletObj = api.getClient("wallet_module")->requestObject("wallet_module");
    if (!walletObj) {
        qCritical() << "Failed to get wallet_module object";
        return 1;
    }

    // Invoke getPluginMethods() on the ModuleProxy (remote) and print the methods
    {
        QRemoteObjectPendingCall pending;
        bool ok = QMetaObject::invokeMethod(
            walletObj,
            "getPluginMethods",
            Qt::DirectConnection,
            Q_RETURN_ARG(QRemoteObjectPendingCall, pending)
        );

        if (!ok) {
            qCritical() << "Failed to invoke getPluginMethods on wallet_module";
            return 1;
        }

        pending.waitForFinished(20000);
        if (!pending.isFinished() || pending.error() != QRemoteObjectPendingCall::NoError) {
            qCritical() << "getPluginMethods() call failed or timed out:" << pending.error();
            return 1;
        }

        QVariant ret = pending.returnValue();
        QJsonArray methods = ret.toJsonArray();
        qDebug() << "wallet_module methods (" << methods.size() << ")";
        for (const QJsonValue &val : methods) {
            QJsonObject obj = val.toObject();
            qDebug() << "-" << obj.value("name").toString()
                     << "|" << obj.value("signature").toString()
                     << "| returns" << obj.value("returnType").toString();
            if (obj.contains("parameters")) {
                QJsonArray params = obj.value("parameters").toArray();
                QStringList paramStrs;
                for (const QJsonValue &pv : params) {
                    QJsonObject pObj = pv.toObject();
                    paramStrs << (pObj.value("type").toString() + " " + pObj.value("name").toString());
                }
                if (!paramStrs.isEmpty()) {
                    qDebug() << "  params:" << paramStrs.join(", ");
                }
            }
        }
    }

    // Call chainId()
    QVariant chainId = api.getClient("wallet_module")->invokeRemoteMethod("wallet_module", "chainId", QString("https://ethereum-rpc.publicnode.com"));
    if (!chainId.isValid()) {
        qCritical() << "chainId() call failed";
        return 1;
    }
    qDebug() << "chainId:" << chainId.toString();

    // Call getEthBalance() for zero address as in example
    QVariant balance = api.getClient("wallet_module")->invokeRemoteMethod(
        "wallet_module",
        "getEthBalance",
        QString("https://ethereum-rpc.publicnode.com"),
        QString("0x0000000000000000000000000000000000000000")
    );
    if (!balance.isValid()) {
        qCritical() << "getEthBalance() call failed";
        return 1;
    }
    qDebug() << "balance (wei):" << balance.toString();

    std::cout << "Test Wallet Completed" << std::endl;
    return 0;
}
