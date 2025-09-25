#ifndef LOGOS_API_CONSUMER_H
#define LOGOS_API_CONSUMER_H

#include <QObject>
#include <QString>
#include <QVariant>
#include <QVariantList>
#include <QHash>
#include <QMap>
#include <functional>

class QRemoteObjectNode;
class TokenManager;

/**
 * @brief LogosAPIConsumer handles connecting to remote objects and invoking their methods
 * 
 * This class is responsible for the consumer/client side functionality:
 * - Connecting to remote object registries
 * - Requesting remote object replicas
 * - Invoking methods on remote objects
 * - Handling events from remote objects
 */
class LogosAPIConsumer : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief Construct a new LogosAPIConsumer
     * @param module_to_talk_to The name of the module to connect to
     * @param origin_module The name of the originating module
     * @param token_manager Pointer to the token manager instance
     * @param parent Parent QObject
     */
    explicit LogosAPIConsumer(const QString& module_to_talk_to, const QString& origin_module, TokenManager* token_manager, QObject *parent = nullptr);
    
    /**
     * @brief Destructor - cleans up connections and resources
     */
    ~LogosAPIConsumer();

    /**
     * @brief Request a remote object replica by name
     * @param objectName The name of the remote object to acquire
     * @param timeoutMs Timeout in milliseconds to wait for the replica to be ready
     * @return QObject* pointer to the replica, or nullptr if failed
     */
    QObject* requestObject(const QString& objectName, int timeoutMs = 20000);

    /**
     * @brief Check if the consumer is connected to the registry
     * @return true if connected, false otherwise
     */
    bool isConnected() const;

    /**
     * @brief Get the registry URL this consumer is connected to
     * @return QString containing the registry URL
     */
    QString registryUrl() const;

    /**
     * @brief Reconnect to the registry
     * @return true if reconnection successful, false otherwise
     */
    bool reconnect();

    /**
     * @brief Invoke a remote method on a remote object
     * @param authToken Authentication token for the operation
     * @param objectName The name of the remote object
     * @param methodName The name of the method to call
     * @param args Arguments to pass to the method
     * @param timeoutMs Timeout in milliseconds to wait for the result
     * @return QVariant containing the result, or invalid QVariant if failed
     */
    QVariant invokeRemoteMethod(const QString& authToken, const QString& objectName, const QString& methodName, 
                             const QVariantList& args = QVariantList(), int timeoutMs = 20000);

    /**
     * @brief Register an event listener for the specified event name
     * @param originObject The object that will emit the event
     * @param destinationObject The object that will receive the event
     * @param eventName The name of the event to listen for
     * @param callback Function to call when the event is triggered
     */
    void onEvent(QObject* originObject, QObject* destinationObject, const QString& eventName, 
                std::function<void(const QString&, const QVariantList&)> callback);
    
    /**
     * @brief Register an event listener without callback (connects to destinationObject's slot)
     * @param originObject The object that will emit the event
     * @param destinationObject The object that will receive the event
     * @param eventName The name of the event to listen for
     */
    void onEvent(QObject* originObject, QObject* destinationObject, const QString& eventName);

public slots:
    /**
     * @brief Helper slot to invoke stored callbacks
     * @param eventName The name of the event that was triggered
     * @param data The event data to pass to the callback
     */
    void invokeCallback(const QString& eventName, const QVariantList& data);

    /**
     * @brief Inform a module about a token
     * @param authToken Authentication token for the operation
     * @param moduleName The name of the module
     * @param token The token to inform the module about
     * @return bool true if successful, false otherwise
     */
    bool informModuleToken(const QString& authToken, const QString& moduleName, const QString& token);

    bool informModuleToken_module(const QString& authToken, const QString& originModule, const QString& moduleName, const QString& token);

private:
    QRemoteObjectNode* m_node;
    QString m_registryUrl;
    bool m_connected;
    QMap<QString, QString> m_tokens;
    TokenManager* m_token_manager;

    // Store callbacks by event name
    QHash<QString, QList<std::function<void(const QString&, const QVariantList&)>>> m_eventCallbacks;
    
    // Track existing connections by origin object to avoid duplicates
    QHash<QObject*, QMetaObject::Connection> m_connections;

    /**
     * @brief Internal method to establish connection to the registry
     * @return true if connection successful, false otherwise
     */
    bool connectToRegistry();


};

#endif // LOGOS_API_CONSUMER_H 