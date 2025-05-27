#include "Counter.h"

Counter::Counter(QObject *parent) : QObject(parent), m_count(0) {
    qDebug() << "Counter created with initial count:" << m_count;
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
    setCount(m_count + 2);
    qDebug() << "Counter incremented to:" << m_count;
}

void Counter::decrement() {
    setCount(m_count - 1);
    qDebug() << "Counter decremented to:" << m_count;
}

void Counter::reset() {
    setCount(0);
    qDebug() << "Counter reset to:" << m_count;
}

void Counter::multiplyWithCallback(const QString &requestId, int factor) {
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
