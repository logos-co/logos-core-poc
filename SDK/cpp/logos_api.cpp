#include "logos_api.h"
#include <QRemoteObjectNode>
#include <QRemoteObjectReplica>
#include <QDebug>
#include <QUrl>

LogosAPI::LogosAPI(const QString& registryUrl, QObject *parent)
    : QObject(parent)
    , m_node(nullptr)
    , m_registryUrl(registryUrl)
    , m_connected(false)
{
    m_node = new QRemoteObjectNode(this);
    connectToRegistry();
}

LogosAPI::~LogosAPI()
{
    // QRemoteObjectNode will be deleted automatically as it's a child object
}

QRemoteObjectReplica* LogosAPI::requestObject(const QString& objectName, int timeoutMs)
{
    qDebug() << "LogosAPI: Requesting object:" << objectName;
    if (!m_connected) {
        qWarning() << "LogosAPI: Not connected to registry. Cannot request object:" << objectName;
        return nullptr;
    }

    if (objectName.isEmpty()) {
        qWarning() << "LogosAPI: Object name cannot be empty";
        return nullptr;
    }

    qDebug() << "LogosAPI: Requesting object:" << objectName;

    // Acquire the dynamic replica
    QRemoteObjectReplica* replica = m_node->acquireDynamic(objectName);
    if (!replica) {
        qWarning() << "LogosAPI: Failed to acquire replica for object:" << objectName;
        return nullptr;
    }

    // Wait for the replica to be initialized
    if (!replica->waitForSource(timeoutMs)) {
        qWarning() << "LogosAPI: Timeout waiting for object replica to be ready:" << objectName;
        delete replica;
        return nullptr;
    }

    qDebug() << "LogosAPI: Successfully acquired replica for object:" << objectName;
    return replica;
}

bool LogosAPI::isConnected() const
{
    return m_connected;
}

QString LogosAPI::registryUrl() const
{
    return m_registryUrl;
}

bool LogosAPI::reconnect()
{
    qDebug() << "LogosAPI: Attempting to reconnect to registry:" << m_registryUrl;
    
    // Disconnect first if already connected
    if (m_connected) {
        // Note: QRemoteObjectNode doesn't have a direct disconnect method
        // We'll create a new node instead
        m_node->deleteLater();
        m_node = new QRemoteObjectNode(this);
        m_connected = false;
    }
    
    return connectToRegistry();
}

bool LogosAPI::connectToRegistry()
{
    if (!m_node) {
        qWarning() << "LogosAPI: Remote object node is null";
        return false;
    }

    if (m_registryUrl.isEmpty()) {
        qWarning() << "LogosAPI: Registry URL is empty";
        return false;
    }

    qDebug() << "LogosAPI: Connecting to registry:" << m_registryUrl;

    // Connect to the registry node
    QUrl url(m_registryUrl);
    bool success = m_node->connectToNode(url);
    
    if (success) {
        m_connected = true;
        qDebug() << "LogosAPI: Successfully connected to registry:" << m_registryUrl;
    } else {
        m_connected = false;
        qWarning() << "LogosAPI: Failed to connect to registry:" << m_registryUrl;
    }

    return m_connected;
} 