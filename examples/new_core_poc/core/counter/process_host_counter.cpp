#include <QtCore>
#include <QtRemoteObjects>
#include "rep_loggerinterface_replica.h"

// Token for authentication - hidden from application code
namespace {
    const QString TOKEN = "abc";
}

// A fully implemented counter class
class Counter : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int count READ count WRITE setCount NOTIFY countChanged)

public:
    Counter(QObject *parent = nullptr) : QObject(parent), m_count(0) {
        qDebug() << "Counter created with initial count:" << m_count;
    }

    int count() const {
        return m_count;
    }
    
    void setCount(int count) {
        if (m_count != count) {
            m_count = count;
            emit countChanged();
        }
    }

public slots:
    void increment() {
        setCount(m_count + 2);
        qDebug() << "Counter incremented to:" << m_count;
    }
    
    void decrement() {
        setCount(m_count - 1);
        qDebug() << "Counter decremented to:" << m_count;
    }
    
    void reset() {
        setCount(0);
        qDebug() << "Counter reset to:" << m_count;
    }
    
    void multiplyWithCallback(const QString &requestId, int factor) {
        static bool isFirstCall = true;
        int result = m_count * factor;
        qDebug() << "Multiplying counter value" << m_count << "by factor" << factor 
                 << "= result:" << result << "requestId:" << requestId;

        // Use larger delay for first call, smaller for subsequent calls
        int minDelay = isFirstCall ? 6000 : 2000;
        int maxDelay = isFirstCall ? 10000 : 5000;
        
        QTimer::singleShot(QRandomGenerator::global()->bounded(minDelay, maxDelay), this, [this, result, requestId]() {
            qDebug() << "Emitting multiplyResult signal with result:" << result << "requestId:" << requestId;
            emit multiplyResult(requestId, result);
        });
        
        // After first call, switch to shorter delay for subsequent calls
        if (isFirstCall) {
            isFirstCall = false;
        }
    }

signals:
    void countChanged();
    void multiplyResult(const QString &requestId, int result);

private:
    int m_count;
};

// Dynamic authentication client proxy that transparently handles authentication
class DynamicAuthClientProxy : public QObject
{
    Q_OBJECT
    
public:
    DynamicAuthClientProxy(QRemoteObjectDynamicReplica *replica, const QString &token, QObject *parent = nullptr)
        : QObject(parent), m_replica(replica), m_token(token) {
        qDebug() << "DynamicAuthClientProxy created (handles authentication transparently)";
        
        // Connect to replica state changes
        connect(replica, &QRemoteObjectDynamicReplica::stateChanged, 
                this, &DynamicAuthClientProxy::onReplicaStateChanged);
                
        // Connect to signal emitted when a remote call is made
        connect(replica, &QRemoteObjectDynamicReplica::initialized,
                this, &DynamicAuthClientProxy::onReplicaInitialized);
    }
    
    // Send a method call with the token automatically added
    QRemoteObjectPendingCall callWithToken(const QString &methodName, const QVariantList &args) {
        if (!m_replica || !m_replica->isInitialized()) {
            qDebug() << "Replica not initialized";
            return QRemoteObjectPendingCall();
        }
        
        qDebug() << "Calling" << methodName << "via dynamic proxy";
        
        // We can't easily make dynamic remote calls and get pending results
        // So we'll just invoke methods directly in the specific method implementations
        
        return QRemoteObjectPendingCall();
    }
    
    // Async method call with callback
    template<typename Func>
    void callAsync(const QString &methodName, const QVariantList &args, Func callback) {
        if (!m_replica || !m_replica->isInitialized()) {
            qDebug() << "Replica not initialized";
            callback(QVariant(-1));
            return;
        }
        
        qDebug() << "Calling" << methodName << "via dynamic proxy";
        
        // Create arguments list with token
        QVariantList callArgs;
        callArgs << m_token;
        callArgs.append(args);
        
        qDebug() << "Calling generic callMethod with" << callArgs.size() << "arguments for method:" << methodName;
        
        // For async calls, we'll use a timer to simulate the delay and then make the actual call
        // This approach works better with Qt Remote Objects' asynchronous nature
        QTimer *timer = new QTimer(this);
        timer->setSingleShot(true);
        
        connect(timer, &QTimer::timeout, this, [this, timer, callback, methodName, callArgs]() {
            // Make the actual remote call using QMetaObject::invokeMethod with QRemoteObjectPendingCall
            QRemoteObjectPendingCall pendingCall;
            
            // Try to get the pending call from the remote method
            bool success = QMetaObject::invokeMethod(m_replica,
                                    "callMethod",
                                    Qt::DirectConnection,
                                    Q_RETURN_ARG(QRemoteObjectPendingCall, pendingCall),
                                    Q_ARG(QString, methodName),
                                    Q_ARG(QVariantList, callArgs));
            
            if (success) {
                qDebug() << "Successfully initiated remote call for" << methodName;
                
                // Create a watcher to handle the asynchronous result
                QRemoteObjectPendingCallWatcher *watcher = new QRemoteObjectPendingCallWatcher(pendingCall, this);
                
                connect(watcher, &QRemoteObjectPendingCallWatcher::finished, this, [callback, methodName, watcher]() {
                    if (watcher->error() == QRemoteObjectPendingCall::NoError) {
                        QVariant result = watcher->returnValue();
                        qDebug() << "Remote call" << methodName << "completed successfully, result:" << result;
                        callback(result);
                    } else {
                        qDebug() << "Remote call" << methodName << "failed with error:" << watcher->error();
                        callback(QVariant(-1));
                    }
                    watcher->deleteLater();
                });
            } else {
                qDebug() << "Failed to initiate remote call for" << methodName;
                callback(QVariant(-1));
            }
            
            timer->deleteLater();
        });
        
        timer->start(500); // Start waiting for result
    }
    
