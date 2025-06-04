#include "logos_api.h"
#include <QRemoteObjectNode>
#include <QRemoteObjectReplica>
#include <QRemoteObjectPendingCall>
#include <QDebug>
#include <QUrl>
#include <QMetaObject>
#include <QTime>
#include <string>

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
    // Clean up event callbacks and connections
    for (auto it = m_connections.begin(); it != m_connections.end(); ++it) {
        QObject::disconnect(it.value());
    }
    m_eventCallbacks.clear();
    m_connections.clear();
    
    // QRemoteObjectNode will be deleted automatically as it's a child object
}

// TODO: return of this could be a LogosModule to it's easier to abstract later
QObject* LogosAPI::requestObject(const QString& objectName, int timeoutMs)
{
    qDebug() << "LogosAPI: Requesting object:" << objectName << "at" << QTime::currentTime().toString("hh:mm:ss.zzz");
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
    qDebug() << "LogosAPI: Replica acquired at" << QTime::currentTime().toString("hh:mm:ss.zzz");
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
    qDebug() << "LogosAPI: Connecting to registry at" << QTime::currentTime().toString("hh:mm:ss.zzz");

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
    qDebug() << "LogosAPI: Connected to registry at" << QTime::currentTime().toString("hh:mm:ss.zzz");

    return m_connected;
}

// Helper function to create QGenericArgument from QVariant
auto LogosAPI::createArgument(const QVariant& variant)
{
    switch (variant.type()) {
        case QVariant::String: {
            m_stringArgs.append(variant.toString());
            return Q_ARG(QString, m_stringArgs.last());
        }
        default: {
            // For now, convert everything else to string as fallback
            m_stringArgs.append(variant.toString());
            return Q_ARG(QString, m_stringArgs.last());
        }
    }
}

QVariant LogosAPI::callRemoteMethod(const QString& objectName, const QString& methodName, 
                                   const QVariantList& args, int timeoutMs)
{
    if (!m_connected) {
        qWarning() << "LogosAPI: Not connected to registry. Cannot call method:" << methodName;
        return QVariant();
    }

    if (objectName.isEmpty() || methodName.isEmpty()) {
        qWarning() << "LogosAPI: Object name and method name cannot be empty";
        return QVariant();
    }

    qDebug() << "LogosAPI: Calling method" << methodName << "on object" << objectName;

    // Clear string storage before each call
    m_stringArgs.clear();

    // Get the replica
    QObject* replica = requestObject(objectName, timeoutMs);
    if (!replica) {
        qWarning() << "LogosAPI: Failed to acquire replica for object:" << objectName;
        return QVariant();
    }

    // Prepare the method call
    QRemoteObjectPendingCall pendingCall;
    bool success = false;

    // Handle different argument counts
    if (args.isEmpty()) {
        success = QMetaObject::invokeMethod(
            replica,
            methodName.toUtf8().constData(),
            Qt::DirectConnection,
            Q_RETURN_ARG(QRemoteObjectPendingCall, pendingCall)
        );
    } else if (args.size() == 1) {
        success = QMetaObject::invokeMethod(
            replica,
            methodName.toUtf8().constData(),
            Qt::DirectConnection,
            Q_RETURN_ARG(QRemoteObjectPendingCall, pendingCall),
            createArgument(args[0])
        );
    } else if (args.size() == 2) {
        success = QMetaObject::invokeMethod(
            replica,
            methodName.toUtf8().constData(),
            Qt::DirectConnection,
            Q_RETURN_ARG(QRemoteObjectPendingCall, pendingCall),
            createArgument(args[0]),
            createArgument(args[1])
        );
    } else if (args.size() == 3) {
        // For void methods like sendMessage, don't expect a return value
        success = QMetaObject::invokeMethod(
            replica,
            methodName.toUtf8().constData(),
            Qt::DirectConnection,
            createArgument(args[0]),
            createArgument(args[1]),
            createArgument(args[2])
        );
        
        // For void methods, if the call succeeded, we're done
        if (success) {
            delete replica;
            qDebug() << "LogosAPI: Successfully called void method" << methodName << "on object" << objectName;
            return QVariant(true); // Return true to indicate success
        }
    } else if (args.size() == 4) {
        success = QMetaObject::invokeMethod(
            replica,
            methodName.toUtf8().constData(),
            Qt::DirectConnection,
            Q_RETURN_ARG(QRemoteObjectPendingCall, pendingCall),
            createArgument(args[0]),
            createArgument(args[1]),
            createArgument(args[2]),
            createArgument(args[3])
        );
    } else if (args.size() == 5) {
        success = QMetaObject::invokeMethod(
            replica,
            methodName.toUtf8().constData(),
            Qt::DirectConnection,
            Q_RETURN_ARG(QRemoteObjectPendingCall, pendingCall),
            createArgument(args[0]),
            createArgument(args[1]),
            createArgument(args[2]),
            createArgument(args[3]),
            createArgument(args[4])
        );
    } else {
        qWarning() << "LogosAPI: Currently supports 0-5 arguments. Got:" << args.size();
        delete replica;
        return QVariant();
    }

    if (!success) {
        qWarning() << "LogosAPI: Failed to invoke method" << methodName << "on object" << objectName;
        delete replica;
        return QVariant();
    }

    // Wait for the result
    pendingCall.waitForFinished(timeoutMs);
    if (!pendingCall.isFinished() || pendingCall.error() != QRemoteObjectPendingCall::NoError) {
        qWarning() << "LogosAPI: Remote call failed or timed out:" << pendingCall.error();
        qDebug() << "LogosAPI: Remote call failed or timed out at" << QTime::currentTime().toString("hh:mm:ss.zzz");
        qWarning() << "LogosAPI: Failed to invoke method" << methodName << "on object" << objectName;
        delete replica;
        return QVariant();
    }

    QVariant result = pendingCall.returnValue();
    delete replica;

    qDebug() << "LogosAPI: Successfully called method" << methodName << "on object" << objectName;
    return result;
}

