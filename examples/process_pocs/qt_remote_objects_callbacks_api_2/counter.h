#ifndef COUNTER_H
#define COUNTER_H

#include <QObject>
#include <QString>
#include <QVariantList>
#include "include/ServerOperationRegistry.h"

class Counter : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int count READ count WRITE setCount NOTIFY countChanged)

public:
    Counter(QObject *parent = nullptr);

    int count() const;
    void setCount(int count);

public slots:
    void increment();
    void decrement();
    void reset();
    void multiplyWithCallback(const QString &requestId, int factor);
    
    // New abstract method that uses the operation registry
    void executeOperationWithCallback(const QString &requestId, 
                                    const QString &operation, 
                                    const QVariantList &arguments);

signals:
    void countChanged();
    void multiplyResult(const QString &requestId, int result);
    
    // Abstract signal for operation results
    void operationResult(const QString &requestId, const QVariantList &results);

private slots:
    // Handle results from the operation registry
    void onRegistryOperationResult(const QString &requestId, const QVariantList &results);

private:
    void setupOperations(); // Initialize the operation registry
    
    int m_count;
    ServerOperationRegistry* m_operationRegistry;
};

#endif // COUNTER_H
