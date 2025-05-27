#ifndef COUNTER_H
#define COUNTER_H

#include <QObject>
#include <QString>

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

signals:
    void countChanged();
    void multiplyResult(const QString &requestId, int result);

private:
    int m_count;
};

#endif // COUNTER_H
