#include <QtCore>
#include <QtRemoteObjects>
#include "rep_loggerinterface_replica.h"

// Token for authentication
const QString TOKEN = "abc";

// The original Calculator class with simple methods
class Calculator : public QObject
{
    Q_OBJECT

public:
    Calculator(QObject *parent = nullptr) : QObject(parent) {
        qDebug() << "Calculator created";
    }

public slots:
    int add(int a, int b) {
        int result = a + b;
        qDebug() << "Calculator adding:" << a << "+" << b << "=" << result;
        return result;
    }
    
    int multiply(int a, int b) {
        int result = a * b;
        qDebug() << "Calculator multiplying:" << a << "*" << b << "=" << result;
        return result;
    }
    
    // Add more methods to test the dynamic nature
    int subtract(int a, int b) {
        int result = a - b;
        qDebug() << "Calculator subtracting:" << a << "-" << b << "=" << result;
        return result;
    }
    
    int divide(int a, int b) {
        if (b == 0) {
            qDebug() << "Calculator division by zero error";
            return -1;
        }
        int result = a / b;
        qDebug() << "Calculator dividing:" << a << "/" << b << "=" << result;
        return result;
    }
};

// DynamicAuthProxy - Uses Qt's meta-object system to automatically authenticate any QObject
class DynamicAuthProxy : public QObject
{
    Q_OBJECT

public:
    DynamicAuthProxy(QObject* targetObject, const QString& validToken, QObject *parent = nullptr) 
        : QObject(parent), m_targetObject(targetObject), m_validToken(validToken) {
        qDebug() << "DynamicAuthProxy created for" << targetObject->metaObject()->className();
        
        // Get all methods from the target object
        const QMetaObject* metaObj = targetObject->metaObject();
        for (int i = metaObj->methodOffset(); i < metaObj->methodCount(); ++i) {
            QMetaMethod method = metaObj->method(i);
            if (method.methodType() == QMetaMethod::Slot) {
                // For each target method, create a corresponding proxy method with token param
                QString methodName = QString::fromLatin1(method.name());
                QByteArray signature = method.methodSignature();
                // Convert return type to QByteArray properly
                QByteArray returnType = QMetaObject::normalizedType(method.typeName());
                
                // Store method information for dynamic invocation
                MethodInfo info;
                info.name = methodName;
                info.signature = signature;
                info.returnType = returnType;
                info.method = method;
                info.parameterTypes = method.parameterTypes();
                info.parameterCount = method.parameterCount();
                
                m_methods[methodName] = info;
                
                qDebug() << "Created proxy for method:" << signature << "with return type:" << returnType;
                qDebug() << "  Parameter types:" << info.parameterTypes;
                qDebug() << "  Parameter count:" << info.parameterCount;
            }
        }
    }

public slots:
    // Generic dynamic method that handles all remote calls
    // This is the only method exposed to Qt Remote Objects
    QVariant callMethod(const QString& methodName, const QVariantList& args) {
        qDebug() << "=== SERVER: Dynamic proxy received call for" << methodName << "with" << args.size() << "arguments";
        qDebug() << "=== SERVER: Arguments:" << args;
        qDebug() << "=== SERVER: Expected token:" << m_validToken;
        
        // Always check first argument for token
        if (args.isEmpty()) {
            qDebug() << "=== SERVER: Authentication failed: No arguments provided";
            return QVariant(-1);
        }
        
        QString receivedToken = args[0].toString();
        qDebug() << "=== SERVER: Received token:" << receivedToken;
        
        if (receivedToken != m_validToken) {
            qDebug() << "=== SERVER: Authentication failed: Token mismatch. Expected:" << m_validToken << "Got:" << receivedToken;
            return QVariant(-1);
        }
        
        qDebug() << "=== SERVER: Authentication successful";
        
        // Find the method in our map
        if (!m_methods.contains(methodName)) {
            qDebug() << "=== SERVER: Method not found:" << methodName;
            qDebug() << "=== SERVER: Available methods:" << m_methods.keys();
            return QVariant(-1);
        }
        
        qDebug() << "=== SERVER: Method found:" << methodName;
        
        const MethodInfo& methodInfo = m_methods[methodName];
        
        // Remove token from arguments
        QVariantList realArgs = args.mid(1);
        qDebug() << "=== SERVER: Real arguments (without token):" << realArgs;
        
        // Validate argument count
        if (realArgs.size() != methodInfo.parameterCount) {
            qDebug() << "=== SERVER: Argument count mismatch for" << methodName 
                     << "- expected:" << methodInfo.parameterCount 
                     << "got:" << realArgs.size();
            return QVariant(-1);
        }
        
        qDebug() << "=== SERVER: Argument count validation passed";
        
        // Dynamically invoke the method using reflection
        QVariant result = invokeMethodDynamically(methodInfo, realArgs);
        qDebug() << "=== SERVER: Method invocation completed, result:" << result;
        return result;
    }

private:
    struct MethodInfo {
        QString name;
        QByteArray signature;
        QByteArray returnType;
        QMetaMethod method;
        QList<QByteArray> parameterTypes;
        int parameterCount;
    };
    
