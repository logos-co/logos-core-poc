#ifndef LOGOS_API_CLIENT_H
#define LOGOS_API_CLIENT_H

#include <QObject>
#include <QString>
#include <QVariant>
#include <QVariantList>
#include <QMap>
#include <functional>

class LogosAPIConsumer;
class TokenManager;

/**
 * @brief LogosAPIClient provides a high-level interface for remote method calls
 * 
 * This class serves as a facade over LogosAPIConsumer, providing a clean interface
 * for applications that need to call remote methods and handle events. It includes
 * additional logic like token management and request routing.
 */
class LogosAPIClient : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief Construct a new LogosAPIClient
     * @param module_to_talk_to The name of the module to connect to
     * @param origin_module The name of the originating module
     * @param token_manager Pointer to the token manager instance
     * @param parent Parent QObject
     */
    explicit LogosAPIClient(const QString& module_to_talk_to, const QString& origin_module, TokenManager* token_manager, QObject *parent = nullptr);
    
    /**
     * @brief Destructor
     */
    ~LogosAPIClient();

    /**
     * @brief Request a remote object replica by name
     * @param objectName The name of the remote object to acquire
     * @param timeoutMs Timeout in milliseconds to wait for the replica to be ready
     * @return QObject* pointer to the replica, or nullptr if failed
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
     * @brief Reconnect to the registry
     * @return true if reconnection successful, false otherwise
     */
    bool reconnect();

    /**
     * @brief Invoke a remote method on a remote object
     * @param objectName The name of the remote object
     * @param methodName The name of the method to call
     * @param args Arguments to pass to the method
     * @param timeoutMs Timeout in milliseconds to wait for the result
     * @return QVariant containing the result, or invalid QVariant if failed
     */
    QVariant invokeRemoteMethod(const QString& objectName, const QString& methodName, 
                             const QVariantList& args = QVariantList(), int timeoutMs = 20000);

    /**
     * @brief Invoke a remote method on a remote object with a single argument
     * @param objectName The name of the remote object
     * @param methodName The name of the method to call
     * @param arg Argument to pass to the method
     * @param timeoutMs Timeout in milliseconds to wait for the result
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
     * @param timeoutMs Timeout in milliseconds to wait for the result
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
     * @param timeoutMs Timeout in milliseconds to wait for the result
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
     * @param timeoutMs Timeout in milliseconds to wait for the result
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
     * @param timeoutMs Timeout in milliseconds to wait for the result
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



    /**
     * @brief Emit an event response (for plugins that also act as event sources)
     * @param replica The replica object that should receive the event
     * @param eventName The name of the event
     * @param data The event data
     */
    void onEventResponse(QObject* replica, const QString& eventName, const QVariantList& data);

    /**
     * @brief Inform a module about a token
     * @param authToken Authentication token for the operation
     * @param moduleName The name of the module
     * @param token The token to inform the module about
     * @return bool true if successful, false otherwise
     */
    bool informModuleToken(const QString& authToken, const QString& moduleName, const QString& token);

    bool informModuleToken_module(const QString& authToken, const QString& originModule, const QString& moduleName, const QString& token);

    /**
     * @brief Get the token manager instance
     * @return TokenManager* Pointer to the token manager
     */
    TokenManager* getTokenManager() const;

    /**
     * @brief Get authentication token for a module
     * @param module_name The module name to get token for
     * @return QString containing the token
     */
    QString getToken(const QString& module_name);

public slots:
    /**
     * @brief Helper slot to invoke stored callbacks
     * @param eventName The name of the event that was triggered
     * @param data The event data to pass to the callback
     */
    void invokeCallback(const QString& eventName, const QVariantList& data);

private:
    LogosAPIConsumer* m_consumer;
    QMap<QString, QString> m_tokens;
    TokenManager* m_token_manager;
    QString m_origin_module;
};

#endif // LOGOS_API_CLIENT_H 