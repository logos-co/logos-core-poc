#include "logos_api_consumer.h"
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

LogosAPIConsumer::LogosAPIConsumer(const QString& module_to_talk_to, const QString& origin_module, QObject *parent)
    : QObject(parent)
    , m_node(nullptr)
    , m_registryUrl(QString("local:logos_%1").arg(module_to_talk_to))
    , m_connected(false)
{
    m_node = new QRemoteObjectNode(this);
    connectToRegistry();
}

LogosAPIConsumer::~LogosAPIConsumer()
{
    // Clean up event callbacks and connections
    for (auto it = m_connections.begin(); it != m_connections.end(); ++it) {
        QObject::disconnect(it.value());
    }
    m_eventCallbacks.clear();
    m_connections.clear();
    
    // QRemoteObjectNode will be deleted automatically as it's a child object
}

QObject* LogosAPIConsumer::requestObject(const QString& objectName, int timeoutMs)
{
    qDebug() << "LogosAPIConsumer: Requesting object:" << objectName << "at" << QTime::currentTime().toString("hh:mm:ss.zzz");
    if (!m_connected) {
        qWarning() << "LogosAPIConsumer: Not connected to registry. Cannot request object:" << objectName;
        return nullptr;
    }

    if (objectName.isEmpty()) {
        qWarning() << "LogosAPIConsumer: Object name cannot be empty";
        return nullptr;
    }

    qDebug() << "LogosAPIConsumer: Requesting object:" << objectName;

    // Acquire the dynamic replica
    QRemoteObjectReplica* replica = m_node->acquireDynamic(objectName);
    if (!replica) {
        qWarning() << "LogosAPIConsumer: Failed to acquire replica for object:" << objectName;
        return nullptr;
    }

    // Wait for the replica to be initialized
    if (!replica->waitForSource(timeoutMs)) {
        qWarning() << "LogosAPIConsumer: Timeout waiting for object replica to be ready:" << objectName;
        delete replica;
        return nullptr;
    }

    qDebug() << "LogosAPIConsumer: Successfully acquired replica for object:" << objectName;
    qDebug() << "LogosAPIConsumer: Replica acquired at" << QTime::currentTime().toString("hh:mm:ss.zzz");
    return replica;
}

bool LogosAPIConsumer::isConnected() const
{
    return m_connected;
}

QString LogosAPIConsumer::registryUrl() const
{
    return m_registryUrl;
}

