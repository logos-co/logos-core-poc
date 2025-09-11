#ifndef MODULE_PROXY_H
#define MODULE_PROXY_H

#include <QObject>
#include <QVariant>
#include <QVariantList>
#include <QUuid>
#include <QHash>
#include <QMetaObject>
#include <QString>
#include <QJsonArray>

/**
 * @brief ModuleProxy provides a proxy interface for module interactions
 *
 * This class serves as a proxy layer for communicating with modules
 * in the Logos Core system.
 */
class ModuleProxy : public QObject
{
    Q_OBJECT

public:

    /**
     * @brief Construct a new ModuleProxy with authentication token
     * @param module The module object to proxy
     * @param authToken Authentication token for the module
     * @param parent Parent QObject
     */
    explicit ModuleProxy(QObject* module, QObject* parent = nullptr);

    /**
     * @brief Destructor
     */
    ~ModuleProxy();

    /**
     * @brief Call a method on the proxied module
     * @param authToken Authentication token for the method call
     * @param methodName The name of the method to call
     * @param args Arguments to pass to the method
     * @return QVariant containing the result, or invalid QVariant if failed
     */
    Q_INVOKABLE QVariant callRemoteMethod(const QString& authToken, const QString& methodName, const QVariantList& args = QVariantList());

    /**
     * @brief Inform module of a token
     * @param authToken Authentication token for the operation
     * @param moduleName The name of the module
     * @param token The token to inform the module about
     * @return bool true if successful, false otherwise
     */
    Q_INVOKABLE bool informModuleToken(const QString& authToken, const QString& moduleName, const QString& token);

    /**
     * @brief Save a token from a module
     * @param from_module_name The name of the module providing the token
     * @param token The token to save
     * @return bool true if token was saved successfully, false otherwise
     */
    bool saveToken(const QString& from_module_name, const QString& token);

    /**
     * @brief Get a list of methods for the encapsulated module
     * @return QJsonArray of method metadata (name, signature, returnType, parameters)
     */
    Q_INVOKABLE QJsonArray getPluginMethods();

signals:
    void eventResponse(const QString& eventName, const QVariantList& data);

private:
    QObject* m_module;
    QHash<QString, QString> m_tokens;
};

#endif // MODULE_PROXY_H
