#include "counter.h"
#include <QDebug>
#include <QTimer>
#include <QRandomGenerator>
#include <QDateTime>
#include <QVariant>
#include <QtMath>
#include <cmath>

Counter::Counter(QObject *parent) : QObject(parent), m_count(0) {
    qDebug() << "Counter created with initial count:" << m_count;
    
    // Initialize the operation registry
    m_operationRegistry = new ServerOperationRegistry(this);
    
    // Connect the registry signal to our slot
    connect(m_operationRegistry, &ServerOperationRegistry::operationResult,
            this, &Counter::onRegistryOperationResult);
    
    // Setup all operations
    setupOperations();
}

void Counter::setupOperations() {
    qDebug() << "Counter: Setting up operations with registry";
    
    // Register multiply operation - simple single return value
    m_operationRegistry->registerOperation<int, int>("multiply", 
        [this](int factor) -> int {
            qDebug() << "Registry multiply operation: count=" << m_count << "factor=" << factor;
            return m_count * factor;
        });
    
    // Register add operation - returns multiple values using tuple
    m_operationRegistry->registerMultiReturnOperation<int, QString, QDateTime, int>("add",
        [this](int a, int b) -> std::tuple<int, QString, QDateTime> {
            int result = m_count + a + b;
            qDebug() << "Registry add operation:" << m_count << "+" << a << "+" << b << "=" << result;
            return std::make_tuple(result, QString("Addition completed"), QDateTime::currentDateTime());
        });
    
    // Register advanced multiply operation
    m_operationRegistry->registerMultiReturnOperation<double, int, double, int, QString>("multiply_advanced",
        [this](int factor, double multiplier) -> std::tuple<double, int, double, int, QString> {
            double result = m_count * factor * multiplier;
            qDebug() << "Registry advanced multiply:" << m_count << "*" << factor << "*" << multiplier << "=" << result;
            return std::make_tuple(result, factor, multiplier, m_count, QString("Advanced multiplication"));
        });
    
    // Register power operation
    m_operationRegistry->registerMultiReturnOperation<double, int, int, QString>("power",
        [this](int exponent) -> std::tuple<double, int, int, QString> {
            double result = qPow(m_count, exponent);
            qDebug() << "Registry power operation:" << m_count << "^" << exponent << "=" << result;
            return std::make_tuple(result, exponent, m_count, QString("Power calculation"));
        });
    
    // Register string operation - variable arguments handled as QVariantList
    m_operationRegistry->registerOperation("string_operation",
        [this](const QVariantList& args) -> QVariantList {
            QString result = QString("Count: %1").arg(m_count);
            for (const QVariant& arg : args) {
                result += " | " + arg.toString();
            }
            qDebug() << "Registry string operation result:" << result;
            return QVariantList{result, args.size(), m_count};
        });
    
    // Register statistics operation - takes a list, returns multiple stats
    m_operationRegistry->registerOperation("statistics",
        [this](const QVariantList& args) -> QVariantList {
            if (args.isEmpty()) {
                qDebug() << "Registry statistics: no arguments provided";
                return QVariantList{QString("No arguments provided")};
            }
            
            QVariantList numbers = args[0].toList();
            if (numbers.isEmpty()) {
                // Use current count if no numbers provided
                numbers << m_count;
            }
            
            double sum = 0;
            double min = numbers[0].toDouble();
            double max = numbers[0].toDouble();
            
            for (const QVariant& num : numbers) {
                double val = num.toDouble();
                sum += val;
                min = qMin(min, val);
                max = qMax(max, val);
            }
            
            double average = sum / numbers.size();
            qDebug() << "Registry statistics - Sum:" << sum << "Avg:" << average << "Min:" << min << "Max:" << max;
            return QVariantList{sum, average, min, max, numbers.size(), QString("Statistics calculated")};
        });
    
    qDebug() << "Counter: Registered operations:" << m_operationRegistry->getRegisteredOperations();
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

void Counter::executeOperationWithCallback(const QString &requestId, 
                                         const QString &operation, 
                                         const QVariantList &arguments) {
    qDebug() << "Counter::executeOperationWithCallback - Operation:" << operation 
             << "Arguments:" << arguments << "RequestId:" << requestId;
    
    // Delegate to the operation registry
    m_operationRegistry->executeOperation(requestId, operation, arguments);
}

void Counter::onRegistryOperationResult(const QString &requestId, const QVariantList &results) {
    qDebug() << "Counter::onRegistryOperationResult - RequestId:" << requestId << "Results:" << results;
    // Forward the result to our signal
    emit operationResult(requestId, results);
}

// Include for MOC generated code
#include "moc_counter.cpp" 