    // Dynamic method invocation using Qt's meta-object system
    QVariant invokeMethodDynamically(const MethodInfo& methodInfo, const QVariantList& args) {
        qDebug() << "Dynamically invoking method:" << methodInfo.name 
                 << "with" << args.size() << "arguments";
        
        // Handle different return types
        if (methodInfo.returnType == "int") {
            return invokeIntMethod(methodInfo, args);
        } else if (methodInfo.returnType == "QString") {
            return invokeStringMethod(methodInfo, args);
        } else if (methodInfo.returnType == "void") {
            invokeVoidMethod(methodInfo, args);
            return QVariant();
        } else {
            qDebug() << "Unsupported return type:" << methodInfo.returnType;
            return -1;
        }
    }
    
    // Specialized method for int return types
    int invokeIntMethod(const MethodInfo& methodInfo, const QVariantList& args) {
        int result = -1;
        
        qDebug() << "=== SERVER: invokeIntMethod called for" << methodInfo.name;
        qDebug() << "=== SERVER: Parameter types:" << methodInfo.parameterTypes;
        qDebug() << "=== SERVER: Input args:" << args;
        
        // Clear storage arrays before use
        m_intArgs.clear();
        m_stringArgs.clear();
        m_doubleArgs.clear();
        m_boolArgs.clear();
        
        // Dynamically create arguments based on parameter types
        QList<QVariant> convertedArgs = convertArgumentTypes(methodInfo.parameterTypes, args);
        qDebug() << "=== SERVER: Converted args:" << convertedArgs;
        
        // For 2-argument int methods (like add, multiply), use a simpler approach
        if (convertedArgs.size() == 2 && 
            methodInfo.parameterTypes.size() == 2 &&
            methodInfo.parameterTypes[0] == "int" && 
            methodInfo.parameterTypes[1] == "int") {
            
            int arg1 = convertedArgs[0].toInt();
            int arg2 = convertedArgs[1].toInt();
            
            qDebug() << "=== SERVER: Using direct int invocation with args:" << arg1 << arg2;
            
            bool success = methodInfo.method.invoke(m_targetObject, 
                                                   Qt::DirectConnection, 
                                                   Q_RETURN_ARG(int, result),
                                                   Q_ARG(int, arg1),
                                                   Q_ARG(int, arg2));
            
            if (success) {
                qDebug() << "=== SERVER: Successfully invoked" << methodInfo.name << "result:" << result;
                return result;
            } else {
                qDebug() << "=== SERVER: Failed to invoke" << methodInfo.name;
                return -1;
            }
        }
        
        // Fallback to generic approach for other cases
        qDebug() << "=== SERVER: Using generic invocation approach";
        
        // Create QGenericArgument array
        QGenericArgument genericArgs[10];
        for (int i = 0; i < qMin(convertedArgs.size(), 10); ++i) {
            genericArgs[i] = createGenericArgument(methodInfo.parameterTypes[i], convertedArgs[i]);
        }
        
        qDebug() << "=== SERVER: About to invoke method with" << convertedArgs.size() << "arguments";
        
        // Invoke the method with the converted arguments using the correct overload
        bool success = false;
        
        switch (convertedArgs.size()) {
            case 0:
                success = methodInfo.method.invoke(m_targetObject, Qt::DirectConnection, Q_RETURN_ARG(int, result));
                break;
            case 1:
                success = methodInfo.method.invoke(m_targetObject, Qt::DirectConnection, Q_RETURN_ARG(int, result), genericArgs[0]);
                break;
            case 2:
                success = methodInfo.method.invoke(m_targetObject, Qt::DirectConnection, Q_RETURN_ARG(int, result), genericArgs[0], genericArgs[1]);
                break;
            case 3:
                success = methodInfo.method.invoke(m_targetObject, Qt::DirectConnection, Q_RETURN_ARG(int, result), genericArgs[0], genericArgs[1], genericArgs[2]);
                break;
            case 4:
                success = methodInfo.method.invoke(m_targetObject, Qt::DirectConnection, Q_RETURN_ARG(int, result), genericArgs[0], genericArgs[1], genericArgs[2], genericArgs[3]);
                break;
            case 5:
                success = methodInfo.method.invoke(m_targetObject, Qt::DirectConnection, Q_RETURN_ARG(int, result), genericArgs[0], genericArgs[1], genericArgs[2], genericArgs[3], genericArgs[4]);
                break;
            default:
                qDebug() << "Too many arguments for method invocation:" << convertedArgs.size();
                return -1;
        }
        
        if (success) {
            qDebug() << "Successfully invoked" << methodInfo.name << "result:" << result;
            return result;
        } else {
            qDebug() << "Failed to invoke" << methodInfo.name;
            return -1;
        }
    }
    
