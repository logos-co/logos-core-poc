#ifndef MODULECOMMUNICATION_H
#define MODULECOMMUNICATION_H

#include <QObject>
#include <QDebug>

class ModuleCommunication : public QObject
{
    Q_OBJECT

public:
    ModuleCommunication(QObject *parent = nullptr);

public slots:
    void sayHello();
    void sendMessageToClients(const QString &message);

signals:
    // Signal that clients can connect to - server uses this to push messages to clients
    void messageReceived(const QString &message);

private:
    // No private members for now
};

#endif // MODULECOMMUNICATION_H 