#include <QCoreApplication>
#include <QRemoteObjectRegistryHost>
#include <QDebug>
#include <QTimer>
#include "counter.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    // Create the registry host
    QRemoteObjectRegistryHost host(QUrl("local:registry"));

    // Create our counter object
    Counter counter;
    
    // Show registered operations
    qDebug() << "\n=== SERVER REGISTRY DEMO ===";
    qDebug() << "The Counter class now automatically registers these operations:";
    qDebug() << "- multiply: (int factor) -> int result";
    qDebug() << "- add: (int a, int b) -> (int sum, QString message, QDateTime timestamp)";  
    qDebug() << "- multiply_advanced: (int factor, double multiplier) -> (double result, int factor, double multiplier, int count, QString message)";
    qDebug() << "- power: (int exponent) -> (double result, int exponent, int count, QString message)";
    qDebug() << "- string_operation: (QString...) -> (QString result, int argCount, int count)";
    qDebug() << "- statistics: (QVariantList numbers) -> (double sum, double avg, double min, double max, int count, QString message)";
    qDebug() << "\nAll operations are automatically exposed via Qt Remote Objects with callback support!";
    qDebug() << "No manual signal/slot/requestId handling needed!\n";

    // Register the counter with Remote Objects
    host.enableRemoting(&counter, "Counter");
    
    qDebug() << "Server started. Counter object registered with Remote Objects.";
    qDebug() << "Client can now call any registered operation and get async callbacks.";
    qDebug() << "\nExample: Client calls multiply(5) -> automatically gets callback with result";
    qDebug() << "Example: Client calls add(10, 20) -> automatically gets callback with sum + metadata";
    qDebug() << "\nServer is running... Press Ctrl+C to stop.";

    return app.exec();
} 