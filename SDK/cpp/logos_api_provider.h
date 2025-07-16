#ifndef LOGOS_API_PROVIDER_H
#define LOGOS_API_PROVIDER_H

#include <QObject>
#include <QString>
#include <QVariant>
#include <QVariantList>
#include <QMap>

class QRemoteObjectRegistryHost;

/**
 * @brief LogosAPIProvider handles registering objects for remote access
 * 
 * This class is responsible for the provider/server side functionality:
 * - Creating registry hosts
 * - Registering objects for remote access
 * - Handling event responses
 */
class LogosAPIProvider : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief Construct a new LogosAPIProvider
     * @param module_name The name of this module
     * @param parent Parent QObject
     */
    explicit LogosAPIProvider(const QString& module_name, QObject *parent = nullptr);
    
    /**
     * @brief Destructor - cleans up registry host
     */
    ~LogosAPIProvider();

    /**
     * @brief Register an object to be available for remote access
     * @param name The name to register the object under
     * @param object The object to register
     * @param authToken Authentication token for the object
     * @return true if registration successful, false otherwise
     */
    bool registerObject(const QString& name, QObject* object, const QString& authToken);

    /**
     * @brief Get the registry URL for this provider
     * @return QString containing the registry URL
     */
    QString registryUrl() const;

public slots:
    /**
     * @brief Handle event responses from objects
     * @param replica The replica object that should receive the event
     * @param eventName The name of the event
     * @param data The event data
     */
    void onEventResponse(QObject* replica, const QString& eventName, const QVariantList& data);

private:
    QRemoteObjectRegistryHost* m_registryHost;
    QString m_registryUrl;
    QMap<QString, QString> m_tokens;


};

#endif // LOGOS_API_PROVIDER_H 