#ifndef MYREMOTEDB_H
#define MYREMOTEDB_H

#include <QObject>
#include <QString>

class MyRemoteDB : public QObject
{
    Q_OBJECT

public:
    explicit MyRemoteDB(QObject *parent = nullptr);
    
public slots:
    // Method that returns "response: " + a
    QString foo(const QString& a);

signals:
    // Signal emitted when foo is called
    void fooTriggered(const QString& value);
};

#endif // MYREMOTEDB_H 