bool LogosAPIConsumer::reconnect()
{
    qDebug() << "LogosAPIConsumer: Attempting to reconnect to registry:" << m_registryUrl;
    
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

bool LogosAPIConsumer::connectToRegistry()
{
    if (!m_node) {
        qWarning() << "LogosAPIConsumer: Remote object node is null";
        return false;
    }

    if (m_registryUrl.isEmpty()) {
        qWarning() << "LogosAPIConsumer: Registry URL is empty";
        return false;
    }

    qDebug() << "LogosAPIConsumer: Connecting to registry:" << m_registryUrl;
    qDebug() << "LogosAPIConsumer: Connecting to registry at" << QTime::currentTime().toString("hh:mm:ss.zzz");

    // Connect to the registry node
    QUrl url(m_registryUrl);
    bool success = m_node->connectToNode(url);
    
    if (success) {
        m_connected = true;
        qDebug() << "LogosAPIConsumer: Successfully connected to registry:" << m_registryUrl;
    } else {
        m_connected = false;
        qWarning() << "LogosAPIConsumer: Failed to connect to registry:" << m_registryUrl;
    }
    qDebug() << "LogosAPIConsumer: Connected to registry at" << QTime::currentTime().toString("hh:mm:ss.zzz");

    return m_connected;
}

QString LogosAPIConsumer::getToken(const QString& module_name)
{
    if (m_tokens.contains(module_name)) {
        qDebug() << "LogosAPIConsumer: Using stored token for module:" << module_name;
        return m_tokens[module_name];
    }
    qDebug() << "LogosAPIConsumer: No stored token for module:" << module_name << "- using default AUTH_TOKEN";
    return AUTH_TOKEN;
}

QVariant LogosAPIConsumer::invokeRemoteMethod(const QString& objectName, const QString& methodName, 
                                   const QVariantList& args, int timeoutMs)
{
    qDebug() << "LogosAPIConsumer: Calling invokeRemoteMethod with params:" << objectName << methodName << args << timeoutMs;

    // first talk to core_manager to call requestModule and print the result
    // if (objectName != "core_manager") {
    //     // Call core_manager's requestModule method first
    //     // TODO: not working because it's requesting core_manager from the wrong URL
    //     QVariant moduleResult = invokeRemoteMethod("core_manager", "requestModule", QVariantList() << objectName, timeoutMs);
    //     if (moduleResult.isValid()) {
    //         qDebug() << "LogosAPIConsumer: requestModule result for" << objectName << ":" << moduleResult.toString();
    //     } else {
    //         qWarning() << "LogosAPIConsumer: Failed to get requestModule result for" << objectName;
    //     }
    // }

    // This method handles both ModuleProxy-wrapped modules (template_module, package_manager) 
    // and direct remote object calls for other modules
    QObject* replica = requestObject(objectName, timeoutMs);
    if (!replica) {
        qWarning() << "LogosAPIConsumer: Failed to acquire replica for object:" << objectName;
        return QVariant();
    }

    // get the token for the module
    QString token = getToken(objectName);

    // Try to cast to ModuleProxy first (in case the replica is a wrapped module)
    ModuleProxy* moduleProxy = qobject_cast<ModuleProxy*>(replica);
    if (moduleProxy) {
        QVariant result = moduleProxy->callRemoteMethod(token, methodName, args);
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
        Q_ARG(QString, token),
        Q_ARG(QString, methodName),
        Q_ARG(QVariantList, args)
    );

    if (!success) {
        qWarning() << "LogosAPIConsumer: Failed to invoke callRemoteMethod on replica for object:" << objectName;
        delete replica;
        return QVariant();
    }

    // Wait for the result
    pendingCall.waitForFinished(timeoutMs);
    delete replica;

    if (!pendingCall.isFinished() || pendingCall.error() != QRemoteObjectPendingCall::NoError) {
        qWarning() << "LogosAPIConsumer: Remote callRemoteMethod failed or timed out:" << pendingCall.error();
        return QVariant();
    }

    return pendingCall.returnValue();
}

// // with one param
// QVariant LogosAPIConsumer::invokeRemoteMethod(const QString& objectName, const QString& methodName, 
//                                    const QVariant& arg, int timeoutMs)
// {
//     return invokeRemoteMethod(objectName, methodName, QVariantList() << arg, timeoutMs);
// }
// 
// // with two params
// QVariant LogosAPIConsumer::invokeRemoteMethod(const QString& objectName, const QString& methodName, 
//                                    const QVariant& arg1, const QVariant& arg2, int timeoutMs)
// {
//     return invokeRemoteMethod(objectName, methodName, QVariantList() << arg1 << arg2, timeoutMs);
// }
// 
// // with three params
// QVariant LogosAPIConsumer::invokeRemoteMethod(const QString& objectName, const QString& methodName, 
//                                    const QVariant& arg1, const QVariant& arg2, const QVariant& arg3, int timeoutMs)
// {
//     return invokeRemoteMethod(objectName, methodName, QVariantList() << arg1 << arg2 << arg3, timeoutMs);
// }
// 
// // with four params
// QVariant LogosAPIConsumer::invokeRemoteMethod(const QString& objectName, const QString& methodName, 
//                                    const QVariant& arg1, const QVariant& arg2, const QVariant& arg3, 
//                                    const QVariant& arg4, int timeoutMs)
// {
//     return invokeRemoteMethod(objectName, methodName, QVariantList() << arg1 << arg2 << arg3 << arg4, timeoutMs);
// }
// 
// // with five params
// QVariant LogosAPIConsumer::invokeRemoteMethod(const QString& objectName, const QString& methodName, 
//                                    const QVariant& arg1, const QVariant& arg2, const QVariant& arg3, 
//                                    const QVariant& arg4, const QVariant& arg5, int timeoutMs)
// {
//     return invokeRemoteMethod(objectName, methodName, QVariantList() << arg1 << arg2 << arg3 << arg4 << arg5, timeoutMs);
// }

// change from objectName to instead, originObject and destinationObject, and use QObject instead
void LogosAPIConsumer::onEvent(QObject* originObject, QObject* destinationObject, const QString& eventName, std::function<void(const QString&, const QVariantList&)> callback)
{
    qDebug() << "LogosAPIConsumer: Registering event listener for event:" << eventName;

    // Store the callback for this event name
    m_eventCallbacks[eventName].append(callback);

    // Check if we already have a connection for this origin object
    if (!m_connections.contains(originObject)) {
        // Create new connection only if it doesn't exist
        auto connection = QObject::connect(originObject, SIGNAL(eventResponse(QString, QVariantList)), 
                                          this, SLOT(invokeCallback(QString, QVariantList)));
        
        if (connection) {
            m_connections[originObject] = connection;
            qDebug() << "LogosAPIConsumer: Created new connection for origin object";
        } else {
            qWarning() << "LogosAPIConsumer: Failed to create connection for event:" << eventName;
        }
    } else {
        qDebug() << "LogosAPIConsumer: Reusing existing connection for origin object";
    }
    
    qDebug() << "LogosAPIConsumer: Registered callback for event:" << eventName;
}

void LogosAPIConsumer::invokeCallback(const QString& eventName, const QVariantList& data)
{
    // qDebug() << "LogosAPIConsumer: invokeCallback called for event:" << eventName;
    
    // Call all registered callbacks
    // Note: This will call all callbacks for any event. In a more sophisticated implementation,
    // you might want to store event names with callbacks to filter them.
    for (const auto& callback : m_eventCallbacks[eventName]) {
        try {
            callback(eventName, data);
        } catch (...) {
            qWarning() << "LogosAPIConsumer: Exception in callback for event:" << eventName;
        }
    }
    
    // qDebug() << "LogosAPIConsumer: Called" << m_eventCallbacks[eventName].size() << "callbacks for event:" << eventName;
}

void LogosAPIConsumer::onEvent(QObject* originObject, QObject* destinationObject, const QString& eventName)
{
    qDebug() << "LogosAPIConsumer: Registering event listener for event:" << eventName << "(connecting to destination slot)";

    // connect to the eventResponse signal of the destinationObject's slot
    QObject::connect(originObject, SIGNAL(eventResponse(QString, QVariantList)), 
                    destinationObject, SLOT(onEventResponse(QString, QVariantList)), Qt::AutoConnection);
}

void LogosAPIConsumer::saveToken(const QString& to_module_name, const QString& token)
{
    qDebug() << "LogosAPIConsumer: Saving token for module:" << to_module_name;
    
    if (to_module_name.isEmpty()) {
        qWarning() << "LogosAPIConsumer: Module name cannot be empty when saving token";
        return;
    }
    
    m_tokens[to_module_name] = token;
    qDebug() << "LogosAPIConsumer: Token saved for module:" << to_module_name;
}

// Include MOC for template instantiation
#include "moc_logos_api_consumer.cpp" 