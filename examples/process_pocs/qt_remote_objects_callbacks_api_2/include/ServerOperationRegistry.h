#pragma once

#include <QObject>
#include <QMap>
#include <QString>
#include <QVariantList>
#include <QVariant>
#include <QTimer>
#include <QRandomGenerator>
#include <QDebug>
#include <functional>

// Server-side operation registry that automatically handles callbacks
class ServerOperationRegistry : public QObject {
    Q_OBJECT

public:
    // Generic operation type that takes arguments and returns results
    using OperationType = std::function<QVariantList(const QVariantList&)>;
    
    ServerOperationRegistry(QObject* parent = nullptr) : QObject(parent) {}
    
    // Register an operation with a name and implementation
    void registerOperation(const QString& operationName, OperationType operation) {
        m_operations[operationName] = operation;
        qDebug() << "ServerOperationRegistry: Registered operation:" << operationName;
    }
    
    // Template method to register operations with specific signatures
    template<typename ReturnType, typename... Args>
    void registerOperation(const QString& operationName, 
                          std::function<ReturnType(Args...)> operation) {
        auto genericOperation = [operation](const QVariantList& args) -> QVariantList {
            if (args.size() != sizeof...(Args)) {
                qDebug() << "ServerOperationRegistry: Argument count mismatch for" << operationName
                         << "Expected:" << sizeof...(Args) << "Got:" << args.size();
                return QVariantList{QVariant::fromValue(QString("Argument count mismatch"))};
            }
            
            // Convert QVariantList to typed arguments and call the operation
            ReturnType result = callWithVariants<ReturnType, Args...>(operation, args, 
                                                                      std::index_sequence_for<Args...>{});
            
            // Convert result back to QVariantList
            return QVariantList{QVariant::fromValue(result)};
        };
        
        registerOperation(operationName, genericOperation);
    }
    
    // Template method for operations that return multiple values
    template<typename... ReturnTypes, typename... Args>
    void registerMultiReturnOperation(const QString& operationName,
                                     std::function<std::tuple<ReturnTypes...>(Args...)> operation) {
        auto genericOperation = [operation](const QVariantList& args) -> QVariantList {
            if (args.size() != sizeof...(Args)) {
                qDebug() << "ServerOperationRegistry: Argument count mismatch for" << operationName;
                return QVariantList{QVariant::fromValue(QString("Argument count mismatch"))};
            }
            
            // Convert QVariantList to typed arguments and call the operation
            auto result = callWithVariants<std::tuple<ReturnTypes...>, Args...>(operation, args,
                                                                               std::index_sequence_for<Args...>{});
            
            // Convert tuple result back to QVariantList
            return tupleToVariantList(result, std::index_sequence_for<ReturnTypes...>{});
        };
        
        registerOperation(operationName, genericOperation);
    }
    
    // Execute an operation asynchronously and emit result
    void executeOperation(const QString& requestId, const QString& operationName, 
                         const QVariantList& arguments) {
        qDebug() << "ServerOperationRegistry: Executing operation" << operationName 
                 << "with requestId" << requestId << "and arguments" << arguments;
        
        if (!m_operations.contains(operationName)) {
            qDebug() << "ServerOperationRegistry: Unknown operation:" << operationName;
            emit operationResult(requestId, QVariantList{QString("Unknown operation: %1").arg(operationName)});
            return;
        }
        
        // Execute the operation asynchronously to simulate real async work
        int delay = QRandomGenerator::global()->bounded(1000, 3000);
        QTimer::singleShot(delay, this, [this, requestId, operationName, arguments]() {
            try {
                auto operation = m_operations[operationName];
                QVariantList results = operation(arguments);
                
                qDebug() << "ServerOperationRegistry: Operation" << operationName 
                         << "completed with results:" << results;
                emit operationResult(requestId, results);
            } catch (const std::exception& e) {
                qDebug() << "ServerOperationRegistry: Operation" << operationName 
                         << "failed with error:" << e.what();
                emit operationResult(requestId, QVariantList{QString("Error: %1").arg(e.what())});
            }
        });
    }
    
    // Get list of registered operations
    QStringList getRegisteredOperations() const {
        return m_operations.keys();
    }

signals:
    void operationResult(const QString& requestId, const QVariantList& results);

private:
    QMap<QString, OperationType> m_operations;
    
    // Helper to convert QVariantList to typed arguments and call function
    template<typename ReturnType, typename... Args, std::size_t... I>
    ReturnType callWithVariants(std::function<ReturnType(Args...)> func,
                               const QVariantList& variants,
                               std::index_sequence<I...>) {
        return func(variants[I].value<Args>()...);
    }
    
    // Helper to convert tuple to QVariantList
    template<typename... Types, std::size_t... I>
    QVariantList tupleToVariantList(const std::tuple<Types...>& tuple,
                                   std::index_sequence<I...>) {
        return QVariantList{QVariant::fromValue(std::get<I>(tuple))...};
    }
}; 