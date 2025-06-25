#include "module_proxy.h"
#include <QDebug>
#include <QMetaObject>
#include <QMetaMethod>
#include <QMetaType>

ModuleProxy::ModuleProxy(QObject* module, QObject* parent)
    : QObject(parent)
    , m_module(module)
{
    qDebug() << "ModuleProxy: Created for module:" << module;
    // Connect to the wrapped object's eventResponse signal to forward events
    if (m_module) {
       QObject::connect(m_module, SIGNAL(eventResponse(QString, QVariantList)),
                       this, SIGNAL(eventResponse(QString, QVariantList)));
       qDebug() << "ModuleProxy: Connected to wrapped object's eventResponse signal";
    }
}

ModuleProxy::~ModuleProxy()
{
    qDebug() << "ModuleProxy: Destroyed for module:" << m_module;
}

// Helper function to create QGenericArgument from QVariant
auto ModuleProxy::createArgument(const QVariant& variant)
{
    switch (variant.type()) {
        case QVariant::String: {
            static thread_local QString localString;
            localString = variant.toString();
            qDebug() << "ModuleProxy: createArgument - creating QString arg with value:" << localString;
            return Q_ARG(const QString&, localString);
        }
        default: {
            // For now, convert everything else to string as fallback
            static thread_local QString localString;
            localString = variant.toString();
            qDebug() << "ModuleProxy: createArgument - converting to QString arg with value:" << localString;
            return Q_ARG(const QString&, localString);
        }
    }
}

