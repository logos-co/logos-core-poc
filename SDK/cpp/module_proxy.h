#ifndef MODULE_PROXY_H
#define MODULE_PROXY_H

#include <QObject>
#include <QVariantList>
#include <QGenericArgument>
#include <QJsonArray>

class ModuleProxy : public QObject
{
    Q_OBJECT

public:
    explicit ModuleProxy(QObject* wrappedObject, QObject *parent = nullptr);

    // Q_INVOKABLE bool foo(const QString &bar);
    
    QObject* getWrappedObject() const;
    
    // Q_INVOKABLE bool foo(const QString &bar);
    
    Q_INVOKABLE QVariant callObjectMethod(const QString& objectName, const QString& methodName, 
                                          const QVariantList& args);
    
    // Q_INVOKABLE QJsonArray callObjectMethodJson(const QString& objectName, const QString& methodName, 
    //                                             const QVariantList& args);

signals:
    void eventResponse(const QString& eventName, const QVariantList& data);

private:
    // Helper function to create QGenericArgument from QVariant
    auto createArgument(const QVariant& variant);
    
    // Helper function to create QGenericReturnArgument for different return types
    template<typename T>
    auto createReturnArgument(T& returnValue);
    
    QObject* m_wrappedObject;
    QStringList m_stringArgs; // Storage for string arguments
};

#endif // MODULE_PROXY_H 