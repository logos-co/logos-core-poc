#include "module_proxy.h"
#include <QDebug>
#include <QMetaObject>
#include <QMetaMethod>
#include <QMetaType>
#include <QJsonArray>
#include <QStringList>

// Helper macro to simplify method invocation with return types
#define INVOKE_METHOD_WITH_RETURN(returnType, castType) \
    do { \
        castType* result = static_cast<castType*>(returnValue); \
        switch (args.size()) { \
            case 0: \
                return QMetaObject::invokeMethod(module, methodNameCStr, Qt::DirectConnection, Q_RETURN_ARG(returnType, *result)); \
            case 1: \
                return QMetaObject::invokeMethod(module, methodNameCStr, Qt::DirectConnection, Q_RETURN_ARG(returnType, *result), scopedArgs[0].arg); \
            case 2: \
                return QMetaObject::invokeMethod(module, methodNameCStr, Qt::DirectConnection, Q_RETURN_ARG(returnType, *result), scopedArgs[0].arg, scopedArgs[1].arg); \
            case 3: \
                return QMetaObject::invokeMethod(module, methodNameCStr, Qt::DirectConnection, Q_RETURN_ARG(returnType, *result), scopedArgs[0].arg, scopedArgs[1].arg, scopedArgs[2].arg); \
            case 4: \
                return QMetaObject::invokeMethod(module, methodNameCStr, Qt::DirectConnection, Q_RETURN_ARG(returnType, *result), scopedArgs[0].arg, scopedArgs[1].arg, scopedArgs[2].arg, scopedArgs[3].arg); \
            case 5: \
                return QMetaObject::invokeMethod(module, methodNameCStr, Qt::DirectConnection, Q_RETURN_ARG(returnType, *result), scopedArgs[0].arg, scopedArgs[1].arg, scopedArgs[2].arg, scopedArgs[3].arg, scopedArgs[4].arg); \
            default: \
                qWarning() << "ModuleProxy: Currently supports 0-5 arguments. Got:" << args.size(); \
                return false; \
        } \
    } while(0)

namespace {
    class ScopedQArg {
    public:
        ScopedQArg(QMetaMethodArgument a, std::function<void(const void*)> d)
            : arg(a), deleter(std::move(d)) {}

        ~ScopedQArg() {
            if (deleter) {
                deleter(arg.data);
            }
        }

        ScopedQArg(ScopedQArg&&) = default;
        ScopedQArg& operator=(ScopedQArg&&) = default;
        ScopedQArg(const ScopedQArg&) = delete;
        ScopedQArg& operator=(const ScopedQArg&) = delete;

        QMetaMethodArgument arg;

    private:
        std::function<void(const void*)> deleter;
    };

    auto toScopedQArgs(const QVariantList& args)
    {
        auto scopedArgs = std::vector<ScopedQArg>{};
        for (const auto& arg : args) {
            switch (arg.typeId()) {
                case QMetaType::Int: {
                    auto value = new int{arg.toInt()};
                    scopedArgs.emplace_back(
                        Q_ARG(int, *value),
                        [](const void* data) {
                            delete static_cast<const int*>(data);
                        }
                    );
                    break;
                }
                case QMetaType::QString:
                default: {
                    auto value = new QString{arg.toString()};
                    scopedArgs.emplace_back(
                        Q_ARG(QString, *value),
                        [](const void* data) {
                            delete static_cast<const QString*>(data);
                        }
                    );
                    break;
                }
            }
        }
        return scopedArgs;
    }

