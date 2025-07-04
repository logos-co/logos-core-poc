#ifndef LOGOS_API_PROVIDER_H
#define LOGOS_API_PROVIDER_H

#include <QObject>
#include <QString>
#include <QUrl>
#include <QVariant>
#include <QVariantList>
#include <QDebug>

class QRemoteObjectRegistryHost;
class ModuleProxy;

/**
 * @brief LogosAPIProvider provides functionality for registering objects 
 * and handling events in the Logos Core system.
 * 
 * This class handles the provider/server side of the Qt Remote Objects
 * functionality, allowing modules to register their services and handle events.
 */
class LogosAPIProvider : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief Construct a new LogosAPIProvider
     * @param module_name The name of the module (default: "core_registry")
     * @param parent Parent QObject
     */
    explicit LogosAPIProvider(const QString& module_name = "core_registry", QObject *parent = nullptr);
    
    /**
     * @brief Destructor - cleans up resources
     */
    ~LogosAPIProvider();

    /**
     * @brief Register an object with the remote object registry
     * @param name The name to register the object under
     * @param object The object to register
     * @param authToken Authentication token for this object
     * @return true if registration succeeded, false otherwise
     */
    bool registerObject(const QString& name, QObject* object, const QString& authToken);

    /**
     * @brief Get the registry URL this provider is using
     * @return QString containing the registry URL
     */
    QString registryUrl() const;

public slots:
    /**
     * @brief Handle incoming event responses and trigger registered callbacks
     * @param replica The replica object that should receive the event
     * @param eventName The name of the event that was triggered
     * @param data The event data to pass to the callbacks
     * 
     * This slot is typically connected to signals from remote objects to handle
     * events and notifications from the Logos Core system.
     */
    void onEventResponse(QObject* replica, const QString& eventName, const QVariantList& data);

private:
    QRemoteObjectRegistryHost* m_registryHost;
    QString m_registryUrl;
};

#endif // LOGOS_API_PROVIDER_H 