#pragma once

#include <QObject>
#include <QRemoteObjectNode>
#include <QRemoteObjectReplica>
#include <functional>
#include "RemoteCallbackManager.h"
#include "AbstractRemoteCallbackManager.h"

// Forward declaration
class CounterProxy;

class RemoteCounter : public QObject {
    Q_OBJECT

public:
    // Constructor sets up the connection to the remote object
    RemoteCounter(QObject* parent = nullptr);
    // Destructor to clean up resources
    ~RemoteCounter();
    
    // Simple method wrappers
    void increment();
    void decrement();
    void reset();
    
    // Get the current count value
    int count() const;
    
    // Async operation with callback (original specific method)
    void multiply(int factor, std::function<void(int)> callback);
    
    // New abstract method that can handle any operation with any arguments
    void executeOperation(const QString& operation, const QVariantList& arguments, 
                         std::function<void(const QVariantList&)> callback);
    
    // Template method for type-safe callbacks
    template<typename... Args>
    void executeOperation(const QString& operation, const QVariantList& arguments,
                         std::function<void(Args...)> callback) {
        QString requestId = m_abstractCallbackManager.registerTypedCallback(callback);
        m_counterProxy->executeOperationWithCallback(requestId, operation, arguments);
    }
    
    // Connection status
    bool isConnected() const;
    
signals:
    void countChanged();
    void connectionStateChanged(QRemoteObjectReplica::State newState, QRemoteObjectReplica::State oldState);
    void initialized();
    
private slots:
    void onMultiplyResult(const QString& requestId, int result);
    void onOperationResult(const QString& requestId, const QVariantList& results);
    void debugCountChanged();
    
private:
    QRemoteObjectNode* m_remoteNode;
    QRemoteObjectDynamicReplica* m_counter;
    RemoteCallbackManager<int> m_callbackManager;
    AbstractRemoteCallbackManager m_abstractCallbackManager;
    CounterProxy* m_counterProxy;
};
