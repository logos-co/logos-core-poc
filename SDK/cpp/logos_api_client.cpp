#include "logos_api_client.h"
#include "logos_api_consumer.h"

LogosAPIClient::LogosAPIClient(const QString& module_to_talk_to, const QString& origin_module, QObject *parent)
    : QObject(parent)
    , m_consumer(new LogosAPIConsumer(module_to_talk_to, origin_module, this))
{
}

LogosAPIClient::~LogosAPIClient()
{
    // m_consumer will be deleted automatically as it's a child object
}

QObject* LogosAPIClient::requestObject(const QString& objectName, int timeoutMs)
{
    return m_consumer->requestObject(objectName, timeoutMs);
}

bool LogosAPIClient::isConnected() const
{
    return m_consumer->isConnected();
}

QString LogosAPIClient::registryUrl() const
{
    return m_consumer->registryUrl();
}

bool LogosAPIClient::reconnect()
{
    return m_consumer->reconnect();
}

QVariant LogosAPIClient::invokeRemoteMethod(const QString& objectName, const QString& methodName, 
                                   const QVariantList& args, int timeoutMs)
{
    // Additional client-level logic can be added here
    // For example, capability module requests or token handling
    
    //if (objectName != "capability_module") {
    //    qDebug() << "LogosAPIClient: calling requestModule for" << objectName;
    //    LogosAPIConsumer* packageManagerConsumer = new LogosAPIConsumer("capability_module", "origin_module", this);
    //    QVariant result = packageManagerConsumer->invokeRemoteMethod("capability_module", "requestModule", QVariantList() << objectName, timeoutMs);
    //    qDebug() << "================================================";
    //    qDebug() << "================================================";
    //    qDebug() << "================================================";
    //    qDebug() << "================================================";
    //    qDebug() << "================================================";
    //    qDebug() << "================================================";
    //    qDebug() << "LogosAPIClient: requestModule result for" << objectName << ":" << result.toString();
    //    qDebug() << "================================================";
    //    qDebug() << "================================================";
    //    qDebug() << "================================================";
    //    qDebug() << "================================================";
    //    qDebug() << "================================================";
    //}

    return m_consumer->invokeRemoteMethod(objectName, methodName, args, timeoutMs);
}

QVariant LogosAPIClient::invokeRemoteMethod(const QString& objectName, const QString& methodName, 
                                   const QVariant& arg, int timeoutMs)
{
    return invokeRemoteMethod(objectName, methodName, QVariantList() << arg, timeoutMs);
}

QVariant LogosAPIClient::invokeRemoteMethod(const QString& objectName, const QString& methodName, 
                                   const QVariant& arg1, const QVariant& arg2, int timeoutMs)
{
    return invokeRemoteMethod(objectName, methodName, QVariantList() << arg1 << arg2, timeoutMs);
}

QVariant LogosAPIClient::invokeRemoteMethod(const QString& objectName, const QString& methodName, 
                                   const QVariant& arg1, const QVariant& arg2, const QVariant& arg3, int timeoutMs)
{
    return invokeRemoteMethod(objectName, methodName, QVariantList() << arg1 << arg2 << arg3, timeoutMs);
}

QVariant LogosAPIClient::invokeRemoteMethod(const QString& objectName, const QString& methodName, 
                                   const QVariant& arg1, const QVariant& arg2, const QVariant& arg3, 
                                   const QVariant& arg4, int timeoutMs)
{
    return invokeRemoteMethod(objectName, methodName, QVariantList() << arg1 << arg2 << arg3 << arg4, timeoutMs);
}

QVariant LogosAPIClient::invokeRemoteMethod(const QString& objectName, const QString& methodName, 
                                   const QVariant& arg1, const QVariant& arg2, const QVariant& arg3, 
                                   const QVariant& arg4, const QVariant& arg5, int timeoutMs)
{
    return invokeRemoteMethod(objectName, methodName, QVariantList() << arg1 << arg2 << arg3 << arg4 << arg5, timeoutMs);
}

void LogosAPIClient::onEvent(QObject* originObject, QObject* destinationObject, const QString& eventName, std::function<void(const QString&, const QVariantList&)> callback)
{
    m_consumer->onEvent(originObject, destinationObject, eventName, callback);
}

void LogosAPIClient::onEvent(QObject* originObject, QObject* destinationObject, const QString& eventName)
{
    m_consumer->onEvent(originObject, destinationObject, eventName);
}



void LogosAPIClient::invokeCallback(const QString& eventName, const QVariantList& data)
{
    m_consumer->invokeCallback(eventName, data);
}

void LogosAPIClient::onEventResponse(QObject* replica, const QString& eventName, const QVariantList& data)
{
    // qDebug() << "LogosAPIClient: Received event:" << eventName << "with data:" << data;
    qDebug() << "LogosAPIClient: Received event:" << eventName;

    if (eventName.isEmpty()) {
        qWarning() << "LogosAPIClient: Event name cannot be empty";
        return;
    }

    // qDebug() << "LogosAPIClient: Emitting event:" << eventName << "with data:" << data;
    qDebug() << "LogosAPIClient: Emitting event:" << eventName;

    // emit the eventResponse signal of replica
    QMetaObject::invokeMethod(replica, "eventResponse", Qt::QueuedConnection, Q_ARG(QString, eventName), Q_ARG(QVariantList, data));
} 

bool LogosAPIClient::informModuleToken(const QString& authToken, const QString& moduleName, const QString& token)
{
    return m_consumer->informModuleToken(authToken, moduleName, token);
}