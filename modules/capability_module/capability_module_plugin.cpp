#include "capability_module_plugin.h"
#include <QDebug>
#include <QCoreApplication>
#include <QVariantList>
#include <QDateTime>
#include "token_manager.h"

CapabilityModulePlugin::CapabilityModulePlugin() : logosAPI(nullptr)
{
    qDebug() << "CapabilityModulePlugin: Initializing...";
    
    // Initialize the Logos API Client
    logosAPI = new LogosAPIClient("template_module", "capability_module", this);
    
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
    qDebug() << "CapabilityModulePlugin::requestModule called with fromModuleName:" << fromModuleName << "moduleName:" << moduleName;
    
    // For now, just return a hardcoded string as requested
    QString result = "abc";

    // // print tokens in token manager
    // qDebug() << "--------------------------------------------------------";
    // qDebug() << "--------------------------------------------------------";
    // TokenManager& tokenManager = TokenManager::instance();
    // qDebug() << "CapabilityModulePlugin: Tokens in TokenManager:";
    // QList<QString> tokenKeys = tokenManager.getTokenKeys();
    // for (const QString& key : tokenKeys) {
    //     QString value = tokenManager.getToken(key);
    //     qDebug() << "Token:" << key << "Value:" << value;
    // }
    // qDebug() << "--------------------------------------------------------";
    // qDebug() << "--------------------------------------------------------";

    qDebug() << "==============------------------------------------------";
    qDebug() << "-------------==============-----------------------------";
    qDebug() << "---------------------------============-----------------";

    // call template module foo method
    if (logosAPI && logosAPI->isConnected()) {
        qDebug() << "CapabilityModulePlugin: Calling template_module foo method";
        
        // Create a test parameter for the foo method
        QString testParam = QString("test_from_capability_module_%1").arg(QDateTime::currentDateTime().toString(Qt::ISODate));
        qDebug() << "CapabilityModulePlugin: Calling foo() with parameter:" << testParam;
        
        // Call the foo method on template_module
        QVariant fooResult = logosAPI->invokeRemoteMethod("template_module", "foo", testParam);
        
        if (fooResult.isValid() && fooResult.toBool()) {
            qDebug() << "CapabilityModulePlugin: Successfully called template_module foo() method, result:" << fooResult.toBool();
        } else {
            qWarning() << "CapabilityModulePlugin: Failed to call template_module foo() method or method returned false";
        }
    } else {
        qWarning() << "CapabilityModulePlugin: LogosAPI not connected, cannot call template_module foo method";
    }

    qDebug() << "CapabilityModulePlugin::requestModule returning:" << result;
    
    return result;
}
