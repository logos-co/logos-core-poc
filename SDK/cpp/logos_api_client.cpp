#include "logos_api_client.h"
#include "module_proxy.h"
#include <QRemoteObjectNode>
#include <QRemoteObjectReplica>
#include <QRemoteObjectPendingCall>
#include <QDebug>
#include <QUrl>
#include <QMetaObject>
#include <QTime>
#include <string>

const QString AUTH_TOKEN = "abc";

LogosAPIClient::LogosAPIClient(const QString& module_name, QObject *parent)
    : QObject(parent)
    , m_node(nullptr)
    , m_registryUrl(QString("local:logos_%1").arg(module_name))
    , m_connected(false)
{
    m_node = new QRemoteObjectNode(this);
    connectToRegistry();
}

LogosAPIClient::~LogosAPIClient()
{
    // Clean up event callbacks and connections
    for (auto it = m_connections.begin(); it != m_connections.end(); ++it) {
        QObject::disconnect(it.value());
    }
    m_eventCallbacks.clear();
    m_connections.clear();
    
    // QRemoteObjectNode will be deleted automatically as it's a child object
}

QObject* LogosAPIClient::requestObject(const QString& objectName, int timeoutMs)
{
    qDebug() << "LogosAPIClient: Requesting object:" << objectName << "at" << QTime::currentTime().toString("hh:mm:ss.zzz");
    if (!m_connected) {
        qWarning() << "LogosAPIClient: Not connected to registry. Cannot request object:" << objectName;
        return nullptr;
    }

    if (objectName.isEmpty()) {
        qWarning() << "LogosAPIClient: Object name cannot be empty";
        return nullptr;
    }

    qDebug() << "LogosAPIClient: Requesting object:" << objectName;

    // Acquire the dynamic replica
    QRemoteObjectReplica* replica = m_node->acquireDynamic(objectName);
    if (!replica) {
        qWarning() << "LogosAPIClient: Failed to acquire replica for object:" << objectName;
        return nullptr;
    }

    // Wait for the replica to be initialized
    if (!replica->waitForSource(timeoutMs)) {
        qWarning() << "LogosAPIClient: Timeout waiting for object replica to be ready:" << objectName;
        delete replica;
        return nullptr;
    }

    qDebug() << "LogosAPIClient: Successfully acquired replica for object:" << objectName;
    qDebug() << "LogosAPIClient: Replica acquired at" << QTime::currentTime().toString("hh:mm:ss.zzz");
    return replica;
}

bool LogosAPIClient::isConnected() const
{
    return m_connected;
}

QString LogosAPIClient::registryUrl() const
{
    return m_registryUrl;
}

