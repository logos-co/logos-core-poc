#include "Counter.h"
#include <QRemoteObjectReplica>

// This is a simple implementation that will get replicated with the real implementation from the server
Counter::Counter(QObject *parent) : QObject(parent), m_count(0) {
}

int Counter::count() const {
    return m_count;
}

void Counter::setCount(int count) {
    if (m_count != count) {
        m_count = count;
        emit countChanged();
    }
}

void Counter::increment() {
    // This is just a stub - the real implementation comes from the remote object
}

void Counter::decrement() {
    // This is just a stub - the real implementation comes from the remote object
}

void Counter::reset() {
    // This is just a stub - the real implementation comes from the remote object
}

void Counter::multiplyWithCallback(const QString &requestId, int factor) {
    // This is just a stub - the real implementation comes from the remote object
} 