    // Helper method to invoke methods with different return types and argument counts
    bool invokeMethodByArgCount(QObject *module, const QString& methodName, const QVariantList& args, void* returnValue, const char* returnTypeName)
    {
        // Store the UTF-8 data to ensure it stays in scope
        QByteArray methodNameBytes = methodName.toUtf8();
        const char* methodNameCStr = methodNameBytes.constData();

        auto scopedArgs = toScopedQArgs(args);

        if (returnValue == nullptr) {
            // Void method - no return value
            switch (args.size()) {
                case 0:
                    return QMetaObject::invokeMethod(module, methodNameCStr, Qt::DirectConnection);
                case 1:
                    return QMetaObject::invokeMethod(module, methodNameCStr, Qt::DirectConnection, scopedArgs[0].arg);
                case 2:
                    return QMetaObject::invokeMethod(module, methodNameCStr, Qt::DirectConnection, scopedArgs[0].arg, scopedArgs[1].arg);
                case 3:
                    return QMetaObject::invokeMethod(module, methodNameCStr, Qt::DirectConnection, scopedArgs[0].arg, scopedArgs[1].arg, scopedArgs[2].arg);
                case 4:
                    return QMetaObject::invokeMethod(module, methodNameCStr, Qt::DirectConnection, scopedArgs[0].arg, scopedArgs[1].arg, scopedArgs[2].arg, scopedArgs[3].arg);
                case 5:
                    return QMetaObject::invokeMethod(module, methodNameCStr, Qt::DirectConnection, scopedArgs[0].arg, scopedArgs[1].arg, scopedArgs[2].arg, scopedArgs[3].arg, scopedArgs[4].arg);
                default:
                    qWarning() << "ModuleProxy: Currently supports 0-5 arguments. Got:" << args.size();
                    return false;
            }
        } else if (strcmp(returnTypeName, "bool") == 0) {
            qDebug() << "ModuleProxy: invokeMethodByArgCount - bool case with" << args.size() << "arguments";
            INVOKE_METHOD_WITH_RETURN(bool, bool);
        } else if (strcmp(returnTypeName, "int") == 0) {
            INVOKE_METHOD_WITH_RETURN(int, int);
        } else if (strcmp(returnTypeName, "QString") == 0) {
            INVOKE_METHOD_WITH_RETURN(QString, QString);
        } else if (strcmp(returnTypeName, "QVariant") == 0) {
            INVOKE_METHOD_WITH_RETURN(QVariant, QVariant);
        } else if (strcmp(returnTypeName, "QJsonArray") == 0) {
            INVOKE_METHOD_WITH_RETURN(QJsonArray, QJsonArray);
        } else if (strcmp(returnTypeName, "QStringList") == 0) {
            INVOKE_METHOD_WITH_RETURN(QStringList, QStringList);
        } else {
            qWarning() << "ModuleProxy: Unsupported return type in invokeMethodByArgCount:" << returnTypeName;
            return false;
        }
    }
}

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

    // Each createArgument() call now generates its own unique GUID

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
        success = invokeMethodByArgCount(m_module, methodName, args, nullptr, nullptr);
        if (success) {
            result = QVariant(true); // Return true to indicate success
        }
    } else if (returnType == QMetaType::fromType<bool>()) {
        // Bool return type
        qDebug() << "ModuleProxy: Invoking bool method" << methodName;
        bool boolResult = false;
        success = invokeMethodByArgCount(m_module, methodName, args, &boolResult, "bool");
        qDebug() << "ModuleProxy: Bool method invocation result:" << success << "value:" << boolResult;
        if (success) {
            result = QVariant(boolResult);
        }
    } else if (returnType == QMetaType::fromType<int>()) {
        // Int return type
        int intResult = 0;
        success = invokeMethodByArgCount(m_module, methodName, args, &intResult, "int");
        if (success) {
            result = QVariant(intResult);
        }
    } else if (returnType == QMetaType::fromType<QString>()) {
        // QString return type
        QString stringResult;
        success = invokeMethodByArgCount(m_module, methodName, args, &stringResult, "QString");
        if (success) {
            result = QVariant(stringResult);
        }
    } else if (returnType == QMetaType::fromType<QVariant>()) {
        // QVariant return type
        QVariant variantResult;
        success = invokeMethodByArgCount(m_module, methodName, args, &variantResult, "QVariant");
        if (success) {
            result = variantResult;
        }
    } else if (returnType == QMetaType::fromType<QJsonArray>()) {
        // QJsonArray return type
        qDebug() << "ModuleProxy: Invoking QJsonArray method" << methodName;
        QJsonArray jsonArrayResult;
        success = invokeMethodByArgCount(m_module, methodName, args, &jsonArrayResult, "QJsonArray");
        qDebug() << "ModuleProxy: QJsonArray method invocation result:" << success << "array size:" << jsonArrayResult.size();
        if (success) {
            result = QVariant(jsonArrayResult);
        }
    } else if (returnType == QMetaType::fromType<QStringList>()) {
        // QStringList return type
        qDebug() << "ModuleProxy: Invoking QStringList method" << methodName;
        QStringList stringListResult;
        success = invokeMethodByArgCount(m_module, methodName, args, &stringListResult, "QStringList");
        qDebug() << "ModuleProxy: QStringList method invocation result:" << success << "list size:" << stringListResult.size();
        if (success) {
            result = QVariant(stringListResult);
        }
    } else {
        qWarning() << "ModuleProxy: Unsupported return type:" << returnType.name() << "for method:" << methodName;
        return QVariant();
    }

    if (!success) {
        qWarning() << "ModuleProxy: Failed to invoke method" << methodName << "on module" << m_module;
        return QVariant();
    }

    // Note: Argument cleanup is now handled automatically by each createArgument() call's unique GUID
    qDebug() << "ModuleProxy: Successfully called method" << methodName << "on module" << m_module;
    return result;
}

// Include MOC for template instantiation
#include "moc_module_proxy.cpp"
