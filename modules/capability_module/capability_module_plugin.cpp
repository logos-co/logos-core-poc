#include "capability_module_plugin.h"
#include <QDebug>
#include <QCoreApplication>
#include <QVariantList>
#include <QDateTime>
#include <QUuid>
#include "token_manager.h"

CapabilityModulePlugin::CapabilityModulePlugin()
{
    qDebug() << "CapabilityModulePlugin: Initializing...";
    qDebug() << "CapabilityModulePlugin: Initialized successfully";
}

CapabilityModulePlugin::~CapabilityModulePlugin() 
{
    // Clean up resources
    if (logosAPI) {
        delete logosAPI;
        logosAPI = nullptr;
    }
}

QString CapabilityModulePlugin::requestModule(const QString &fromModuleName, const QString &moduleName)
{
    qDebug() << "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++";
    qDebug() << "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++";
    qDebug() << "CapabilityModulePlugin::requestModule called with fromModuleName:" << fromModuleName << "moduleName:" << moduleName;

    TokenManager* tokenManager = logosAPI->getTokenManager();
    QList<QString> keys = tokenManager->getTokenKeys();
    for (const QString& key : keys) {
        qDebug() << "CapabilityModulePlugin::requestModule token key:" << key << "value:" << tokenManager->getToken(key);
    }

    // actually here, need to:
    // 1. create a token

    QUuid authToken = QUuid::createUuid();
    QString authTokenString = authToken.toString(QUuid::WithoutBraces);

    // 2. call informModuleToken on the target moduleName

    // Get the capability module's own token to use as auth token
    QString moduleToken = logosAPI->getTokenManager()->getToken(moduleName);
    
    qDebug() << "CapabilityModulePlugin: Calling informModuleToken on target module:" << moduleName;
    
    // Call informModuleToken with the capability module auth token, requesting module name, and new token
    bool success = logosAPI->getClient(moduleName)->informModuleToken_module(moduleToken, moduleName, fromModuleName, authTokenString);
    if (success) {
        qDebug() << "CapabilityModulePlugin: Successfully informed" << moduleName << "about token for" << fromModuleName;
    } else {
        qWarning() << "CapabilityModulePlugin: Failed to inform" << moduleName << "about token for" << fromModuleName;
    }

    // 3. return that token

    qDebug() << "CapabilityModulePlugin::requestModule returning new auth token:" << authTokenString;
    return authTokenString;
}

void CapabilityModulePlugin::initLogos(LogosAPI* logosAPIInstance) {
    if (logosAPI) {
        delete logosAPI;
    }
    logosAPI = logosAPIInstance;
}