    // Placeholder for string return types
    QString invokeStringMethod(const MethodInfo& methodInfo, const QVariantList& args) {
        QString result;
        // Similar implementation for QString return types
        qDebug() << "String method invocation not fully implemented:" << methodInfo.name;
        return result;
    }
    
    // Placeholder for void return types
    void invokeVoidMethod(const MethodInfo& methodInfo, const QVariantList& args) {
        // Similar implementation for void return types
        qDebug() << "Void method invocation not fully implemented:" << methodInfo.name;
    }
    
    // Convert arguments to proper types based on parameter type information
    QList<QVariant> convertArgumentTypes(const QList<QByteArray>& parameterTypes, const QVariantList& args) {
        QList<QVariant> convertedArgs;
        
        for (int i = 0; i < parameterTypes.size() && i < args.size(); ++i) {
            const QByteArray& paramType = parameterTypes[i];
            const QVariant& arg = args[i];
            
            if (paramType == "int") {
                convertedArgs.append(arg.toInt());
            } else if (paramType == "QString") {
                convertedArgs.append(arg.toString());
            } else if (paramType == "double" || paramType == "float") {
                convertedArgs.append(arg.toDouble());
            } else if (paramType == "bool") {
                convertedArgs.append(arg.toBool());
            } else {
                // Default: keep as is
                convertedArgs.append(arg);
            }
        }
        
        return convertedArgs;
    }
    
    // Create QGenericArgument from type and value
    QGenericArgument createGenericArgument(const QByteArray& typeName, const QVariant& value) {
        if (typeName == "int") {
            // Store the value in a member variable to ensure it stays alive
            m_intArgs.append(value.toInt());
            qDebug() << "=== SERVER: Creating int argument with value:" << m_intArgs.last();
            return QGenericArgument("int", &m_intArgs.last());
        } else if (typeName == "QString") {
            m_stringArgs.append(value.toString());
            qDebug() << "=== SERVER: Creating QString argument with value:" << m_stringArgs.last();
            return QGenericArgument("QString", &m_stringArgs.last());
        } else if (typeName == "double") {
            m_doubleArgs.append(value.toDouble());
            qDebug() << "=== SERVER: Creating double argument with value:" << m_doubleArgs.last();
            return QGenericArgument("double", &m_doubleArgs.last());
        } else if (typeName == "bool") {
            m_boolArgs.append(value.toBool());
            qDebug() << "=== SERVER: Creating bool argument with value:" << m_boolArgs.last();
            return QGenericArgument("bool", &m_boolArgs.last());
        }
        
        qDebug() << "=== SERVER: Unknown type name:" << typeName;
        return QGenericArgument();
    }
    
