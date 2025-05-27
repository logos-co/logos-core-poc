#ifndef MYREMOTEDB_H
#define MYREMOTEDB_H

#include <QObject>
#include <QString>
#include <QMap>
#include <QList>

class MyRemoteDB : public QObject
{
    Q_OBJECT

public:
    explicit MyRemoteDB(QObject *parent = nullptr);
    
public slots:
    // Method that returns "response: " + a
    QString foo(const QString& a);
    
    // Method to get a specific message by id - now async, returns bool for success
    bool getMessage(int id);
    
    // Method to get a specific message by id with request ID - async, returns bool for success
    bool getMessageWithRequestId(const QString& requestId, int id);
    
    // Method to get all messages asynchronously - returns bool for success
    bool getMessages();
    
    // Method to add a message (helper for testing)
    void addMessage(int id, const QString& message);

signals:
    // Signal emitted for each message when getMessages is called
    void messageReceived(const QString& message);
    
    // Signal emitted when a single message is retrieved
    void singleMessageReceived(int id, const QString& message);
    
    // Signal emitted when a single message is retrieved with request ID
    void singleMessageWithRequestId(const QString& requestId, const QString& message);

private:
    // Data structure to store messages: int -> string mapping
    QMap<int, QString> messages_objects;
};

#endif // MYREMOTEDB_H 