#include "logos_api_provider.h"
#include "module_proxy.h"
#include <QRemoteObjectRegistryHost>
#include <QDebug>
#include <QUrl>
#include <QMetaObject>

LogosAPIProvider::LogosAPIProvider(const QString& module_name, QObject *parent)
    : QObject(parent)
    , m_registryHost(nullptr)
    , m_registryUrl(QString("local:logos_%1").arg(module_name))
    , m_moduleProxy(nullptr)
{
}

LogosAPIProvider::~LogosAPIProvider()
{
    // QRemoteObjectRegistryHost will be deleted automatically as it's a child object
    // ModuleProxy will be deleted automatically as it's a child object
}

bool LogosAPIProvider::registerObject(const QString& name, QObject* object, const QString& authToken)
{
    if (!object) {
        qWarning() << "LogosAPIProvider: Cannot register null object";
        return false;
    }

    if (name.isEmpty()) {
        qWarning() << "LogosAPIProvider: Cannot register object with empty name";
        return false;
    }

    if (authToken.isEmpty()) {
        qWarning() << "LogosAPIProvider: Cannot register object with empty auth token";
        return false;
    }

    // Check if a ModuleProxy was already created - only allow one registration
    if (m_moduleProxy) {
        qCritical() << "LogosAPIProvider: Object already registered. Only one registration per provider is allowed";
        return false;
    }

    qDebug() << "LogosAPIProvider: Creating ModuleProxy for" << name << "wrapping the provided object";
    
    m_moduleProxy = new ModuleProxy(object, authToken, this);
    object = m_moduleProxy;

    if (!m_registryHost) {
        m_registryHost = new QRemoteObjectRegistryHost(QUrl(m_registryUrl));
        if (!m_registryHost) {
            qCritical() << "LogosAPIProvider: Failed to create registry host";
            return false;
        }
        qDebug() << "LogosAPIProvider: Created registry host with URL:" << m_registryUrl;
    }

    bool success = m_registryHost->enableRemoting(object, name);
    if (success) {
        qDebug() << "LogosAPIProvider: Successfully registered object with name:" << name;
    } else {
        qCritical() << "LogosAPIProvider: Failed to register object with name:" << name;
    }

    return success;
}

QString LogosAPIProvider::registryUrl() const
{
    return m_registryUrl;
}

void LogosAPIProvider::onEventResponse(QObject* replica, const QString& eventName, const QVariantList& data)
{
    // qDebug() << "LogosAPIProvider: Received event:" << eventName << "with data:" << data;
    qDebug() << "LogosAPIProvider: Received event:" << eventName;

    if (eventName.isEmpty()) {
        qWarning() << "LogosAPIProvider: Event name cannot be empty";
        return;
    }

    // qDebug() << "LogosAPIProvider: Emitting event:" << eventName << "with data:" << data;
    qDebug() << "LogosAPIProvider: Emitting event:" << eventName;

    // emit the eventResponse signal of replica
    QMetaObject::invokeMethod(replica, "eventResponse", Qt::QueuedConnection, Q_ARG(QString, eventName), Q_ARG(QVariantList, data));
    // QMetaObject::invokeMethod(replica, "eventResponse_another", Qt::QueuedConnection, Q_ARG(QString, eventName), Q_ARG(QVariantList, data));
    // TODO: try queued connection instead
    // QMetaObject::invokeMethod(replica, "eventResponse_another", Qt::DirectConnection, Q_ARG(QString, eventName), Q_ARG(QVariantList, data));
}

