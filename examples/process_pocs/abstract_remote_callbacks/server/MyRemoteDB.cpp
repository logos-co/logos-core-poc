#include "MyRemoteDB.h"
#include <QDebug>

MyRemoteDB::MyRemoteDB(QObject *parent) : QObject(parent)
{
    // Initialize with some sample data
    messages_objects[1] = "Hello World";
    messages_objects[2] = "Qt is awesome";
    messages_objects[3] = "Remote objects are cool";
}

QString MyRemoteDB::foo(const QString& a)
{
    return "response: " + a;
}

bool MyRemoteDB::getMessage(int id)
{
    if (messages_objects.contains(id)) {
        QString message = messages_objects[id];
        emit singleMessageReceived(id, message);
        return true;
    } else {
        QString errorMessage = QString("Message with id %1 not found").arg(id);
        emit singleMessageReceived(id, errorMessage);
        return false;
    }
}

bool MyRemoteDB::getMessageWithRequestId(const QString& requestId, int id)
{
    if (messages_objects.contains(id)) {
        QString message = messages_objects[id];
        emit singleMessageWithRequestId(requestId, message);
        return true;
    } else {
        QString errorMessage = QString("Message with id %1 not found").arg(id);
        emit singleMessageWithRequestId(requestId, errorMessage);
        return false;
    }
}

bool MyRemoteDB::getMessages()
{
    // Iterate through all messages and emit signal for each
    QMap<int, QString>::const_iterator i = messages_objects.constBegin();
    while (i != messages_objects.constEnd()) {
        QString formattedMessage = QString("ID %1: %2").arg(i.key()).arg(i.value());
        emit messageReceived(formattedMessage);
        ++i;
    }
    
    // Return true to indicate success
    return true;
}

void MyRemoteDB::addMessage(int id, const QString& message)
{
    messages_objects[id] = message;
}
