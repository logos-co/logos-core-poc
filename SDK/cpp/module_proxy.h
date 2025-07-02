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
     * @param authToken Authentication token for secure access
     * @param methodName The name of the method to call
     * @param args Arguments to pass to the method
     * @return QVariant containing the result, or invalid QVariant if failed
     */
    Q_INVOKABLE QVariant callRemoteMethod(const QString& authToken, const QString& methodName, const QVariantList& args = QVariantList());

signals:
    void eventResponse(const QString& eventName, const QVariantList& data);

private:
    QObject* m_module;
    QString m_authToken;
    
    // Storage for string arguments to keep them alive during method calls
    // Key: GUID for method call, Value: list of string arguments for that call
    // This is currently quite an ugly hack, need to review
    mutable QHash<QString, QList<QString>> m_stringArgsStorage;
    
    // Storage for int arguments to keep them alive during method calls
    // This is currently quite an ugly hack, need to review
    mutable QHash<QString, QList<int>> m_intArgsStorage;

    /**
     * @brief Helper function to create QGenericArgument from QVariant
     * @param variant The QVariant to convert
     * @return QGenericArgument that can be used with QMetaObject::invokeMethod
     * 
     * @note Each call generates its own unique GUID for isolated argument storage
     */
    auto createArgument(const QVariant& variant);

    /**
     * @brief Helper method to invoke methods with different return types and argument counts
     * @param methodName The name of the method to call
     * @param args Arguments to pass to the method
     * @param returnValue Pointer to store the return value (nullptr for void methods)
     * @param returnTypeName String name of the return type ("bool", "int", "QString", "QVariant", etc.)
     * @return true if the method call succeeded, false otherwise
     */
    bool invokeMethodByArgCount(const QString& methodName, const QVariantList& args, void* returnValue, const char* returnTypeName);
};

#endif // MODULE_PROXY_H 