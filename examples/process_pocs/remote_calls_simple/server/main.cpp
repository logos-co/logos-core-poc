#include <QCoreApplication>
#include <QDebug>
#include <QRemoteObjectHost>
#include <QUrl>
#include "MyRemoteDB.h"
#include <QTimer>

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    qDebug() << "Hello World from Qt Server!";
    qDebug() << "Server is running... Press Ctrl+C to exit";

    // Create QRemoteObjectHost with local registry
    QRemoteObjectHost srcNode2(QUrl(QStringLiteral("local:registry")));
    
    // Initialize the MyRemoteDB object
    MyRemoteDB myDB;
    
    // Enable remoting for the MyRemoteDB object
    srcNode2.enableRemoting(&myDB, "MyRemoteDB");
    
    qDebug() << "MyRemoteDB object is now available for remote access";

    return app.exec();
}

#include "main.moc" 