    // Specific convenience methods for calculator
    void add(int a, int b, std::function<void(int)> callback) {
        QVariantList args;
        args << a << b;
        callAsync("add", args, [callback](const QVariant &result) {
            callback(result.toInt());
        });
    }
    
    void multiply(int a, int b, std::function<void(int)> callback) {
        QVariantList args;
        args << a << b;
        callAsync("multiply", args, [callback](const QVariant &result) {
            callback(result.toInt());
        });
    }
    
    // Synchronous version for simple use cases
    QVariant callSync(const QString &methodName, const QVariantList &args) {
        if (!m_replica || !m_replica->isInitialized()) {
            qDebug() << "Replica not initialized for sync call";
            return QVariant(-1);
        }
        
        // Create arguments list with token
        QVariantList callArgs;
        callArgs << m_token;
        callArgs.append(args);
        
        qDebug() << "Making synchronous call to" << methodName;
        
        // Make the remote call and get a pending call
        QRemoteObjectPendingCall pendingCall;
        bool success = QMetaObject::invokeMethod(m_replica,
                                "callMethod",
                                Qt::DirectConnection,
                                Q_RETURN_ARG(QRemoteObjectPendingCall, pendingCall),
                                Q_ARG(QString, methodName),
                                Q_ARG(QVariantList, callArgs));
        
        if (success) {
            // Wait for the call to complete synchronously
            pendingCall.waitForFinished();
            
            if (pendingCall.error() == QRemoteObjectPendingCall::NoError) {
                QVariant result = pendingCall.returnValue();
                qDebug() << "Synchronous call to" << methodName << "succeeded, result:" << result;
                return result;
            } else {
                qDebug() << "Synchronous call to" << methodName << "failed with error:" << pendingCall.error();
                return QVariant(-1);
            }
        } else {
            qDebug() << "Failed to initiate synchronous call to" << methodName;
            return QVariant(-1);
        }
    }
    
private slots:
    void onReplicaInitialized() {
        qDebug() << "Replica initialized - remote methods now available";
    }
    
    void onReplicaStateChanged(QRemoteObjectReplica::State state, QRemoteObjectReplica::State oldState) {
        qDebug() << "Replica state changed from" << oldState << "to" << state;
        
        if (state == QRemoteObjectReplica::Valid) {
            // When replica becomes valid, we can discover its available methods
            const QMetaObject *metaObj = m_replica->metaObject();
            qDebug() << "Remote methods available:";
            for (int i = metaObj->methodOffset(); i < metaObj->methodCount(); ++i) {
                QMetaMethod method = metaObj->method(i);
                qDebug() << " - " << method.methodSignature();
            }
        }
    }
    
private:
    QRemoteObjectDynamicReplica *m_replica;
    QString m_token;
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
        
        // Create the Counter instance
        m_counter = new Counter(this);
        
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
        connect(m_timer, &QTimer::timeout, this, &ProcessHostApp::printHelloWorld);
        m_timer->start(3000); // 3000 ms = 3 seconds
        
        // Execute the initial action immediately
        QMetaObject::invokeMethod(this, &ProcessHostApp::onTimerTimeout, Qt::QueuedConnection);
        
        // Set up a timer to update the counter every 5 seconds
        m_counterTimer = new QTimer(this);
        connect(m_counterTimer, &QTimer::timeout, this, &ProcessHostApp::updateCounter);
        m_counterTimer->start(5000); // 5000 ms = 5 seconds
        