QVariant LogosAPI::callRemoteMethod(const QString& objectName, const QString& methodName, 
                                   const QVariant& arg, int timeoutMs)
{
    // Simply delegate to the main method with a single-item list
    return callRemoteMethod(objectName, methodName, QVariantList() << arg, timeoutMs);
}

QVariant LogosAPI::callRemoteMethod(const QString& objectName, const QString& methodName, 
                                   const QVariant& arg1, const QVariant& arg2, int timeoutMs)
{
    // Delegate to the main method with a two-item list
    return callRemoteMethod(objectName, methodName, QVariantList() << arg1 << arg2, timeoutMs);
}

QVariant LogosAPI::callRemoteMethod(const QString& objectName, const QString& methodName, 
                                   const QVariant& arg1, const QVariant& arg2, const QVariant& arg3, int timeoutMs)
{
    // Delegate to the main method with a three-item list
    return callRemoteMethod(objectName, methodName, QVariantList() << arg1 << arg2 << arg3, timeoutMs);
}

QVariant LogosAPI::callRemoteMethod(const QString& objectName, const QString& methodName, 
                                   const QVariant& arg1, const QVariant& arg2, const QVariant& arg3, 
                                   const QVariant& arg4, int timeoutMs)
{
    // Delegate to the main method with a four-item list
    return callRemoteMethod(objectName, methodName, QVariantList() << arg1 << arg2 << arg3 << arg4, timeoutMs);
}

QVariant LogosAPI::callRemoteMethod(const QString& objectName, const QString& methodName, 
                                   const QVariant& arg1, const QVariant& arg2, const QVariant& arg3, 
                                   const QVariant& arg4, const QVariant& arg5, int timeoutMs)
{
    // Delegate to the main method with a five-item list
    return callRemoteMethod(objectName, methodName, QVariantList() << arg1 << arg2 << arg3 << arg4 << arg5, timeoutMs);
}

// change from objectName to instead, originObject and destinationObject, and use QObject instead
void LogosAPI::onEvent(QObject* originObject, QObject* destinationObject, const QString& eventName, std::function<void(const QString&, const QVariantList&)> callback)
{
    qDebug() << "LogosAPI: Registering event listener for event:" << eventName;

    // Store the callback for this event name
    m_eventCallbacks[eventName].append(callback);

    // Create connection key for this origin/destination pair
    // TODO: probably doesn't need destinationObject actually
    ConnectionKey connKey = {originObject, destinationObject};
    
    // Check if we already have a connection for this origin/destination pair
    if (!m_connections.contains(connKey)) {
        // Create new connection only if it doesn't exist
        auto connection = QObject::connect(originObject, SIGNAL(eventResponse(QString, QVariantList)), 
                                          this, SLOT(invokeCallback(QString, QVariantList)));
        
        if (connection) {
            m_connections[connKey] = connection;
            qDebug() << "LogosAPI: Created new connection for origin/destination pair";
        } else {
            qWarning() << "LogosAPI: Failed to create connection for event:" << eventName;
        }
    } else {
        qDebug() << "LogosAPI: Reusing existing connection for origin/destination pair";
    }
    
    qDebug() << "LogosAPI: Registered callback for event:" << eventName;
}

void LogosAPI::onEventResponse(QObject* replica, const QString& eventName, const QVariantList& data)
{
    qDebug() << "LogosAPI: Received event:" << eventName << "with data:" << data;

    if (eventName.isEmpty()) {
        qWarning() << "LogosAPI: Event name cannot be empty";
        return;
    }

    qDebug() << "LogosAPI: Emitting event:" << eventName << "with data:" << data;

    // emit the eventResponse signal of replica
    QMetaObject::invokeMethod(replica, "eventResponse", Qt::QueuedConnection, Q_ARG(QString, eventName), Q_ARG(QVariantList, data));
    // QMetaObject::invokeMethod(replica, "eventResponse_another", Qt::QueuedConnection, Q_ARG(QString, eventName), Q_ARG(QVariantList, data));
    // TODO: try queued connection instead
    // QMetaObject::invokeMethod(replica, "eventResponse_another", Qt::DirectConnection, Q_ARG(QString, eventName), Q_ARG(QVariantList, data));
}

void LogosAPI::invokeCallback(const QString& eventName, const QVariantList& data)
{
    qDebug() << "LogosAPI: invokeCallback called for event:" << eventName;
    
    // Call all registered callbacks
    // Note: This will call all callbacks for any event. In a more sophisticated implementation,
    // you might want to store event names with callbacks to filter them.
    for (const auto& callback : m_eventCallbacks[eventName]) {
        try {
            callback(eventName, data);
        } catch (...) {
            qWarning() << "LogosAPI: Exception in callback for event:" << eventName;
        }
    }
    
    qDebug() << "LogosAPI: Called" << m_eventCallbacks[eventName].size() << "callbacks for event:" << eventName;
}

void LogosAPI::onEvent(QObject* originObject, QObject* destinationObject, const QString& eventName)
{
    qDebug() << "LogosAPI: Registering event listener for event:" << eventName << "(connecting to destination slot)";

    // connect to the eventResponse signal of the destinationObject's slot
    QObject::connect(originObject, SIGNAL(eventResponse(QString, QVariantList)), 
                    destinationObject, SLOT(onEventResponse(QString, QVariantList)), Qt::AutoConnection);
}

// Include MOC for template instantiation
#include "moc_logos_api.cpp" 