// Helper method to invoke methods with different return types and argument counts
bool ModuleProxy::invokeMethodByArgCount(const QString& methodName, const QVariantList& args, void* returnValue, const char* returnTypeName)
{
    // Store the UTF-8 data to ensure it stays in scope
    QByteArray methodNameBytes = methodName.toUtf8();
    const char* methodNameCStr = methodNameBytes.constData();
    qDebug() << "ModuleProxy: invokeMethodByArgCount - method name:" << methodNameCStr;
    
    if (returnValue == nullptr) {
        // Void method - no return value
        switch (args.size()) {
            case 0:
                return QMetaObject::invokeMethod(m_module, methodNameCStr, Qt::DirectConnection);
            case 1:
                return QMetaObject::invokeMethod(m_module, methodNameCStr, Qt::DirectConnection, createArgument(args[0]));
            case 2:
                return QMetaObject::invokeMethod(m_module, methodNameCStr, Qt::DirectConnection, createArgument(args[0]), createArgument(args[1]));
            case 3:
                return QMetaObject::invokeMethod(m_module, methodNameCStr, Qt::DirectConnection, createArgument(args[0]), createArgument(args[1]), createArgument(args[2]));
            case 4:
                return QMetaObject::invokeMethod(m_module, methodNameCStr, Qt::DirectConnection, createArgument(args[0]), createArgument(args[1]), createArgument(args[2]), createArgument(args[3]));
            case 5:
                return QMetaObject::invokeMethod(m_module, methodNameCStr, Qt::DirectConnection, createArgument(args[0]), createArgument(args[1]), createArgument(args[2]), createArgument(args[3]), createArgument(args[4]));
            default:
                qWarning() << "ModuleProxy: Currently supports 0-5 arguments. Got:" << args.size();
                return false;
        }
    } else if (strcmp(returnTypeName, "bool") == 0) {
        // Bool return type
        qDebug() << "ModuleProxy: invokeMethodByArgCount - bool case with" << args.size() << "arguments";
        bool* result = static_cast<bool*>(returnValue);
        bool success = false;
        switch (args.size()) {
            case 0:
                success = QMetaObject::invokeMethod(m_module, methodNameCStr, Qt::DirectConnection, Q_RETURN_ARG(bool, *result));
                break;
            case 1:
                qDebug() << "ModuleProxy: Calling" << methodNameCStr << "with arg:" << args[0];
                success = QMetaObject::invokeMethod(m_module, methodNameCStr, Qt::DirectConnection, Q_RETURN_ARG(bool, *result), createArgument(args[0]));
                break;
            case 2:
                success = QMetaObject::invokeMethod(m_module, methodNameCStr, Qt::DirectConnection, Q_RETURN_ARG(bool, *result), createArgument(args[0]), createArgument(args[1]));
                break;
            case 3:
                success = QMetaObject::invokeMethod(m_module, methodNameCStr, Qt::DirectConnection, Q_RETURN_ARG(bool, *result), createArgument(args[0]), createArgument(args[1]), createArgument(args[2]));
                break;
            case 4:
                success = QMetaObject::invokeMethod(m_module, methodNameCStr, Qt::DirectConnection, Q_RETURN_ARG(bool, *result), createArgument(args[0]), createArgument(args[1]), createArgument(args[2]), createArgument(args[3]));
                break;
            case 5:
                success = QMetaObject::invokeMethod(m_module, methodNameCStr, Qt::DirectConnection, Q_RETURN_ARG(bool, *result), createArgument(args[0]), createArgument(args[1]), createArgument(args[2]), createArgument(args[3]), createArgument(args[4]));
                break;
            default:
                qWarning() << "ModuleProxy: Currently supports 0-5 arguments. Got:" << args.size();
                return false;
        }
        qDebug() << "ModuleProxy: QMetaObject::invokeMethod returned:" << success << "result value:" << *result;
        return success;
    } else if (strcmp(returnTypeName, "int") == 0) {
        // Int return type
        int* result = static_cast<int*>(returnValue);
        switch (args.size()) {
            case 0:
                return QMetaObject::invokeMethod(m_module, methodNameCStr, Qt::DirectConnection, Q_RETURN_ARG(int, *result));
            case 1:
                return QMetaObject::invokeMethod(m_module, methodNameCStr, Qt::DirectConnection, Q_RETURN_ARG(int, *result), createArgument(args[0]));
            case 2:
                return QMetaObject::invokeMethod(m_module, methodNameCStr, Qt::DirectConnection, Q_RETURN_ARG(int, *result), createArgument(args[0]), createArgument(args[1]));
            case 3:
                return QMetaObject::invokeMethod(m_module, methodNameCStr, Qt::DirectConnection, Q_RETURN_ARG(int, *result), createArgument(args[0]), createArgument(args[1]), createArgument(args[2]));
            case 4:
                return QMetaObject::invokeMethod(m_module, methodNameCStr, Qt::DirectConnection, Q_RETURN_ARG(int, *result), createArgument(args[0]), createArgument(args[1]), createArgument(args[2]), createArgument(args[3]));
            case 5:
                return QMetaObject::invokeMethod(m_module, methodNameCStr, Qt::DirectConnection, Q_RETURN_ARG(int, *result), createArgument(args[0]), createArgument(args[1]), createArgument(args[2]), createArgument(args[3]), createArgument(args[4]));
            default:
                qWarning() << "ModuleProxy: Currently supports 0-5 arguments. Got:" << args.size();
                return false;
        }
    } else if (strcmp(returnTypeName, "QString") == 0) {
        // QString return type
        QString* result = static_cast<QString*>(returnValue);
        switch (args.size()) {
            case 0:
                return QMetaObject::invokeMethod(m_module, methodNameCStr, Qt::DirectConnection, Q_RETURN_ARG(QString, *result));
            case 1:
                return QMetaObject::invokeMethod(m_module, methodNameCStr, Qt::DirectConnection, Q_RETURN_ARG(QString, *result), createArgument(args[0]));
            case 2:
                return QMetaObject::invokeMethod(m_module, methodNameCStr, Qt::DirectConnection, Q_RETURN_ARG(QString, *result), createArgument(args[0]), createArgument(args[1]));
            case 3:
                return QMetaObject::invokeMethod(m_module, methodNameCStr, Qt::DirectConnection, Q_RETURN_ARG(QString, *result), createArgument(args[0]), createArgument(args[1]), createArgument(args[2]));
            case 4:
                return QMetaObject::invokeMethod(m_module, methodNameCStr, Qt::DirectConnection, Q_RETURN_ARG(QString, *result), createArgument(args[0]), createArgument(args[1]), createArgument(args[2]), createArgument(args[3]));
            case 5:
                return QMetaObject::invokeMethod(m_module, methodNameCStr, Qt::DirectConnection, Q_RETURN_ARG(QString, *result), createArgument(args[0]), createArgument(args[1]), createArgument(args[2]), createArgument(args[3]), createArgument(args[4]));
            default:
                qWarning() << "ModuleProxy: Currently supports 0-5 arguments. Got:" << args.size();
                return false;
        }
    } else if (strcmp(returnTypeName, "QVariant") == 0) {
        // QVariant return type
        QVariant* result = static_cast<QVariant*>(returnValue);
        switch (args.size()) {
            case 0:
                return QMetaObject::invokeMethod(m_module, methodNameCStr, Qt::DirectConnection, Q_RETURN_ARG(QVariant, *result));
            case 1:
                return QMetaObject::invokeMethod(m_module, methodNameCStr, Qt::DirectConnection, Q_RETURN_ARG(QVariant, *result), createArgument(args[0]));
            case 2:
                return QMetaObject::invokeMethod(m_module, methodNameCStr, Qt::DirectConnection, Q_RETURN_ARG(QVariant, *result), createArgument(args[0]), createArgument(args[1]));
            case 3:
                return QMetaObject::invokeMethod(m_module, methodNameCStr, Qt::DirectConnection, Q_RETURN_ARG(QVariant, *result), createArgument(args[0]), createArgument(args[1]), createArgument(args[2]));
            case 4:
                return QMetaObject::invokeMethod(m_module, methodNameCStr, Qt::DirectConnection, Q_RETURN_ARG(QVariant, *result), createArgument(args[0]), createArgument(args[1]), createArgument(args[2]), createArgument(args[3]));
            case 5:
                return QMetaObject::invokeMethod(m_module, methodNameCStr, Qt::DirectConnection, Q_RETURN_ARG(QVariant, *result), createArgument(args[0]), createArgument(args[1]), createArgument(args[2]), createArgument(args[3]), createArgument(args[4]));
            default:
                qWarning() << "ModuleProxy: Currently supports 0-5 arguments. Got:" << args.size();
                return false;
        }
    } else {
        qWarning() << "ModuleProxy: Unsupported return type in invokeMethodByArgCount:" << returnTypeName;
        return false;
    }
}

