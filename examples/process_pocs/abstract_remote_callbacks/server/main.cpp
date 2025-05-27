#include <QCoreApplication>
#include <QDebug>
#include <QRemoteObjectHost>
#include <QUrl>
#include "MyRemoteDB.h"
#include <QTimer>
#include <QObject>
#include <QVariant>
#include <QUuid>
#include <QDateTime>
#include <QRandomGenerator>

// MyProxy class definition
class MyProxy : public QObject
{
    Q_OBJECT

public:
    explicit MyProxy(QObject *parent = nullptr) : QObject(parent) {}

public slots:
    // Updated getMessages to accept a requestId and actually do something
    void getMessages(const QString &requestId) {
        qDebug() << "MyProxy::getMessages called with requestId:" << requestId;
        
        // Simulate some async work with a timer
        QTimer::singleShot(500, [this, requestId]() {
            // Generate some sample data
            QVariantList messages;
            messages << "Message 1: Hello from server!";
            messages << "Message 2: Current time is " + QDateTime::currentDateTime().toString();
            messages << "Message 3: Random number: " + QString::number(QRandomGenerator::global()->bounded(1000));
            
            qDebug() << "Emitting callbackResponse for requestId:" << requestId;
            emit callbackResponse(requestId, messages);
        });
    }
    
    // Keep the old parameterless version for backward compatibility
    //void getMessages() {
    //    qDebug() << "MyProxy::getMessages called without requestId - generating one";
    //    QString requestId = QUuid::createUuid().toString(QUuid::WithoutBraces);
    //    getMessages(requestId);
    //}

signals:
    void callbackResponse(const QString &requestId, const QVariantList &data);
};

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    qDebug() << "Hello World from Qt Server!";
    qDebug() << "Server is running... Press Ctrl+C to exit";

    // Create QRemoteObjectHost with local registry
    QRemoteObjectHost srcNode2(QUrl(QStringLiteral("local:registry")));
    
    // Initialize the MyRemoteDB object
    MyRemoteDB myDB;
    
    // Initialize the MyProxy object
    MyProxy myProxy;
    
    // Enable remoting for the MyRemoteDB object
    srcNode2.enableRemoting(&myDB, "MyRemoteDB");
    
    // Enable remoting for the MyProxy object
    srcNode2.enableRemoting(&myProxy, "MyProxy");
    
    qDebug() << "MyRemoteDB object is now available for remote access";
    qDebug() << "MyProxy object is now available for remote access";
    
    // Test the MyProxy periodically
    //QTimer *proxyTimer = new QTimer();
    //QObject::connect(proxyTimer, &QTimer::timeout, [&myProxy]() {
    //    qDebug() << "Server: Testing MyProxy.getMessages()";
    //    myProxy.getMessages("server-test-" + QString::number(QDateTime::currentMSecsSinceEpoch()));
    //});
    //proxyTimer->start(10000); // Test every 10 seconds
    
    // Call getMessages() every 5 seconds
    QTimer *timer = new QTimer();
    QObject::connect(timer, &QTimer::timeout, [&myDB]() {
        // qDebug() << "Calling getMessages() from server...";
        // myDB.getMessages();
    });
    timer->start(3000);

    return app.exec();
}

#include "main.moc" 