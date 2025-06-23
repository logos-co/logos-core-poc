#include "module_proxy.h"
#include <QDebug>
#include <QDateTime>
#include <QVariantList>
#include <QMetaObject>
#include <QJsonArray>
#include <QJsonDocument>

ModuleProxy::ModuleProxy(QObject* wrappedObject, QObject *parent)
    : QObject(parent)
    , m_wrappedObject(wrappedObject)
{
    // Connect to the wrapped object's eventResponse signal to forward events
    if (m_wrappedObject) {
       QObject::connect(m_wrappedObject, SIGNAL(eventResponse(QString, QVariantList)),
                       this, SIGNAL(eventResponse(QString, QVariantList)));
       qDebug() << "ModuleProxy: Connected to wrapped object's eventResponse signal";
    }
}

QObject* ModuleProxy::getWrappedObject() const
{
    return m_wrappedObject;
}

// Helper function to create QGenericArgument from QVariant
auto ModuleProxy::createArgument(const QVariant& variant)
{
    switch (variant.type()) {
        case QVariant::String: {
            m_stringArgs.append(variant.toString());
            return Q_ARG(QString, m_stringArgs.last());
        }
        default: {
            // For now, convert everything else to string as fallback
            m_stringArgs.append(variant.toString());
            return Q_ARG(QString, m_stringArgs.last());
        }
    }
}

// Helper function to create QGenericReturnArgument for different return types
template<typename T>
auto ModuleProxy::createReturnArgument(T& returnValue)
{
    return Q_RETURN_ARG(T, returnValue);
}

QVariant ModuleProxy::callObjectMethod(const QString& objectName, const QString& methodName, 
                                     const QVariantList& args)
{
    qDebug() << "ModuleProxy: Calling method" << methodName << "on wrapped object";

    if (objectName.isEmpty() || methodName.isEmpty()) {
        qWarning() << "ModuleProxy: Object name and method name cannot be empty";
        return QVariant();
    }

    if (!m_wrappedObject) {
        qWarning() << "ModuleProxy: Wrapped object is null";
        return QVariant();
    }

    qDebug() << "ModuleProxy: Calling method" << methodName << "on wrapped object";

    // Clear string storage before each call
    m_stringArgs.clear();

    bool success = false;
    QVariant returnValue;
    
    // Local return value storage to avoid race conditions
    QJsonArray jsonArrayResult;
    bool boolResult = false;
    QVariant variantResult;

    // QVariant jsonArrayResult;
    // QVariant boolResult;

    // Special handling for package_manager getPackages method
    if (objectName == "package_manager" && methodName == "getPackages") {
        qDebug() << "ModuleProxy: Special handling for package_manager getPackages - returning QJsonArray";
        
        if (args.isEmpty()) {
            success = QMetaObject::invokeMethod(
                m_wrappedObject,
                methodName.toUtf8().constData(),
                Qt::DirectConnection,
                createReturnArgument(jsonArrayResult)
            );
        } else if (args.size() == 1) {
            success = QMetaObject::invokeMethod(
                m_wrappedObject,
                methodName.toUtf8().constData(),
                Qt::DirectConnection,
                createReturnArgument(jsonArrayResult),
                createArgument(args[0])
            );
        }
        
        if (success) {
            qDebug() << "ModuleProxy: Successfully called getPackages, returning QJsonArray with" << jsonArrayResult.size() << "elements";
            return QVariant::fromValue(jsonArrayResult);
        } else {
            qWarning() << "ModuleProxy: Failed to call getPackages on package_manager";
            return QVariant::fromValue(QJsonArray());
        }
    }
    // For methods that return bool (like foo), we need to capture as bool first
    else if (methodName == "foo") {
        if (args.isEmpty()) {
            success = QMetaObject::invokeMethod(
                m_wrappedObject,
                methodName.toUtf8().constData(),
                Qt::DirectConnection,
                createReturnArgument(boolResult)
            );
        } else if (args.size() == 1) {
            success = QMetaObject::invokeMethod(
                m_wrappedObject,
                methodName.toUtf8().constData(),
                Qt::DirectConnection,
                createReturnArgument(boolResult),
                createArgument(args[0])
            );
        }
        if (success) {
            returnValue = QVariant(boolResult);
        }
    } else {
        // Handle different argument counts - call the wrapped object directly
        if (args.isEmpty()) {
            success = QMetaObject::invokeMethod(
                m_wrappedObject,
                methodName.toUtf8().constData(),
                Qt::DirectConnection,
                createReturnArgument(variantResult)
            );
        } else if (args.size() == 1) {
            success = QMetaObject::invokeMethod(
                m_wrappedObject,
                methodName.toUtf8().constData(),
                Qt::DirectConnection,
                createReturnArgument(variantResult),
                createArgument(args[0])
            );
        } else if (args.size() == 2) {
            success = QMetaObject::invokeMethod(
                m_wrappedObject,
                methodName.toUtf8().constData(),
                Qt::DirectConnection,
                createReturnArgument(variantResult),
                createArgument(args[0]),
                createArgument(args[1])
            );
        } else if (args.size() == 3) {
            success = QMetaObject::invokeMethod(
                m_wrappedObject,
                methodName.toUtf8().constData(),
                Qt::DirectConnection,
                createReturnArgument(variantResult),
                createArgument(args[0]),
                createArgument(args[1]),
                createArgument(args[2])
            );
        } else if (args.size() == 4) {
            success = QMetaObject::invokeMethod(
                m_wrappedObject,
                methodName.toUtf8().constData(),
                Qt::DirectConnection,
                createReturnArgument(variantResult),
                createArgument(args[0]),
                createArgument(args[1]),
                createArgument(args[2]),
                createArgument(args[3])
            );
        } else if (args.size() == 5) {
            success = QMetaObject::invokeMethod(
                m_wrappedObject,
                methodName.toUtf8().constData(),
                Qt::DirectConnection,
                createReturnArgument(variantResult),
                createArgument(args[0]),
                createArgument(args[1]),
                createArgument(args[2]),
                createArgument(args[3]),
                createArgument(args[4])
            );
        } else {
            qWarning() << "ModuleProxy: Currently supports 0-5 arguments. Got:" << args.size();
            return QVariant();
        }
        
        if (success) {
            returnValue = variantResult;
        }
    }

    if (success) {
        qDebug() << "ModuleProxy: Successfully called method" << methodName << "on wrapped object";
        qDebug() << "ModuleProxy: Method returned:" << returnValue;
        return returnValue;
    } else {
        qWarning() << "ModuleProxy: Failed to invoke method" << methodName << "on wrapped object";
        return QVariant();
    }
}