QVariant ModuleProxy::callRemoteMethod(const QString& methodName, const QVariantList& args)
{
    if (!m_module) {
        qWarning() << "ModuleProxy: Cannot call method on null module:" << methodName;
        return QVariant();
    }

    if (methodName.isEmpty()) {
        qWarning() << "ModuleProxy: Method name cannot be empty";
        return QVariant();
    }

    qDebug() << "ModuleProxy: Calling method" << methodName << "on module" << m_module << "with args:" << args;

    // Find the method to get its return type
    const QMetaObject* metaObject = m_module->metaObject();
    int methodIndex = -1;
    
    qDebug() << "ModuleProxy: Looking for method" << methodName << "with" << args.size() << "arguments";
    qDebug() << "ModuleProxy: Available methods in" << metaObject->className() << ":";
    
    // Debug: List all available methods
    for (int i = 0; i < metaObject->methodCount(); ++i) {
        QMetaMethod method = metaObject->method(i);
        qDebug() << "  Method" << i << ":" << method.name() << "with" << method.parameterCount() << "parameters, return type:" << method.returnMetaType().name();
    }
    
    // Find the method with matching name and argument count
    for (int i = 0; i < metaObject->methodCount(); ++i) {
        QMetaMethod method = metaObject->method(i);
        if (method.name() == methodName && method.parameterCount() == args.size()) {
            methodIndex = i;
            qDebug() << "ModuleProxy: Found matching method at index" << i;
            break;
        }
    }
    
    if (methodIndex == -1) {
        qWarning() << "ModuleProxy: Method not found:" << methodName << "with" << args.size() << "arguments";
        return QVariant();
    }
    
    QMetaMethod method = metaObject->method(methodIndex);
    QMetaType returnType = method.returnMetaType();
    
    qDebug() << "ModuleProxy: Method signature:" << method.methodSignature();
    qDebug() << "ModuleProxy: Parameter types:";
    for (int i = 0; i < method.parameterCount(); ++i) {
        qDebug() << "  Param" << i << ":" << method.parameterMetaType(i).name();
    }
    
    // Handle different return types
    bool success = false;
    QVariant result;
    
    if (returnType == QMetaType::fromType<void>()) {
        // Void method - no return value expected
        success = invokeMethodByArgCount(methodName, args, nullptr, nullptr);
        if (success) {
            result = QVariant(true); // Return true to indicate success
        }
    } else if (returnType == QMetaType::fromType<bool>()) {
        // Bool return type
        qDebug() << "ModuleProxy: Invoking bool method" << methodName;
        bool boolResult = false;
        success = invokeMethodByArgCount(methodName, args, &boolResult, "bool");
        qDebug() << "ModuleProxy: Bool method invocation result:" << success << "value:" << boolResult;
        if (success) {
            result = QVariant(boolResult);
        }
    } else if (returnType == QMetaType::fromType<int>()) {
        // Int return type
        int intResult = 0;
        success = invokeMethodByArgCount(methodName, args, &intResult, "int");
        if (success) {
            result = QVariant(intResult);
        }
    } else if (returnType == QMetaType::fromType<QString>()) {
        // QString return type
        QString stringResult;
        success = invokeMethodByArgCount(methodName, args, &stringResult, "QString");
        if (success) {
            result = QVariant(stringResult);
        }
    } else if (returnType == QMetaType::fromType<QVariant>()) {
        // QVariant return type
        QVariant variantResult;
        success = invokeMethodByArgCount(methodName, args, &variantResult, "QVariant");
        if (success) {
            result = variantResult;
        }
    } else {
        qWarning() << "ModuleProxy: Unsupported return type:" << returnType.name() << "for method:" << methodName;
        return QVariant();
    }

    if (!success) {
        qWarning() << "ModuleProxy: Failed to invoke method" << methodName << "on module" << m_module;
        return QVariant();
    }

    qDebug() << "ModuleProxy: Successfully called method" << methodName << "on module" << m_module;
    return result;
}

// Include MOC for template instantiation
#include "moc_module_proxy.cpp" 