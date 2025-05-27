#include <QCoreApplication>
#include <QRemoteObjectHost>
#include <QDebug>
#include "counter.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    
    // Connect to registry at TCP port 45000
    // 450001 -> listening address
    // 450000 -> registry address
    QRemoteObjectHost sourceNode(QUrl("tcp://localhost:45001"), QUrl("tcp://localhost:45000"));
    
    // Create a Counter instance
    Counter counter;
    
    // Enable remoting for the Counter object
    sourceNode.enableRemoting(&counter, "Counter");
    
    qDebug() << "Source is running and connected to registry";
    qDebug() << "Counter object is available as 'Counter'";
    qDebug() << "Press Ctrl+C to quit";
    
    return app.exec();
} 