#ifndef MODULE_PROXY_H
#define MODULE_PROXY_H

#include <QObject>
#include <QVariant>
#include <QVariantList>
#include <QUuid>
#include <QHash>
#include <QMetaObject>

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
     * @brief Construct a new ModuleProxy
     * @param module The module object to proxy
     * @param parent Parent QObject
     */
    explicit ModuleProxy(QObject* module, QObject* parent = nullptr);

    /**
     * @brief Destructor
     */
    ~ModuleProxy();

    /**
     * @brief Call a method on the proxied module
     * @param methodName The name of the method to call
     * @param args Arguments to pass to the method
     * @return QVariant containing the result, or invalid QVariant if failed
     */
    Q_INVOKABLE QVariant callRemoteMethod(const QString& methodName, const QVariantList& args = QVariantList());

signals:
    void eventResponse(const QString& eventName, const QVariantList& data);

private:
    QObject* m_module;
};

#endif // MODULE_PROXY_H