    QObject* m_targetObject;
    QString m_validToken;
    QMap<QString, MethodInfo> m_methods;
    
    // Storage for argument values to keep them alive during method calls
    mutable QList<int> m_intArgs;
    mutable QList<QString> m_stringArgs;
    mutable QList<double> m_doubleArgs;
    mutable QList<bool> m_boolArgs;
};

class ProcessHostApp : public QObject
{
    Q_OBJECT

public:
    ProcessHostApp(const QString &guid, const QUrl &registryUrl, QObject *parent = nullptr)
        : QObject(parent), m_guid(guid)
    {
        qDebug() << "Process host started with PID:" << QCoreApplication::applicationPid();
        qDebug() << "Using GUID:" << m_guid;
        qDebug() << "Connecting to registry at:" << registryUrl.toString();
        
        // Create the Calculator instance
        m_calculator = new Calculator(this);
        
        // Create a dynamic authentication proxy for the calculator
        m_authProxy = new DynamicAuthProxy(m_calculator, TOKEN, this);
        
        // Create a QRemoteObjectHost for the calculator
        m_srcNode = new QRemoteObjectHost(QUrl(QStringLiteral("local:calculator")), this);
        qDebug() << "Created calculator host at:" << m_srcNode->hostUrl().toString();
        
        // Expose the authenticated calculator instance
        m_srcNode->enableRemoting(m_authProxy, "Calculator");
        qDebug() << "Exposed Calculator as a remote object (with dynamic authentication)";
        
        // Connect to remote objects registry
        m_remoteNode = new QRemoteObjectNode(this);
        bool connected = m_remoteNode->connectToNode(registryUrl);
        
        if (!connected) {
            qDebug() << "Failed to connect to registry at:" << registryUrl.toString();
            return;
        }
        
        qDebug() << "Connected to registry successfully";
        
        // Acquire the replica of the LoggerInterface
        m_loggerReplica = m_remoteNode->acquire<LoggerInterfaceReplica>("LoggerInterface");
        
        // Connect to the replica state change signal
        connect(m_loggerReplica, &LoggerInterfaceReplica::stateChanged,
                this, &ProcessHostApp::onLoggerStateChanged);
        
        // Connect to the replica signals
        connect(m_loggerReplica, &LoggerInterfaceReplica::messageLogged,
                this, &ProcessHostApp::onMessageLogged);
        
        // Create a timer to write to the file and use remote logger every 3 seconds
        m_timer = new QTimer(this);
        connect(m_timer, &QTimer::timeout, this, &ProcessHostApp::onTimerTimeout);
        m_timer->start(3000); // 3000 ms = 3 seconds
        
        // Execute the initial action immediately
        QMetaObject::invokeMethod(this, &ProcessHostApp::onTimerTimeout, Qt::QueuedConnection);
        
        // Set up a timer to perform calculator operations every 5 seconds
        m_calculatorTimer = new QTimer(this);
        connect(m_calculatorTimer, &QTimer::timeout, this, &ProcessHostApp::performCalculation);
        m_calculatorTimer->start(5000); // 5000 ms = 5 seconds
    }
    
    ~ProcessHostApp()
    {
        if (m_timer) {
            m_timer->stop();
        }
        
        if (m_calculatorTimer) {
            m_calculatorTimer->stop();
        }
    }

private slots:
    void onLoggerStateChanged(QRemoteObjectReplica::State state, QRemoteObjectReplica::State oldState)
    {
        qDebug() << "Logger replica state changed from" << oldState << "to" << state;
        
        if (state == QRemoteObjectReplica::Valid) {
            qDebug() << "Logger replica is now valid, guid:" << m_loggerReplica->guid();
        }
    }
    
