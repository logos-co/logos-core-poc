#pragma once

#include <QObject>
#include <QRemoteObjectNode>
#include <QRemoteObjectReplica>
#include <functional>
#include "RemoteCallbackManager.h"

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
    
    // Async operation with callback
    void multiply(int factor, std::function<void(int)> callback);
    
    // Connection status
    bool isConnected() const;
    
signals:
    void countChanged();
    void connectionStateChanged(QRemoteObjectReplica::State newState, QRemoteObjectReplica::State oldState);
    void initialized();
    
private slots:
    void onMultiplyResult(const QString& requestId, int result);
    void debugCountChanged();
    
private:
    QRemoteObjectNode* m_remoteNode;
    QRemoteObjectDynamicReplica* m_counter;
    RemoteCallbackManager<int> m_callbackManager;
    CounterProxy* m_counterProxy;
};
