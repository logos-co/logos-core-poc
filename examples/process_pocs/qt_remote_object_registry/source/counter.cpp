#include "counter.h"
#include <QDebug>

Counter::Counter(QObject *parent)
    : CounterSimpleSource(parent)
{
    // Initialize with 0
    setValue(0);
    qDebug() << "Counter initialized with value:" << value();
}

void Counter::increment()
{
    int newValue = value() + 1;
    setValue(newValue);
    qDebug() << "Counter incremented to:" << newValue;
    emit valueChanged(newValue);
}

void Counter::decrement()
{
    int newValue = value() - 1;
    setValue(newValue);
    qDebug() << "Counter decremented to:" << newValue;
    emit valueChanged(newValue);
} 