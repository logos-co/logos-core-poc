#include "MyRemoteDB.h"
#include <QDebug>

MyRemoteDB::MyRemoteDB(QObject *parent) : QObject(parent)
{
    // Constructor - no sample data needed anymore
}

QString MyRemoteDB::foo(const QString& a)
{
    // Emit the signal with the parameter value
    emit fooTriggered(a);
    
    return "response: " + a;
}
