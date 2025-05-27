#include <QCoreApplication>
#include <QRemoteObjectHost>
#include <QDebug>
#include "counter.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    // Create a host node to share objects
    QRemoteObjectHost srcNode(QUrl(QStringLiteral("local:registry")));
    
    // Create and share the counter
    Counter *counter = new Counter(&srcNode);
    srcNode.enableRemoting(counter, "Counter");
    
    qDebug() << "Counter Server started. Registry available at:" << "local:registry";
    qDebug() << "Press Ctrl+C to quit";

    return app.exec();
}

// Include for MOC generated code
#include "server.moc" 