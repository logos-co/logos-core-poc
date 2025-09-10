#include "wallet_module_plugin.h"
#include <QDebug>
#include <QVariantList>
#include <QDateTime>

WalletModulePlugin::WalletModulePlugin() : walletHandle(0)
{
    qDebug() << "WalletModulePlugin: Initializing...";
}

WalletModulePlugin::~WalletModulePlugin()
{
    if (logosAPI) {
        delete logosAPI;
        logosAPI = nullptr;
    }
    if (walletHandle != 0) {
        GoWSK_CloseClient(walletHandle);
        walletHandle = 0;
    }
}

void WalletModulePlugin::initLogos(LogosAPI* logosAPIInstance) {
    if (logosAPI) {
        delete logosAPI;
    }
    logosAPI = logosAPIInstance;
}

void WalletModulePlugin::simple_callback(int callerRet, const char* msg, size_t len, void* userData)
{
    Q_UNUSED(userData);
    qDebug() << "WalletModulePlugin::simple_callback ret:" << callerRet;
    if (msg && len > 0) {
        QString message = QString::fromUtf8(msg, static_cast<int>(len));
        qDebug() << "WalletModulePlugin::simple_callback message:" << message;
    }
}

bool WalletModulePlugin::initWallet(const QString &configJson)
{
    qDebug() << "WalletModulePlugin::initWallet called";
    Q_UNUSED(configJson);

    const char* url = "https://ethereum-rpc.publicnode.com";
    char* err = nullptr;
    walletHandle = GoWSK_NewClient((char*)url, &err);
    if (walletHandle == 0) {
        QString emsg = err ? QString::fromUtf8(err) : QString("unknown error");
        if (err) GoWSK_FreeCString(err);
        qWarning() << "WalletModulePlugin: Failed to create client:" << emsg;
        return false;
    }
    qDebug() << "WalletModulePlugin: Client created: handle=" << (qulonglong)walletHandle;
    return true;
}

QString WalletModulePlugin::chainId(const QString &rpcUrl)
{
    qDebug() << "WalletModulePlugin::chainId" << rpcUrl;
    Q_UNUSED(rpcUrl);
    if (walletHandle == 0) {
        if (!initWallet(QString())) {
            return QString();
        }
    }
    char* err = nullptr;
    char* chain = GoWSK_ChainID(walletHandle, &err);
    if (chain == nullptr) {
        QString emsg = err ? QString::fromUtf8(err) : QString("unknown error");
        if (err) GoWSK_FreeCString(err);
        qWarning() << "WalletModulePlugin: ChainID error:" << emsg;
        return QString();
    }
    QString result = QString::fromUtf8(chain);
    GoWSK_FreeCString(chain);
    return result;
}

QString WalletModulePlugin::getEthBalance(const QString &rpcUrl, const QString &address)
{
    qDebug() << "WalletModulePlugin::getEthBalance" << rpcUrl << address;
    Q_UNUSED(rpcUrl);
    if (walletHandle == 0) {
        if (!initWallet(QString())) {
            return QString();
        }
    }
    QByteArray addrUtf8 = address.toUtf8();
    char* err = nullptr;
    char* balance = GoWSK_GetBalance(walletHandle, addrUtf8.data(), &err);
    if (balance == nullptr) {
        QString emsg = err ? QString::fromUtf8(err) : QString("unknown error");
        if (err) GoWSK_FreeCString(err);
        qWarning() << "WalletModulePlugin: GetBalance error:" << emsg;
        return QString();
    }
    QString result = QString::fromUtf8(balance);
    GoWSK_FreeCString(balance);
    return result;
}

QString WalletModulePlugin::getErc20Balances(const QString &rpcUrl, const QString &address, const QStringList &tokenAddresses)
{
    qDebug() << "WalletModulePlugin::getErc20Balances" << rpcUrl << address << tokenAddresses;
    Q_UNUSED(rpcUrl);
    Q_UNUSED(address);
    Q_UNUSED(tokenAddresses);
    // Not yet implemented in cshared; stub for future
    return QString();
}


