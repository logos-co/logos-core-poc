#include "RemoteCounter.h"
#include "counter.h"
#include <QDebug>
#include <QTimer>

// Create a static counter to track initialization
static int s_instanceCount = 0;

// A wrapper class that provides -> syntax for Counter methods
// but forwards all calls to the remote object
class CounterProxy {
public:
    CounterProxy(QRemoteObjectDynamicReplica* replica) : m_replica(replica) {}
    
    // Provide forward methods that match Counter's interface
    void increment() {
        qDebug() << "CounterProxy: Forwarding increment() to remote object";
        QMetaObject::invokeMethod(m_replica, "increment", Qt::DirectConnection);
    }
    
    void decrement() {
        qDebug() << "CounterProxy: Forwarding decrement() to remote object";
        QMetaObject::invokeMethod(m_replica, "decrement", Qt::DirectConnection);
    }
    
    void reset() {
        qDebug() << "CounterProxy: Forwarding reset() to remote object";
        QMetaObject::invokeMethod(m_replica, "reset", Qt::DirectConnection);
    }
    
    void multiplyWithCallback(const QString& requestId, int factor) {
        qDebug() << "CounterProxy: Forwarding multiplyWithCallback() to remote object";
        QMetaObject::invokeMethod(m_replica, "multiplyWithCallback", 
                                  Qt::DirectConnection,
                                  Q_ARG(QString, requestId),
                                  Q_ARG(int, factor));
    }
    
private:
    QRemoteObjectDynamicReplica* m_replica;
};

RemoteCounter::RemoteCounter(QObject* parent) : QObject(parent) {
    int instanceId = ++s_instanceCount;
    qDebug() << "RemoteCounter[" << instanceId << "]: Constructor called";
    
    // Set up Remote Objects
    m_remoteNode = new QRemoteObjectNode(this);
    m_remoteNode->connectToNode(QUrl(QStringLiteral("local:registry")));
    
    // Get the counter from the registry
    m_counter = m_remoteNode->acquireDynamic("Counter");
    
    // Create the proxy wrapper for direct -> syntax
    m_counterProxy = new CounterProxy(m_counter);
    
    // Add debug to check when signals are connected
    qDebug() << "RemoteCounter[" << instanceId << "]: Setting up signal connections";
    
    // IMPORTANT: Connect to the countChanged signal with a direct connection
    // to ensure it's processed immediately
    connect(m_counter, SIGNAL(countChanged()), 
            this, SIGNAL(countChanged()), Qt::DirectConnection);
    
    // Debug when count changes - use a separate method for signal connection
    QObject::connect(m_counter, SIGNAL(countChanged()), this, SLOT(debugCountChanged()));
    
    // Use the proper connect syntax for lambdas
    connect(m_counter, &QRemoteObjectDynamicReplica::initialized, this, [this, instanceId]() {
        qDebug() << "RemoteCounter[" << instanceId << "]: Replica initialized";
        
        // Try to connect to countChanged once the replica is initialized
        // Try to look up the signal metaobject directly
        const QMetaObject* mo = m_counter->metaObject();
        int signalIndex = mo->indexOfSignal("countChanged()");
        if (signalIndex >= 0) {
            qDebug() << "RemoteCounter[" << instanceId << "]: Found countChanged signal at index" << signalIndex;
            
            // Extra connect with the SIGNAL macro just to be sure
            QObject::connect(m_counter, SIGNAL(countChanged()), this, SIGNAL(countChanged()));
            
            // Add a direct timer to poll for count changes instead of relying on signals
            QTimer* pollTimer = new QTimer(this);
            connect(pollTimer, &QTimer::timeout, this, [this, instanceId]() {
                static int lastCount = -1;
                int currentCount = m_counter->property("count").toInt();
                if (currentCount != lastCount) {
                    qDebug() << "RemoteCounter[" << instanceId << "]: Poll detected count change:" 
                             << lastCount << "->" << currentCount;
                    lastCount = currentCount;
                    emit countChanged();
                }
            });
            pollTimer->start(100); // Poll every 100ms
        } else {
            qDebug() << "RemoteCounter[" << instanceId << "]: ERROR - countChanged signal not found!";
        }
        
        // Explicitly check available properties and signals
        qDebug() << "RemoteCounter[" << instanceId << "]: Available properties:";
        for (int i = 0; i < mo->propertyCount(); ++i) {
            QMetaProperty prop = mo->property(i);
            qDebug() << "  - Property:" << prop.name();
        }
        
        qDebug() << "RemoteCounter[" << instanceId << "]: Available signals:";
        for (int i = 0; i < mo->methodCount(); ++i) {
            QMetaMethod method = mo->method(i);
            if (method.methodType() == QMetaMethod::Signal) {
                qDebug() << "  - Signal:" << method.methodSignature();
            }
        }
        
        // Connect to the multiplyResult signal from the remote object
        connect(m_counter, SIGNAL(multiplyResult(QString,int)), 
                this, SLOT(onMultiplyResult(QString,int)));
        
        emit initialized();
    });
    
    connect(m_counter, &QRemoteObjectReplica::stateChanged, 
            this, &RemoteCounter::connectionStateChanged);
    
    // Add a signal handler directly for debugging
    connect(m_counter, &QRemoteObjectReplica::stateChanged, this, [this, instanceId](QRemoteObjectReplica::State state, QRemoteObjectReplica::State) {
        qDebug() << "RemoteCounter[" << instanceId << "]: Connection state changed to:" << state;
        if (state == QRemoteObjectReplica::Valid) {
            qDebug() << "RemoteCounter[" << instanceId << "]: Connected to server, count =" << m_counter->property("count").toInt();
            // Refresh the UI with the initial count
            emit countChanged();
        }
    });
}

RemoteCounter::~RemoteCounter() {
    // Clean up the proxy
    delete m_counterProxy;
}

// Debug slot for countChanged
void RemoteCounter::debugCountChanged() {
    qDebug() << "RemoteCounter::debugCountChanged - Count =" << m_counter->property("count").toInt();
    // Also emit our own signal to ensure it propagates
    emit countChanged();
}

int RemoteCounter::count() const {
    if (isConnected()) {
        return m_counter->property("count").toInt();
    }
    return 0;
}

bool RemoteCounter::isConnected() const {
    return m_counter && m_counter->state() == QRemoteObjectReplica::Valid;
}

void RemoteCounter::increment() {
    if (isConnected()) {
        // Use the proxy with -> syntax
        m_counterProxy->increment();
    }
}

void RemoteCounter::decrement() {
    if (isConnected()) {
        m_counterProxy->decrement();
    }
}

void RemoteCounter::reset() {
    if (isConnected()) {
        m_counterProxy->reset();
    }
}

void RemoteCounter::multiply(int factor, std::function<void(int)> callback) {
    if (!isConnected()) {
        qDebug() << "Cannot multiply: not connected";
        return;
    }
    
    // Register the callback and get a request ID
    QString requestId = m_callbackManager.registerCallback(callback);
    
    // Call the remote method with the request ID via the proxy
    m_counterProxy->multiplyWithCallback(requestId, factor);
}

void RemoteCounter::onMultiplyResult(const QString& requestId, int result) {
    // Execute the callback for this request
    m_callbackManager.executeCallback(requestId, result);
}

// Include the MOC for the RemoteCounter class
#include "moc_RemoteCounter.cpp"