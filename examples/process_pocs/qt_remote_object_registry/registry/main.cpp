#include <QCoreApplication>
#include <QRemoteObjectRegistryHost>
#include <QDebug>

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    
    // Create registry host on TCP port 45000
    QRemoteObjectRegistryHost registryHost(QUrl("tcp://localhost:45000"));
    
    qDebug() << "Registry is running at tcp://localhost:45000";
    qDebug() << "Press Ctrl+C to quit";
    
    return app.exec();
} 