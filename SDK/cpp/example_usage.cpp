/**
 * @file example_usage.cpp
 * @brief Example showing how to use the LogosAPI SDK
 * 
 * This example demonstrates how to use the LogosAPI to connect
 * to the Logos Core registry and request remote objects.
 */

#include "logos_api_client.h"
#include <QMetaObject>
#include <QRemoteObjectReplica>
#include <QDebug>

void exampleUsage()
{
    // Create a client instance (uses default registry URL)
    LogosAPI client;
    
    // Check if connected
    if (!client.isConnected()) {
        qWarning() << "Failed to connect to Logos Core registry";
        return;
    }
    
    // Request the Core Manager object
    QRemoteObjectReplica* coreManager = client.requestObject("Core Manager");
    if (!coreManager) {
        qWarning() << "Failed to acquire Core Manager replica";
        return;
    }
    
    // Use the replica to call methods
    QString pluginName;
    bool success = QMetaObject::invokeMethod(
        coreManager,
        "processPlugin",
        Qt::DirectConnection,
        Q_RETURN_ARG(QString, pluginName),
        Q_ARG(QString, "/path/to/plugin.dylib")
    );
    
    if (success && !pluginName.isEmpty()) {
        qDebug() << "Successfully processed plugin:" << pluginName;
    } else {
        qWarning() << "Failed to process plugin";
    }
    
    // Clean up the replica when done
    delete coreManager;
}

// Alternative usage with custom registry URL and timeout
void exampleCustomUsage()
{
    // Create client with custom registry URL
    LogosAPI client("custom_registry");
    
    if (!client.isConnected()) {
        // Try to reconnect
        if (!client.reconnect()) {
            qWarning() << "Failed to connect to custom registry";
            return;
        }
    }
    
    // Request object with custom timeout (10 seconds)
    QRemoteObjectReplica* someObject = client.requestObject("Some Object", 10000);
    if (someObject) {
        // Use the object...
        
        // Clean up
        delete someObject;
    }
} 