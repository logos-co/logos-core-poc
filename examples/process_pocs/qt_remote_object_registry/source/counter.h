#ifndef COUNTER_H
#define COUNTER_H

#include "rep_counter_source.h"

class Counter : public CounterSimpleSource
{
    Q_OBJECT
public:
    explicit Counter(QObject *parent = nullptr);
    
public slots:
    void increment() override;
    void decrement() override;
};

#endif // COUNTER_H 