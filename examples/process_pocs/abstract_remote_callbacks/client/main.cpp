#include <QCoreApplication>
#include <QDebug>
#include <QRemoteObjectNode>
#include <QRemoteObjectPendingCall>
#include <QTimer>
#include <QUuid>
#include <functional>
#include <QHash>

// Client-side MyProxy wrapper class that provides callback-style interface
class MyProxy : public QObject
{
    Q_OBJECT
    
private:
    QRemoteObjectDynamicReplica *m_remoteProxy;
    QHash<QString, std::function<void(const QVariantList&)>> m_callbacks;
    
public:
    explicit MyProxy(QRemoteObjectDynamicReplica *remoteProxy, QObject *parent = nullptr) 
        : QObject(parent), m_remoteProxy(remoteProxy) 
    {
        // Connect to the callbackResponse signal from the remote MyProxy
        bool connected = connect(m_remoteProxy, SIGNAL(callbackResponse(QString,QVariantList)), 
                this, SLOT(onCallbackResponse(QString,QVariantList)));
        
        if (connected) {
            qDebug() << "Successfully connected to callbackResponse signal";
        } else {
            qDebug() << "Failed to connect to callbackResponse signal";
        }
        
        // Also check the state of the remote proxy
        qDebug() << "Remote proxy state:" << m_remoteProxy->state();
        qDebug() << "Remote proxy is valid:" << (m_remoteProxy->state() == QRemoteObjectReplica::Valid);
    }
    
    // The callback-style getMessages method you wanted
    void getMessages(std::function<void(const QVariantList&)> callback) {
        // Generate a unique request ID
        QString requestId = QUuid::createUuid().toString(QUuid::WithoutBraces);
        
        // Store the callback for this request ID
        m_callbacks[requestId] = callback;
        
        qDebug() << "Calling remote MyProxy.getMessages() with requestId:" << requestId;
        
        // Call the remote getMessages method WITH the requestId parameter
        // Since the server method returns void, don't expect a QRemoteObjectPendingCall
        QMetaObject::invokeMethod(m_remoteProxy, "getMessages", 
                                Q_ARG(QString, requestId));
        
        // The real response will come via the callbackResponse signal
    }
    
public slots:
    void onCallbackResponse(const QString& requestId, const QVariantList& data) {
        qDebug() << "Received callback response for requestId:" << requestId << "with data:" << data;
        
        // Find and call the callback for this request ID
        if (m_callbacks.contains(requestId)) {
            auto callback = m_callbacks[requestId];
            callback(data);
            
            // TODO: this will depend if it's a event or a one time return
            // Remove the callback after use (one-time use)
            m_callbacks.remove(requestId);
        } else {
            qDebug() << "No callback found for requestId:" << requestId;
        }
    }
};

// Simple message receiver class
class MessageReceiver : public QObject
{
    Q_OBJECT
public slots:
    void onMessageReceived(const QString& message) {
        qDebug() << "Received message:" << message;
    }
    
    void onSingleMessageReceived(int id, const QString& message) {
        qDebug() << "Received single message - ID:" << id << "Message:" << message;
    }
    
