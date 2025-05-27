#pragma once

#include <QObject>
#include <QUuid>
#include <QMap>
#include <QDebug>
#include <QVariantList>
#include <QVariant>
#include <functional>

// Abstract callback manager that can handle any method signature
class AbstractRemoteCallbackManager : public QObject {
    Q_OBJECT

public:
    // Generic callback type that accepts a QVariantList of results
    using GenericCallbackType = std::function<void(const QVariantList&)>;
    
    AbstractRemoteCallbackManager(QObject* parent = nullptr) : QObject(parent) {}
    
    // Register a callback and get a unique ID for the request
    QString registerCallback(GenericCallbackType callback) {
        QString requestId = QUuid::createUuid().toString();
        m_callbacks[requestId] = callback;
        qDebug() << "AbstractRemoteCallbackManager: Registered callback with ID:" << requestId;
        return requestId;
    }
    
    // Execute the callback with the given results
    void executeCallback(const QString& requestId, const QVariantList& results) {
        if (m_callbacks.contains(requestId)) {
            auto callback = m_callbacks.take(requestId);
            qDebug() << "AbstractRemoteCallbackManager: Executing callback for ID:" << requestId << "with results:" << results;
            callback(results);
        } else {
            qDebug() << "AbstractRemoteCallbackManager: No callback found for request ID:" << requestId;
        }
    }
    
    // Convenience method for single result
    void executeCallback(const QString& requestId, const QVariant& result) {
        executeCallback(requestId, QVariantList{result});
    }
    
    // Template method to register callbacks with specific signatures
    template<typename... Args>
    QString registerTypedCallback(std::function<void(Args...)> callback) {
        auto genericCallback = [callback](const QVariantList& results) {
            if (results.size() != sizeof...(Args)) {
                qDebug() << "AbstractRemoteCallbackManager: Argument count mismatch. Expected:" 
                         << sizeof...(Args) << "Got:" << results.size();
                return;
            }
            callWithVariants<Args...>(callback, results, std::index_sequence_for<Args...>{});
        };
        return registerCallback(genericCallback);
    }

private:
    QMap<QString, GenericCallbackType> m_callbacks;
    
    // Helper to convert QVariantList back to typed arguments
    template<typename... Args, std::size_t... I>
    void callWithVariants(std::function<void(Args...)> callback, 
                         const QVariantList& variants,
                         std::index_sequence<I...>) {
        callback(variants[I].value<Args>()...);
    }
}; 