        // Connect to counter changes
        connect(m_counter, &Counter::countChanged, this, &ProcessHostApp::onCounterChanged);
        connect(m_counter, &Counter::multiplyResult, this, &ProcessHostApp::onMultiplyResult);
    }
    
    ~ProcessHostApp()
    {
        if (m_timer) {
            m_timer->stop();
        }
        
        if (m_counterTimer) {
            m_counterTimer->stop();
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
    
    void printHelloWorld()
    {
        qDebug() << "hello world";
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
    
    void updateCounter()
    {
        // Randomly choose which operation to perform on the counter
        int operation = QRandomGenerator::global()->bounded(4);
        static int factorCounter = 2;
        
        switch (operation) {
            case 0:
                m_counter->increment();
                break;
            case 1:
                m_counter->decrement();
                break;
            case 2:
                if (factorCounter > 10) {
                    m_counter->reset();
                    factorCounter = 2;
                } else {
                    QString requestId = QUuid::createUuid().toString(QUuid::WithoutBraces);
                    m_counter->multiplyWithCallback(requestId, factorCounter);
                    factorCounter++;
                }
                break;
            case 3:
                m_counter->reset();
                break;
        }
    }
    
    void onCounterChanged()
    {
        int currentCount = m_counter->count();
        qDebug() << "Counter value changed to:" << currentCount;
        
        // Log the counter change via the remote logger if available
        if (m_loggerReplica && m_loggerReplica->state() == QRemoteObjectReplica::Valid) {
            QString message = QString("Counter value changed to: %1").arg(currentCount);
            m_loggerReplica->logMessage(message);
        }
    }
    
    void onMultiplyResult(const QString &requestId, int result)
    {
        qDebug() << "Multiply operation completed - Request ID:" << requestId << "Result:" << result;
        
        // Log the multiply result via the remote logger if available
        if (m_loggerReplica && m_loggerReplica->state() == QRemoteObjectReplica::Valid) {
            QString message = QString("Multiply operation completed - Request ID: %1, Result: %2").arg(requestId).arg(result);
            m_loggerReplica->logMessage(message);
        }
    }

private:
    QString m_guid;
    QTimer *m_timer = nullptr;
    QTimer *m_counterTimer = nullptr;
    QRemoteObjectNode *m_remoteNode = nullptr;
    LoggerInterfaceReplica *m_loggerReplica = nullptr;
    Counter *m_counter = nullptr;
};

// Simple function that uses the calculator proxy
void useCalculator(DynamicAuthClientProxy *calculator)
{
    qDebug() << "=========================== Using calculator";
    
    // Call methods without token - the proxy handles authentication
    int a = 5;
    int b = 7;
    
    qDebug() << "=========================== Calling calculator add function with:" << a << "+" << b;
    
    // Use the proxy's simple API - token is handled transparently
    calculator->add(a, b, [a, b](int result) {
        if (result >= 0) {
            qDebug() << "=========================== Calculator result:" << a << "+" << b << "=" << result;
        } else {
            qDebug() << "=========================== Calculator call failed";
        }
    });
    
    // Test multiply too
    a = 6;
    b = 8;
    
    qDebug() << "=========================== Calling calculator multiply function with:" << a << "*" << b;
    
    calculator->multiply(a, b, [a, b](int result) {
        if (result >= 0) {
            qDebug() << "=========================== Calculator result:" << a << "*" << b << "=" << result;
        } else {
            qDebug() << "=========================== Calculator call failed";
        }
    });
    
    // Demonstrate a synchronous call
    a = 9;
    b = 11;
    qDebug() << "=========================== Synchronous calculator add function with:" << a << "+" << b;
    
    QVariantList args;
    args << a << b;
    int result = calculator->callSync("add", args).toInt();
    
    if (result >= 0) {
        qDebug() << "=========================== Synchronous calculator result:" << a << "+" << b << "=" << result;
    } else {
        qDebug() << "=========================== Synchronous calculator call failed";
    }
}

// Modify the main function to use the dynamic proxy
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
    
    // Connect to calculator service through registry
    QRemoteObjectNode node;
    node.connectToNode(QUrl(QStringLiteral("local:calculator")));
    
    // Acquire the dynamic replica of the Calculator
    QRemoteObjectDynamicReplica *calculatorReplica = node.acquireDynamic("Calculator");
    
    // Create a dynamic proxy that handles authentication transparently
    DynamicAuthClientProxy *calculatorProxy = new DynamicAuthClientProxy(calculatorReplica, TOKEN, &app);
    
    // Set up a timer to call the calculator function every 3 seconds
    QTimer *calculatorTimer = new QTimer(&app);
    QObject::connect(calculatorTimer, &QTimer::timeout, [calculatorProxy]() {
        useCalculator(calculatorProxy);
    });
    calculatorTimer->start(3000); // 3000 ms = 3 seconds
    
    // Create the application object for the counter part
    ProcessHostApp processHost(guid, registryUrl);
    
    // Wait for the application to exit
    return app.exec();
}

#include "process_host_counter.moc" 