    void onSingleMessageWithRequestId(const QString& requestId, const QString& message) {
        qDebug() << "Received message with request ID:" << requestId << "Message:" << message;
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

// Function to call getMessage(1)
void callMessage(QRemoteObjectDynamicReplica *myDB, MessageReceiver *receiver) {
    // Connect to the singleMessageReceived signal
    bool connected = QObject::connect(myDB, SIGNAL(singleMessageReceived(int,QString)), 
                                    receiver, SLOT(onSingleMessageReceived(int,QString)));
    
    if (connected) {
        qDebug() << "Successfully connected to singleMessageReceived signal";
    } else {
        qDebug() << "Failed to connect to singleMessageReceived signal";
    }
    
    qDebug() << "Calling getMessage(1)...";
    QRemoteObjectPendingCall messageCall;
    QMetaObject::invokeMethod(myDB, "getMessage", 
        Q_RETURN_ARG(QRemoteObjectPendingCall, messageCall),
        Q_ARG(int, 1));
    
    messageCall.waitForFinished();
    
    // Check if the call was successful
    bool success = messageCall.returnValue().toBool();
    if (success) {
        qDebug() << "getMessage(1) call successful - message should be received via signal";
    } else {
        qDebug() << "getMessage(1) call failed";
    }
}

// Function to call getMessageWithRequestId
void callMessageWithRequestId(QRemoteObjectDynamicReplica *myDB, MessageReceiver *receiver) {
    // Connect to the singleMessageWithRequestId signal
    bool connected = QObject::connect(myDB, SIGNAL(singleMessageWithRequestId(QString,QString)), 
                                    receiver, SLOT(onSingleMessageWithRequestId(QString,QString)));
    
    if (connected) {
        qDebug() << "Successfully connected to singleMessageWithRequestId signal";
    } else {
        qDebug() << "Failed to connect to singleMessageWithRequestId signal";
    }
    
    QString requestId = QUuid::createUuid().toString(QUuid::WithoutBraces);
    qDebug() << "Calling getMessageWithRequestId(" << requestId << ", 2)...";
    QRemoteObjectPendingCall messageCall;
    QMetaObject::invokeMethod(myDB, "getMessageWithRequestId", 
        Q_RETURN_ARG(QRemoteObjectPendingCall, messageCall),
        Q_ARG(QString, requestId),
        Q_ARG(int, 2));
    
    messageCall.waitForFinished();
    
    // Check if the call was successful
    bool success = messageCall.returnValue().toBool();
    if (success) {
        qDebug() << "getMessageWithRequestId call successful - message should be received via signal";
    } else {
        qDebug() << "getMessageWithRequestId call failed";
    }
}

// Function to call the getMessages method and handle the signals
void callMessages(QRemoteObjectDynamicReplica *myDB, MessageReceiver *receiver) {
    // Connect to the messageReceived signal using string-based connection
    bool connected = QObject::connect(myDB, SIGNAL(messageReceived(QString)), 
                                    receiver, SLOT(onMessageReceived(QString)));
    
    if (connected) {
        qDebug() << "Successfully connected to messageReceived signal";
    } else {
        qDebug() << "Failed to connect to messageReceived signal";
    }
    
    qDebug() << "Calling getMessages() asynchronously...";
    QRemoteObjectPendingCall messagesCall;
    QMetaObject::invokeMethod(myDB, "getMessages", 
        Q_RETURN_ARG(QRemoteObjectPendingCall, messagesCall));
    
    messagesCall.waitForFinished();
    
    // Check if the call was successful
    bool success = messagesCall.returnValue().toBool();
    if (success) {
        qDebug() << "getMessages() call successful - messages should be received via signals";
    } else {
        qDebug() << "getMessages() call failed";
    }
}

// Function to demonstrate the MyProxy callback pattern
void testMyProxy(QRemoteObjectDynamicReplica *remoteProxy) {
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
    
    // Acquire the remote MyProxy object
    QRemoteObjectDynamicReplica *remoteProxy = remoteNode.acquireDynamic("MyProxy");
    
    // Create message receiver
    MessageReceiver receiver;
    
    // Wait for connection and call methods
    QTimer::singleShot(1000, [myDB, remoteProxy, &receiver]() {
        //if (myDB && myDB->state() == QRemoteObjectReplica::Valid) {
        //    qDebug() << "Connected to remote MyRemoteDB object";
        //    
        //    // Call foo method
        //    callFoo(myDB);
        //    
        //    // Call getMessage(1) method
        //    callMessage(myDB, &receiver);
        //    
        //    // Call getMessageWithRequestId method
        //    callMessageWithRequestId(myDB, &receiver);
        //    
        //    // Call getMessages method
        //    callMessages(myDB, &receiver);
        //    
        //} else {
        //    qDebug() << "Failed to connect to remote MyRemoteDB object";
        //}
        
        if (remoteProxy && remoteProxy->state() == QRemoteObjectReplica::Valid) {
            qDebug() << "Connected to remote MyProxy object";
            
            // Test the MyProxy callback pattern
            // testMyProxy(remoteProxy);

            // Create MyProxy as a static object so it persists beyond function scope
            static MyProxy *myProxy = nullptr;
            if (myProxy) {
                delete myProxy; // Clean up previous instance
            }
            myProxy = new MyProxy(remoteProxy);
            
            // Check if the signal connection was successful
            qDebug() << "Testing signal connection for MyProxy...";
            
            // This is the interface you wanted!
            myProxy->getMessages([](const QVariantList& messages) {
                qDebug() << "Callback received messages:";
                for (const auto& message : messages) {
                    qDebug() << "  hello" << message.toString();
                }
            });




            
        } else {
            qDebug() << "Failed to connect to remote MyProxy object";
        }
    });

    return app.exec();
}

#include "main.moc" 