bool LogosAPIClient::reconnect()
{
    qDebug() << "LogosAPIClient: Attempting to reconnect to registry:" << m_registryUrl;
    
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

bool LogosAPIClient::connectToRegistry()
{
    if (!m_node) {
        qWarning() << "LogosAPIClient: Remote object node is null";
        return false;
    }

    if (m_registryUrl.isEmpty()) {
        qWarning() << "LogosAPIClient: Registry URL is empty";
        return false;
    }

    qDebug() << "LogosAPIClient: Connecting to registry:" << m_registryUrl;
    qDebug() << "LogosAPIClient: Connecting to registry at" << QTime::currentTime().toString("hh:mm:ss.zzz");

    // Connect to the registry node
    QUrl url(m_registryUrl);
    bool success = m_node->connectToNode(url);
    
    if (success) {
        m_connected = true;
        qDebug() << "LogosAPIClient: Successfully connected to registry:" << m_registryUrl;
    } else {
        m_connected = false;
        qWarning() << "LogosAPIClient: Failed to connect to registry:" << m_registryUrl;
    }
    qDebug() << "LogosAPIClient: Connected to registry at" << QTime::currentTime().toString("hh:mm:ss.zzz");

    return m_connected;
}

QVariant LogosAPIClient::invokeRemoteMethod(const QString& objectName, const QString& methodName, 
                                   const QVariantList& args, int timeoutMs)
{
    qDebug() << "LogosAPIClient: Calling invokeRemoteMethod with params:" << objectName << methodName << args << timeoutMs;

    // This method handles both ModuleProxy-wrapped modules (template_module, package_manager) 
    // and direct remote object calls for other modules
    QObject* replica = requestObject(objectName, timeoutMs);
    if (!replica) {
        qWarning() << "LogosAPIClient: Failed to acquire replica for object:" << objectName;
        return QVariant();
    }

    // Try to cast to ModuleProxy first (in case the replica is a wrapped module)
    ModuleProxy* moduleProxy = qobject_cast<ModuleProxy*>(replica);
    if (moduleProxy) {
        QVariant result = moduleProxy->callRemoteMethod(AUTH_TOKEN, methodName, args);
        delete replica;
        return result;
    }

    // Fallback: use QMetaObject::invokeMethod directly
    // Note: Remote objects' callRemoteMethod returns QRemoteObjectPendingCall, not QVariant
    QRemoteObjectPendingCall pendingCall;
    bool success = QMetaObject::invokeMethod(
        replica,
        "callRemoteMethod",
        Qt::DirectConnection,
        Q_RETURN_ARG(QRemoteObjectPendingCall, pendingCall),
        Q_ARG(QString, AUTH_TOKEN),
        Q_ARG(QString, methodName),
        Q_ARG(QVariantList, args)
    );

    if (!success) {
        qWarning() << "LogosAPIClient: Failed to invoke callRemoteMethod on replica for object:" << objectName;
        delete replica;
        return QVariant();
    }

    // Wait for the result
    pendingCall.waitForFinished(timeoutMs);
    delete replica;

    if (!pendingCall.isFinished() || pendingCall.error() != QRemoteObjectPendingCall::NoError) {
        qWarning() << "LogosAPIClient: Remote callRemoteMethod failed or timed out:" << pendingCall.error();
        return QVariant();
    }

    return pendingCall.returnValue();
}

// with one param
QVariant LogosAPIClient::invokeRemoteMethod(const QString& objectName, const QString& methodName, 
                                   const QVariant& arg, int timeoutMs)
{
    return invokeRemoteMethod(objectName, methodName, QVariantList() << arg, timeoutMs);
}

// with two params
QVariant LogosAPIClient::invokeRemoteMethod(const QString& objectName, const QString& methodName, 
                                   const QVariant& arg1, const QVariant& arg2, int timeoutMs)
{
    return invokeRemoteMethod(objectName, methodName, QVariantList() << arg1 << arg2, timeoutMs);
}

// with three params
QVariant LogosAPIClient::invokeRemoteMethod(const QString& objectName, const QString& methodName, 
                                   const QVariant& arg1, const QVariant& arg2, const QVariant& arg3, int timeoutMs)
{
    return invokeRemoteMethod(objectName, methodName, QVariantList() << arg1 << arg2 << arg3, timeoutMs);
}

// with four params
QVariant LogosAPIClient::invokeRemoteMethod(const QString& objectName, const QString& methodName, 
                                   const QVariant& arg1, const QVariant& arg2, const QVariant& arg3, 
                                   const QVariant& arg4, int timeoutMs)
{
    return invokeRemoteMethod(objectName, methodName, QVariantList() << arg1 << arg2 << arg3 << arg4, timeoutMs);
}

// with five params
QVariant LogosAPIClient::invokeRemoteMethod(const QString& objectName, const QString& methodName, 
                                   const QVariant& arg1, const QVariant& arg2, const QVariant& arg3, 
                                   const QVariant& arg4, const QVariant& arg5, int timeoutMs)
{
    return invokeRemoteMethod(objectName, methodName, QVariantList() << arg1 << arg2 << arg3 << arg4 << arg5, timeoutMs);
}

// change from objectName to instead, originObject and destinationObject, and use QObject instead
void LogosAPIClient::onEvent(QObject* originObject, QObject* destinationObject, const QString& eventName, std::function<void(const QString&, const QVariantList&)> callback)
{
    qDebug() << "LogosAPIClient: Registering event listener for event:" << eventName;

    // Store the callback for this event name
    m_eventCallbacks[eventName].append(callback);

    // Check if we already have a connection for this origin object
    if (!m_connections.contains(originObject)) {
        // Create new connection only if it doesn't exist
        auto connection = QObject::connect(originObject, SIGNAL(eventResponse(QString, QVariantList)), 
                                          this, SLOT(invokeCallback(QString, QVariantList)));
        
        if (connection) {
            m_connections[originObject] = connection;
            qDebug() << "LogosAPIClient: Created new connection for origin object";
        } else {
            qWarning() << "LogosAPIClient: Failed to create connection for event:" << eventName;
        }
    } else {
        qDebug() << "LogosAPIClient: Reusing existing connection for origin object";
    }
    
    qDebug() << "LogosAPIClient: Registered callback for event:" << eventName;
}

void LogosAPIClient::invokeCallback(const QString& eventName, const QVariantList& data)
{
    qDebug() << "LogosAPIClient: invokeCallback called for event:" << eventName;
    
    // Call all registered callbacks
    // Note: This will call all callbacks for any event. In a more sophisticated implementation,
    // you might want to store event names with callbacks to filter them.
    for (const auto& callback : m_eventCallbacks[eventName]) {
        try {
            callback(eventName, data);
        } catch (...) {
            qWarning() << "LogosAPIClient: Exception in callback for event:" << eventName;
        }
    }
    
    qDebug() << "LogosAPIClient: Called" << m_eventCallbacks[eventName].size() << "callbacks for event:" << eventName;
}

void LogosAPIClient::onEvent(QObject* originObject, QObject* destinationObject, const QString& eventName)
{
    qDebug() << "LogosAPIClient: Registering event listener for event:" << eventName << "(connecting to destination slot)";

    // connect to the eventResponse signal of the destinationObject's slot
    QObject::connect(originObject, SIGNAL(eventResponse(QString, QVariantList)), 
                    destinationObject, SLOT(onEventResponse(QString, QVariantList)), Qt::AutoConnection);
}

// Include MOC for template instantiation
#include "moc_logos_api_client.cpp" 