    void onMessageLogged(const QString &message, const QString &timestamp)
    {
        qDebug() << "Message logged event received - Message:" << message << "Timestamp:" << timestamp;
    }
    
    void onTimerTimeout()
    {
        // 1. Write to file (keeping existing functionality)
        QFile file("test.txt");
        if (file.open(QIODevice::Append | QIODevice::Text)) {
            QTextStream stream(&file);
            QString timestamp = QDateTime::currentDateTime().toString(Qt::ISODate);
            stream << "hello [" << m_guid << "] - written at " << timestamp << Qt::endl;
            file.close();
            qDebug() << "Wrote to test.txt with GUID:" << m_guid << "at" << timestamp;
            
            // 2. Use the remote logger if it's valid
            if (m_loggerReplica && m_loggerReplica->state() == QRemoteObjectReplica::Valid) {
                QString message = QString("File updated from process_host with GUID: %1").arg(m_guid);
                m_loggerReplica->logMessage(message);
                qDebug() << "Sent log message to main application";
            } else {
                qDebug() << "Logger replica not valid yet, couldn't send message";
            }
        } else {
            qDebug() << "Failed to open test.txt";
        }
    }
    
    void performCalculation()
    {
        // Randomly choose between different operations to test dynamic nature
        int operation = QRandomGenerator::global()->bounded(4);

        qDebug() << "\n\n\n=========================== Performing calculation\n\n\n";

        // Generate random numbers between 1 and 100
        int a = QRandomGenerator::global()->bounded(1, 101);
        int b = QRandomGenerator::global()->bounded(1, 101);
        
        int result;
        QString operationName;
        
        switch (operation) {
            case 0:
                result = m_calculator->add(a, b);
                operationName = "addition";
                break;
            case 1:
                result = m_calculator->multiply(a, b);
                operationName = "multiplication";
                break;
            case 2:
                result = m_calculator->subtract(a, b);
                operationName = "subtraction";
                break;
            case 3:
                result = m_calculator->divide(a, b);
                operationName = "division";
                break;
        }
        
        // Log the calculation result via the remote logger if available
        if (m_loggerReplica && m_loggerReplica->state() == QRemoteObjectReplica::Valid) {
            QString message = QString("Calculator %1 result: %2").arg(operationName).arg(result);
            m_loggerReplica->logMessage(message);
        }
    }

private:
    QString m_guid;
    QTimer *m_timer = nullptr;
    QTimer *m_calculatorTimer = nullptr;
    QRemoteObjectNode *m_remoteNode = nullptr;
    QRemoteObjectHost *m_srcNode = nullptr;
    LoggerInterfaceReplica *m_loggerReplica = nullptr;
    Calculator *m_calculator = nullptr;
    DynamicAuthProxy *m_authProxy = nullptr;
};

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    
    // Set up command line parser for the GUID and registry URL
    QCommandLineParser parser;
    parser.addHelpOption();
    parser.addOption(QCommandLineOption("guid", "GUID to include in output", "guid"));
    parser.addOption(QCommandLineOption("registry", "URL of the registry to connect to", "registry"));
    
    // Process the command line arguments
    parser.process(app);
    
    // Get the GUID from the command line
    QString guid = parser.value("guid");
    if (guid.isEmpty()) {
        qDebug() << "No GUID provided, using default";
        guid = "no-guid-provided";
    }
    
    // Get the registry URL
    QString registryUrlStr = parser.value("registry");
    if (registryUrlStr.isEmpty()) {
        qDebug() << "No registry URL provided, using default";
        registryUrlStr = "local:registry";
    }
    
    QUrl registryUrl(registryUrlStr);
    
    // Create the application object
    ProcessHostApp processHost(guid, registryUrl);
    
    // Wait for the application to exit
    return app.exec();
}

#include "process_host_calculator.moc" 
