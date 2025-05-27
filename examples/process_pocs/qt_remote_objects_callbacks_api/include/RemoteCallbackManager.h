// RemoteCallbackManager.h
#pragma once

#include <QObject>
#include <QRemoteObjectNode>
#include <QUuid>
#include <QMap>
#include <QDebug>
#include <functional>

template <typename ReturnType>
class RemoteCallbackManager : public QObject {
public:
    using CallbackType = std::function<void(ReturnType)>;
    
    RemoteCallbackManager(QObject* parent = nullptr) : QObject(parent) {}
    
    // Register a callback and get a unique ID for the request
    QString registerCallback(CallbackType callback) {
        QString requestId = QUuid::createUuid().toString();
        m_callbacks[requestId] = callback;
        return requestId;
    }
    
    // Execute the callback with the given result
    void executeCallback(const QString& requestId, ReturnType result) {
        if (m_callbacks.contains(requestId)) {
            auto callback = m_callbacks.take(requestId);
            callback(result);
        } else {
            qDebug() << "No callback found for request ID:" << requestId;
        }
    }
    
private:
    QMap<QString, CallbackType> m_callbacks;
};
