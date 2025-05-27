#include <QCoreApplication>
#include <QRemoteObjectHost>
#include <QDebug>
#include "Counter.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    // Create a host node to share objects
    QRemoteObjectHost srcNode(QUrl(QStringLiteral("local:registry")));
    
    // Connect to the remoteObjectAdded signal to detect client connections
    QObject::connect(&srcNode, &QRemoteObjectHost::remoteObjectAdded,
                   [](const QRemoteObjectSourceLocation &entry) {
                       qDebug() << "Client connected for object:" << entry.objectName;
                   });

    // Create and share the counter
    Counter *counter = new Counter(&srcNode);
    srcNode.enableRemoting(counter, "Counter");

    qDebug() << "Counter Server started. Registry available at:" << "local:registry";
    qDebug() << "Press Ctrl+C to quit";

    return app.exec();
}

#include "server.moc" 