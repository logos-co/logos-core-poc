#include <QCoreApplication>
#include <QDebug>
#include <QRemoteObjectNode>
#include <QRemoteObjectPendingCall>
#include <QTimer>

// Signal receiver class with proper slot
class SignalReceiver : public QObject
{
    Q_OBJECT
public slots:
    void onFooTriggered(const QString& value) {
        qDebug() << "Signal received! fooTriggered with value:" << value;
    }
};

// Function to call the foo method
void callFoo(QRemoteObjectDynamicReplica *myDB) {
    qDebug() << "Calling foo() method...";
    QRemoteObjectPendingCall fooCall;
    QMetaObject::invokeMethod(myDB, "foo", 
        Q_RETURN_ARG(QRemoteObjectPendingCall, fooCall),
        Q_ARG(QString, "Hello from client!"));
    
    fooCall.waitForFinished();
    qDebug() << "Result from foo():" << fooCall.returnValue().toString();
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    qDebug() << "Hello World from Qt Client!";
    qDebug() << "Client is running... Press Ctrl+C to exit";

    // Create QRemoteObjectNode and connect to the registry
    QRemoteObjectNode remoteNode;
    remoteNode.connectToNode(QUrl(QStringLiteral("local:registry")));
    
    // Acquire the remote MyRemoteDB object
    QRemoteObjectDynamicReplica *myDB = remoteNode.acquireDynamic("MyRemoteDB");
    
    // Create signal receiver
    SignalReceiver receiver;
    
    // Wait for connection and call methods
    QTimer::singleShot(1000, [myDB, &receiver]() {
        if (myDB && myDB->state() == QRemoteObjectReplica::Valid) {
            qDebug() << "Connected to remote MyRemoteDB object";
            
            // Connect to the fooTriggered signal using SLOT
            bool connected = QObject::connect(myDB, SIGNAL(fooTriggered(QString)), 
                                            &receiver, SLOT(onFooTriggered(QString)));
            
            if (connected) {
                qDebug() << "Successfully connected to fooTriggered signal";
            } else {
                qDebug() << "Failed to connect to fooTriggered signal";
            }
            
            // Call foo method - this will trigger the signal
            callFoo(myDB);
            
        } else {
            qDebug() << "Failed to connect to remote MyRemoteDB object";
        }
    });

    return app.exec();
}

#include "main.moc" 