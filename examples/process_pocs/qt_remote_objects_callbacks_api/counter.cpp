#include "counter.h"
#include <QDebug>
#include <QTimer>
#include <QRandomGenerator>

Counter::Counter(QObject *parent) : QObject(parent), m_count(0) {
    qDebug() << "Counter created with initial count:" << m_count;
}

int Counter::count() const { 
    return m_count; 
}

void Counter::setCount(int count) {
    if (m_count != count) {
        m_count = count;
        qDebug() << "Counter::setCount - Emitting countChanged signal with new value:" << m_count;
        emit countChanged();
    }
}

void Counter::increment() {
    qDebug() << "Counter::increment - Current count:" << m_count;
    setCount(m_count + 2);
    qDebug() << "Counter incremented to:" << m_count;
}

void Counter::decrement() {
    qDebug() << "Counter::decrement - Current count:" << m_count;
    setCount(m_count - 1);
    qDebug() << "Counter decremented to:" << m_count;
}

void Counter::reset() {
    qDebug() << "Counter::reset - Current count:" << m_count;
    setCount(0);
    qDebug() << "Counter reset to:" << m_count;
}

void Counter::multiplyWithCallback(const QString &requestId, int factor) {
    int result = m_count * factor;
    qDebug() << "Multiplying counter value" << m_count << "by factor" << factor 
             << "= result:" << result << "requestId:" << requestId;

    // simulate async delayed response
    QTimer::singleShot(QRandomGenerator::global()->bounded(2000, 5000), this, [this, result, requestId]() {
        qDebug() << "Emitting multiplyResult signal with result:" << result << "requestId:" << requestId;
        emit multiplyResult(requestId, result);
    });
}

// Include for MOC generated code
#include "moc_counter.cpp" 