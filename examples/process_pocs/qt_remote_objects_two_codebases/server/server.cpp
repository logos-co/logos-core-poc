#include <QCoreApplication>
#include <QRemoteObjectHost>
#include <QDebug>
#include <QTimer>
#include <QRandomGenerator>
#include <QUuid>
#include <QDateTime>
#include <QLocalServer>
#include <QLocalSocket>
#include <QBuffer>
#include <QDataStream>
#include "Counter.h"
#include "ModuleCommunication.h"

class LocalIPCServer : public QObject
{
    Q_OBJECT

public:
    LocalIPCServer(QObject *parent = nullptr) : QObject(parent) {
        m_server = new QLocalServer(this);
        
        // Remove any existing socket file
        QLocalServer::removeServer("registry");
        
        // Start listening
        if (m_server->listen("registry")) {
            qDebug() << "Local IPC server listening on local:registry";
        } else {
            qDebug() << "Failed to start Local IPC server:" << m_server->errorString();
        }
        
        // Connect to newConnection signal
        connect(m_server, &QLocalServer::newConnection, this, &LocalIPCServer::handleNewConnection);
    }
    
    ~LocalIPCServer() {
        m_server->close();
    }
    
private slots:
    void handleNewConnection() {
        QLocalSocket *clientSocket = m_server->nextPendingConnection();
        qDebug() << "New connection received at Local IPC server!";
        
        // Set up connection monitoring
        connect(clientSocket, &QLocalSocket::disconnected, [clientSocket]() {
            qDebug() << "Client disconnected from Local IPC server";
            clientSocket->deleteLater();
        });
        
        connect(clientSocket, &QLocalSocket::errorOccurred, [clientSocket](QLocalSocket::LocalSocketError error) {
            qDebug() << "Socket error:" << error << "-" << clientSocket->errorString();
        });
        
        connect(clientSocket, &QLocalSocket::readyRead, [this, clientSocket]() {
            // Read data from client
            QByteArray data = clientSocket->readAll();
            qDebug() << "Received data from client, size:" << data.size() << "bytes";
            
            // Forward the data to our backend
            emit dataReceived(clientSocket, data);
        });
        
        // Notify about new connection
        emit newConnectionReceived(clientSocket);
    }
    
public slots:
    void sendDataToClient(QLocalSocket *socket, const QByteArray &data) {
        if (socket && socket->state() == QLocalSocket::ConnectedState) {
            qDebug() << "Sending data to client, size:" << data.size() << "bytes";
            socket->write(data);
        }
    }
    
signals:
    void newConnectionReceived(QLocalSocket *socket);
    void dataReceived(QLocalSocket *socket, const QByteArray &data);
    
private:
    QLocalServer *m_server;
};

// Custom QRemoteObjectHost that proxies through our Local IPC server
class ProxyRemoteObjectHost : public QRemoteObjectHost
{
    Q_OBJECT
    
public:
    ProxyRemoteObjectHost(const QUrl &address, LocalIPCServer *ipcServer, QObject *parent = nullptr)
        : QRemoteObjectHost(address, parent), m_ipcServer(ipcServer)
    {
        // Connect our IPC server's signals to our proxy handler methods
        connect(m_ipcServer, &LocalIPCServer::newConnectionReceived, 
                this, &ProxyRemoteObjectHost::handleNewConnection);
                
        connect(m_ipcServer, &LocalIPCServer::dataReceived, 
                this, &ProxyRemoteObjectHost::handleClientData);
    }
    
    // Override clientCount to provide connection info
    int clientCount() const {
        // return m_connectedClients.size() + QRemoteObjectHost::clientCount();
        return 0;
    }
    
private slots:
    void handleNewConnection(QLocalSocket *socket) {
        qDebug() << "ProxyRemoteObjectHost: New client connected";
        
        // Store the client socket
        m_connectedClients.insert(socket);
        
        // Send an acknowledgment message to the client
        QByteArray welcomeMsg;
        QDataStream stream(&welcomeMsg, QIODevice::WriteOnly);
        stream << QString("WELCOME_TO_PROXY_SERVER");
        
        m_ipcServer->sendDataToClient(socket, welcomeMsg);
    }
    
    void handleClientData(QLocalSocket *socket, const QByteArray &data) {
        qDebug() << "ProxyRemoteObjectHost: Processing client data";
        
        // In a real implementation, you would need to:
        // 1. Deserialize the client request
        // 2. Forward it to the QRemoteObjectHost backend
        // 3. Get the response
        // 4. Send the response back to the client
        
        // For now, we'll just send a simple response to the client
        QByteArray response;
        QDataStream stream(&response, QIODevice::WriteOnly);
        stream << QString("RECEIVED_DATA_ACKNOWLEDGMENT");
        
        m_ipcServer->sendDataToClient(socket, response);
        
        // In a complete implementation, you would relay messages between
        // this RemoteObjectHost and the connected clients through the IPC sockets
    }
    
private:
    LocalIPCServer *m_ipcServer;
    QSet<QLocalSocket*> m_connectedClients;
};

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    // Create the Local IPC server
    LocalIPCServer *ipcServer = new LocalIPCServer(&app);
    
    // Create a host node to share objects, using a different address since local:registry is used by our IPC server
    ProxyRemoteObjectHost srcNode(QUrl(QStringLiteral("local:registry_backend")), ipcServer, &app);
    QRemoteObjectHost srcNode2(QUrl(QStringLiteral("local:single_connection_registry")));

    // Track when an object is registered
    QObject::connect(&srcNode, &QRemoteObjectHost::remoteObjectAdded,
                [](const QRemoteObjectSourceLocation &entry) {
                    qDebug() << "Object registered: " << entry.first;
                });

    // Create and share the counter
    Counter *counter = new Counter(&srcNode);
    srcNode.enableRemoting(counter, "Counter");

    // Create and share ModuleCommunication
    ModuleCommunication *moduleCommunication = new ModuleCommunication(&srcNode);
    srcNode.enableRemoting(moduleCommunication, "ModuleCommunication");

    // Set up a timer to periodically send messages to clients
    QTimer *messageTimer = new QTimer(&app);
    QObject::connect(messageTimer, &QTimer::timeout, [moduleCommunication]() {
        static int messageCount = 1;
        QString message = QString("Server message #%1 - %2")
                          .arg(messageCount++)
                          .arg(QDateTime::currentDateTime().toString("hh:mm:ss"));
        
        moduleCommunication->sendMessageToClients(message);
    });
    messageTimer->start(5000); // Send a message every 5 seconds

    // Set up connection status reporting
    QTimer *statusTimer = new QTimer(&app);
    QObject::connect(statusTimer, &QTimer::timeout, [&srcNode]() {
        qDebug() << "Current client count:" << srcNode.clientCount();
    });
    statusTimer->start(10000); // Report every 10 seconds

    qDebug() << "Counter Server started.";
    qDebug() << "Local IPC Server listening at: local:registry";
    qDebug() << "QRemoteObjectHost backend running at: local:registry_backend";
    qDebug() << "Press Ctrl+C to quit";

    return app.exec();
}

#include "server.moc" 