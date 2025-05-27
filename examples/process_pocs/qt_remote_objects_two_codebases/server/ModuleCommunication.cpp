#include "ModuleCommunication.h"

ModuleCommunication::ModuleCommunication(QObject *parent) : QObject(parent) {
    qDebug() << "ModuleCommunication instance created";
}

void ModuleCommunication::sayHello() {
    qDebug() << "Hello";
}

void ModuleCommunication::sendMessageToClients(const QString &message) {
    qDebug() << "Server sending message to clients:" << message;
    emit messageReceived(message);
} 