// QJsonArray ModuleProxy::callObjectMethodJson(const QString& objectName, const QString& methodName, 
//                                             const QVariantList& args)
// {
//     qDebug() << "ModuleProxy: Calling JSON method" << methodName << "on wrapped object";
// 
//     if (objectName.isEmpty() || methodName.isEmpty()) {
//         qWarning() << "ModuleProxy: Object name and method name cannot be empty";
//         return QJsonArray();
//     }
// 
//     if (!m_wrappedObject) {
//         qWarning() << "ModuleProxy: Wrapped object is null";
//         return QJsonArray();
//     }
// 
//     qDebug() << "ModuleProxy: Calling JSON method" << methodName << "on wrapped object";
// 
//     // Clear string storage before each call
//     m_stringArgs.clear();
// 
//     bool success = false;
//     QJsonArray returnValue;
// 
//     // Handle different argument counts - call the wrapped object directly
//     if (args.isEmpty()) {
//         success = QMetaObject::invokeMethod(
//             m_wrappedObject,
//             methodName.toUtf8().constData(),
//             Qt::DirectConnection,
//             Q_RETURN_ARG(QJsonArray, returnValue)
//         );
//     } else if (args.size() == 1) {
//         success = QMetaObject::invokeMethod(
//             m_wrappedObject,
//             methodName.toUtf8().constData(),
//             Qt::DirectConnection,
//             Q_RETURN_ARG(QJsonArray, returnValue),
//             createArgument(args[0])
//         );
//     } else if (args.size() == 2) {
//         success = QMetaObject::invokeMethod(
//             m_wrappedObject,
//             methodName.toUtf8().constData(),
//             Qt::DirectConnection,
//             Q_RETURN_ARG(QJsonArray, returnValue),
//             createArgument(args[0]),
//             createArgument(args[1])
//         );
//     } else if (args.size() == 3) {
//         success = QMetaObject::invokeMethod(
//             m_wrappedObject,
//             methodName.toUtf8().constData(),
//             Qt::DirectConnection,
//             Q_RETURN_ARG(QJsonArray, returnValue),
//             createArgument(args[0]),
//             createArgument(args[1]),
//             createArgument(args[2])
//         );
//     } else if (args.size() == 4) {
//         success = QMetaObject::invokeMethod(
//             m_wrappedObject,
//             methodName.toUtf8().constData(),
//             Qt::DirectConnection,
//             Q_RETURN_ARG(QJsonArray, returnValue),
//             createArgument(args[0]),
//             createArgument(args[1]),
//             createArgument(args[2]),
//             createArgument(args[3])
//         );
//     } else if (args.size() == 5) {
//         success = QMetaObject::invokeMethod(
//             m_wrappedObject,
//             methodName.toUtf8().constData(),
//             Qt::DirectConnection,
//             Q_RETURN_ARG(QJsonArray, returnValue),
//             createArgument(args[0]),
//             createArgument(args[1]),
//             createArgument(args[2]),
//             createArgument(args[3]),
//             createArgument(args[4])
//         );
//     } else {
//         qWarning() << "ModuleProxy: Currently supports 0-5 arguments. Got:" << args.size();
//         return QJsonArray();
//     }
// 
//     if (success) {
//         qDebug() << "ModuleProxy: Successfully called JSON method" << methodName << "on wrapped object";
//         qDebug() << "ModuleProxy: JSON method returned array with" << returnValue.size() << "elements";
//         return returnValue;
//     } else {
//         qWarning() << "ModuleProxy: Failed to invoke JSON method" << methodName << "on wrapped object";
//         return QJsonArray();
//     }
// }

// bool ModuleProxy::foo(const QString &bar)
// {
//    qDebug() << "TemplateModulePlugin::foo called with:" << bar;
//    
//    // Create event data with the bar parameter
//    QVariantList eventData;
//    eventData << bar; // Add the bar parameter to the event data
//    eventData << QDateTime::currentDateTime().toString(Qt::ISODate); // Add timestamp
//    
//    // Trigger the event by emitting the signal directly
//    qDebug() << "TemplateModulePlugin: Triggering event 'fooTriggered' with data:" << eventData;
//    emit eventResponse("fooTriggered", eventData);
//    qDebug() << "TemplateModulePlugin: Event 'fooTriggered' triggered with data:" << eventData;
//    
//    return true;
// } 
