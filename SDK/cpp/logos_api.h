#ifndef LOGOS_API_H
#define LOGOS_API_H

#include <QObject>
#include <QString>
#include <QUrl>

class QRemoteObjectNode;
class QRemoteObjectReplica;

/**
 * @brief LogosAPI provides a simplified interface for connecting to 
 * and acquiring remote objects from the Logos Core registry.
 * 
 * This class abstracts the Qt Remote Objects functionality, making it easier
 * to connect to the core registry and request remote object replicas by name.
 */
class LogosAPI : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief Construct a new LogosAPI
     * @param registryUrl The URL of the remote registry (default: "local:logoscore_registry")
     * @param parent Parent QObject
     */
    explicit LogosAPI(const QString& registryUrl = "local:logoscore_registry", QObject *parent = nullptr);
    
    /**
     * @brief Destructor - cleans up the remote object node
     */
    ~LogosAPI();

    /**
     * @brief Request a remote object replica by name
     * @param objectName The name of the remote object to acquire
     * @param timeoutMs Timeout in milliseconds to wait for the replica to be ready (default: 5000)
     * @return QRemoteObjectReplica* pointer to the replica, or nullptr if failed
     * 
     * @note The caller is responsible for deleting the returned replica when done
     */
    QRemoteObjectReplica* requestObject(const QString& objectName, int timeoutMs = 5000);

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

private:
    QRemoteObjectNode* m_node;
    QString m_registryUrl;
    bool m_connected;

    /**
     * @brief Internal method to establish connection to the registry
     * @return true if connection successful, false otherwise
     */
    bool connectToRegistry();
};

#endif // LOGOS_API_H 