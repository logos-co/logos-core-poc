#include "logos_api_client.h"
#include "logos_api_consumer.h"

LogosAPIClient::LogosAPIClient(const QString& module_name, QObject *parent)
    : QObject(parent)
    , m_consumer(new LogosAPIConsumer(module_name, this))
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
    return m_consumer->invokeRemoteMethod(objectName, methodName, args, timeoutMs);
}

QVariant LogosAPIClient::invokeRemoteMethod(const QString& objectName, const QString& methodName, 
                                   const QVariant& arg, int timeoutMs)
{
    return m_consumer->invokeRemoteMethod(objectName, methodName, arg, timeoutMs);
}

QVariant LogosAPIClient::invokeRemoteMethod(const QString& objectName, const QString& methodName, 
                                   const QVariant& arg1, const QVariant& arg2, int timeoutMs)
{
    return m_consumer->invokeRemoteMethod(objectName, methodName, arg1, arg2, timeoutMs);
}

QVariant LogosAPIClient::invokeRemoteMethod(const QString& objectName, const QString& methodName, 
                                   const QVariant& arg1, const QVariant& arg2, const QVariant& arg3, int timeoutMs)
{
    return m_consumer->invokeRemoteMethod(objectName, methodName, arg1, arg2, arg3, timeoutMs);
}

QVariant LogosAPIClient::invokeRemoteMethod(const QString& objectName, const QString& methodName, 
                                   const QVariant& arg1, const QVariant& arg2, const QVariant& arg3, 
                                   const QVariant& arg4, int timeoutMs)
{
    return m_consumer->invokeRemoteMethod(objectName, methodName, arg1, arg2, arg3, arg4, timeoutMs);
}

QVariant LogosAPIClient::invokeRemoteMethod(const QString& objectName, const QString& methodName, 
                                   const QVariant& arg1, const QVariant& arg2, const QVariant& arg3, 
                                   const QVariant& arg4, const QVariant& arg5, int timeoutMs)
{
    return m_consumer->invokeRemoteMethod(objectName, methodName, arg1, arg2, arg3, arg4, arg5, timeoutMs);
}

void LogosAPIClient::onEvent(QObject* originObject, QObject* destinationObject, const QString& eventName, std::function<void(const QString&, const QVariantList&)> callback)
{
    m_consumer->onEvent(originObject, destinationObject, eventName, callback);
}

void LogosAPIClient::onEvent(QObject* originObject, QObject* destinationObject, const QString& eventName)
{
    m_consumer->onEvent(originObject, destinationObject, eventName);
}

void LogosAPIClient::saveToken(const QString& to_module_name, const QString& token)
{
    m_consumer->saveToken(to_module_name, token);
}

QString LogosAPIClient::getToken(const QString& module_name)
{
    return m_consumer->getToken(module_name);
}

void LogosAPIClient::invokeCallback(const QString& eventName, const QVariantList& data)
{
    m_consumer->invokeCallback(eventName, data);
}

// Include MOC for template instantiation
#include "moc_logos_api_client.cpp" 