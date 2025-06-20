#include "module_proxy.h"
#include <QDebug>
#include <QDateTime>
#include <QVariantList>
#include <QMetaObject>

ModuleProxy::ModuleProxy(QObject* wrappedObject, QObject *parent)
    : QObject(parent)
    , m_wrappedObject(wrappedObject)
{
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

bool ModuleProxy::callObjectMethod(const QString& objectName, const QString& methodName, 
                                  const QVariantList& args)
{
    qDebug() << "ModuleProxy: Calling method" << methodName << "on wrapped object";

    if (objectName.isEmpty() || methodName.isEmpty()) {
        qWarning() << "ModuleProxy: Object name and method name cannot be empty";
        return false;
    }

    if (!m_wrappedObject) {
        qWarning() << "ModuleProxy: Wrapped object is null";
        return false;
    }

    qDebug() << "ModuleProxy: Calling method" << methodName << "on wrapped object";

    // Clear string storage before each call
    m_stringArgs.clear();

    bool success = false;

    // Handle different argument counts - call the wrapped object directly
    if (args.isEmpty()) {
        success = QMetaObject::invokeMethod(
            m_wrappedObject,
            methodName.toUtf8().constData(),
            Qt::DirectConnection
        );
    } else if (args.size() == 1) {
        success = QMetaObject::invokeMethod(
            m_wrappedObject,
            methodName.toUtf8().constData(),
            Qt::DirectConnection,
            createArgument(args[0])
        );
    } else if (args.size() == 2) {
        success = QMetaObject::invokeMethod(
            m_wrappedObject,
            methodName.toUtf8().constData(),
            Qt::DirectConnection,
            createArgument(args[0]),
            createArgument(args[1])
        );
    } else if (args.size() == 3) {
        success = QMetaObject::invokeMethod(
            m_wrappedObject,
            methodName.toUtf8().constData(),
            Qt::DirectConnection,
            createArgument(args[0]),
            createArgument(args[1]),
            createArgument(args[2])
        );
    } else if (args.size() == 4) {
        success = QMetaObject::invokeMethod(
            m_wrappedObject,
            methodName.toUtf8().constData(),
            Qt::DirectConnection,
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
            createArgument(args[0]),
            createArgument(args[1]),
            createArgument(args[2]),
            createArgument(args[3]),
            createArgument(args[4])
        );
    } else {
        qWarning() << "ModuleProxy: Currently supports 0-5 arguments. Got:" << args.size();
        return false;
    }

    if (success) {
        qDebug() << "ModuleProxy: Successfully called method" << methodName << "on wrapped object";
    } else {
        qWarning() << "ModuleProxy: Failed to invoke method" << methodName << "on wrapped object";
    }

    return success;
}

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
