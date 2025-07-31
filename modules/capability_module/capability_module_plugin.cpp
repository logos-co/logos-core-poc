#include "capability_module_plugin.h"
#include <QDebug>
#include <QCoreApplication>
#include <QVariantList>
#include <QDateTime>
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
    // get the token for the module
    QString token = logosAPI->getTokenManager()->getToken(fromModuleName);
    qDebug() << "CapabilityModulePlugin::requestModule token:" << token;

    // check if the token is valid
    if (token.isEmpty()) {
        qDebug() << "CapabilityModulePlugin::requestModule token is empty";

        return "error";
    } else {
        qDebug() << "CapabilityModulePlugin::requestModule token is valid";
        qDebug() << "Token is:" << token;
        return token;
    }
}

void CapabilityModulePlugin::initLogos(LogosAPI* logosAPIInstance) {
    if (logosAPI) {
        delete logosAPI;
    }
    logosAPI = logosAPIInstance;
} 