#ifndef LOGOS_API_CLIENT_H
#define LOGOS_API_CLIENT_H

#include <QObject>
#include <QString>
#include <QUrl>
#include <QVariant>
#include <QVariantList>
#include <QList>
#include <QMap>
#include <QHash>
#include <QDebug>
#include <functional>

class QRemoteObjectNode;
class QRemoteObjectReplica;

/**
 * @brief LogosAPIClient provides a simplified interface for connecting to 
 * and acquiring remote objects from the Logos Core registry.
 * 
 * This class abstracts the Qt Remote Objects functionality, making it easier
 * to connect to the core registry and request remote object replicas by name.
 */
class LogosAPIClient : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief Construct a new LogosAPIClient
     * @param module_name The name of the module to connect to (default: "core_registry")
     * @param parent Parent QObject
     */
    explicit LogosAPIClient(const QString& module_name = "core_registry", QObject *parent = nullptr);
    
    /**
     * @brief Destructor - cleans up the remote object node
     */
    ~LogosAPIClient();

    /**
     * @brief Request a remote object replica by name
     * @param objectName The name of the remote object to acquire
     * @param timeoutMs Timeout in milliseconds to wait for the replica to be ready (default: 20000)
     * @return QObject* pointer to the replica, or nullptr if failed
     * 
     * @note The caller is responsible for deleting the returned replica when done
     */
    QObject* requestObject(const QString& objectName, int timeoutMs = 20000);

    /**
     * @brief Check if the client is connected to the registry
     * @return true if connected, false otherwise
     */
    bool isConnected() const;

    /**
     * @brief Get the registry URL this client is connected to
     * @return QString containing the registry URL
     */
    QString registryUrl() const;

    /**
     * @brief Reconnect to the registry (useful if connection was lost)
     * @return true if reconnection successful, false otherwise
     */
    bool reconnect();

    /**
     * @brief Invoke a remote method on a remote object
     * @param objectName The name of the remote object
     * @param methodName The name of the method to call
     * @param args Arguments to pass to the method (supports 0-5 arguments)
     * @param timeoutMs Timeout in milliseconds to wait for the result (default: 20000)
     * @return QVariant containing the result, or invalid QVariant if failed
     */
    QVariant invokeRemoteMethod(const QString& objectName, const QString& methodName, 
                             const QVariantList& args = QVariantList(), int timeoutMs = 20000);

    /**
     * @brief Invoke a remote method on a remote object with a single argument
     * @param objectName The name of the remote object
     * @param methodName The name of the method to call
     * @param arg Argument to pass to the method
     * @param timeoutMs Timeout in milliseconds to wait for the result (default: 20000)
     * @return QVariant containing the result, or invalid QVariant if failed
     */
    QVariant invokeRemoteMethod(const QString& objectName, const QString& methodName, 
                             const QVariant& arg, int timeoutMs = 20000);

    /**
     * @brief Invoke a remote method on a remote object with two arguments
     * @param objectName The name of the remote object
     * @param methodName The name of the method to call
     * @param arg1 First argument to pass to the method
     * @param arg2 Second argument to pass to the method
     * @param timeoutMs Timeout in milliseconds to wait for the result (default: 20000)
     * @return QVariant containing the result, or invalid QVariant if failed
     */
    QVariant invokeRemoteMethod(const QString& objectName, const QString& methodName, 
                             const QVariant& arg1, const QVariant& arg2, int timeoutMs = 20000);

    /**
     * @brief Invoke a remote method on a remote object with three arguments
     * @param objectName The name of the remote object
     * @param methodName The name of the method to call
     * @param arg1 First argument to pass to the method
     * @param arg2 Second argument to pass to the method
     * @param arg3 Third argument to pass to the method
     * @param timeoutMs Timeout in milliseconds to wait for the result (default: 20000)
     * @return QVariant containing the result, or invalid QVariant if failed
     */
    QVariant invokeRemoteMethod(const QString& objectName, const QString& methodName, 
                             const QVariant& arg1, const QVariant& arg2, const QVariant& arg3, int timeoutMs = 20000);

    /**
     * @brief Invoke a remote method on a remote object with four arguments
     * @param objectName The name of the remote object
     * @param methodName The name of the method to call
     * @param arg1 First argument to pass to the method
     * @param arg2 Second argument to pass to the method
     * @param arg3 Third argument to pass to the method
     * @param arg4 Fourth argument to pass to the method
     * @param timeoutMs Timeout in milliseconds to wait for the result (default: 20000)
     * @return QVariant containing the result, or invalid QVariant if failed
     */
    QVariant invokeRemoteMethod(const QString& objectName, const QString& methodName, 
                             const QVariant& arg1, const QVariant& arg2, const QVariant& arg3, 
                             const QVariant& arg4, int timeoutMs = 20000);

    /**
     * @brief Invoke a remote method on a remote object with five arguments
     * @param objectName The name of the remote object
     * @param methodName The name of the method to call
     * @param arg1 First argument to pass to the method
     * @param arg2 Second argument to pass to the method
     * @param arg3 Third argument to pass to the method
     * @param arg4 Fourth argument to pass to the method
     * @param arg5 Fifth argument to pass to the method
     * @param timeoutMs Timeout in milliseconds to wait for the result (default: 20000)
     * @return QVariant containing the result, or invalid QVariant if failed
     */
    QVariant invokeRemoteMethod(const QString& objectName, const QString& methodName, 
                             const QVariant& arg1, const QVariant& arg2, const QVariant& arg3, 
                             const QVariant& arg4, const QVariant& arg5, int timeoutMs = 20000);

    /**
     * @brief Register an event listener for the specified event name
     * @param originObject The object that will emit the event
     * @param destinationObject The object that will receive the event
     * @param eventName The name of the event to listen for
     * @param callback Function to call when the event is triggered, receives eventName and event data
     * 
     * Multiple listeners can be registered for the same event name.
     */
    void onEvent(QObject* originObject, QObject* destinationObject, const QString& eventName, std::function<void(const QString&, const QVariantList&)> callback);
    
    /**
     * @brief Register an event listener without callback (connects to destinationObject's slot)
     * @param originObject The object that will emit the event
     * @param destinationObject The object that will receive the event
     * @param eventName The name of the event to listen for
     */
    void onEvent(QObject* originObject, QObject* destinationObject, const QString& eventName);

    /**
     * @brief Save a token associated with a module name
     * @param to_module_name The module name to associate with the token
     * @param token The token string to save
     */
    void saveToken(const QString& to_module_name, const QString& token);

public slots:
    /**
     * @brief Helper slot to invoke stored callbacks
     * @param eventName The name of the event that was triggered
     * @param data The event data to pass to the callback
     */
    void invokeCallback(const QString& eventName, const QVariantList& data);

private:
    QRemoteObjectNode* m_node;
    QString m_registryUrl;
    bool m_connected;

    // Storage for string arguments to keep them alive during method calls
    mutable QList<QString> m_stringArgs;

    // Event listeners storage - maps event names to lists of callback functions
    QHash<QString, QList<std::function<void(const QVariantList&)>>> m_eventListeners;
    
    // Store callbacks by event name for the new callback-based approach
    QHash<QString, QList<std::function<void(const QString&, const QVariantList&)>>> m_eventCallbacks;
    
    // Track existing connections by origin object to avoid duplicates
    // Since we always connect to 'this' LogosAPIClient instance, we only need to track origin objects
    QHash<QObject*, QMetaObject::Connection> m_connections;

    // Storage for tokens by module name
    QHash<QString, QString> m_tokens;

    /**
     * @brief Internal method to establish connection to the registry
     * @return true if connection successful, false otherwise
     */
    bool connectToRegistry();
};

#endif // LOGOS_API_CLIENT_H 