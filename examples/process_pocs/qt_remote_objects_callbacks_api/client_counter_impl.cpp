#include "counter.h"
#include <QDebug>

// Minimal implementation of Counter class for the client
// These methods should never be directly called in practice,
// they just satisfy the linker requirements

Counter::Counter(QObject *parent) : QObject(parent), m_count(0) {
    // This constructor should never be called by client code
    qDebug() << "WARNING: Counter constructor called in client code";
}

int Counter::count() const {
    // This should never be called in client code
    qDebug() << "WARNING: Direct call to Counter::count() in client code";
    return m_count;
}

void Counter::setCount(int count) {
    // This should never be called in client code
    qDebug() << "WARNING: Direct call to Counter::setCount() in client code";
    if (m_count != count) {
        m_count = count;
        emit countChanged();
    }
}

void Counter::increment() {
    // This should never be called in client code
    qDebug() << "WARNING: Direct call to Counter::increment() in client code";
    // Do nothing, as this is only to satisfy the linker
}

void Counter::decrement() {
    // This should never be called in client code
    qDebug() << "WARNING: Direct call to Counter::decrement() in client code";
    // Do nothing, as this is only to satisfy the linker
}

void Counter::reset() {
    // This should never be called in client code
    qDebug() << "WARNING: Direct call to Counter::reset() in client code";
    // Do nothing, as this is only to satisfy the linker
}

void Counter::multiplyWithCallback(const QString &requestId, int factor) {
    // This should never be called in client code
    qDebug() << "WARNING: Direct call to Counter::multiplyWithCallback() in client code";
    // Do nothing, as this is only to satisfy the linker
}
