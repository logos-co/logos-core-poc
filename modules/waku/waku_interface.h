#pragma once

#include <QtCore/QObject>
#include "../../core/interface.h"

// Callback type definitions
using WakuInitCallback = std::function<void(bool success, const QString &message)>;
using WakuVersionCallback = std::function<void(const QString &version)>;
using WakuStartCallback = std::function<void(bool success, const QString &message)>;
using WakuStopCallback = std::function<void(bool success, const QString &message)>;
using WakuContentTopicCallback = std::function<void(bool success, const QString &contentTopic)>;
using WakuPubSubTopicCallback = std::function<void(bool success, const QString &pubSubTopic)>;
using WakuPublishCallback = std::function<void(bool success, const QString &message)>;
using WakuProtectedShardCallback = std::function<void(bool success, const QString &message)>;
using WakuSubscribeCallback = std::function<void(bool success, const QString &message)>;
using WakuFilterSubscribeCallback = std::function<void(bool success, const QString &message)>;
using WakuConnectCallback = std::function<void(bool success, const QString &message)>;
using WakuStoreQueryCallback = std::function<void(bool success, const QString &message)>;
using WakuDestroyCallback = std::function<void(bool success, const QString &message)>;
using WakuEventCallback = std::function<void(const QString &event)>;

class WakuInterface : public PluginInterface {
public:
    virtual ~WakuInterface() {}

    // Plugin methods
    virtual void initWaku(const QString &cfg = "{}") = 0;
    virtual void getVersion(WakuVersionCallback callback = nullptr) = 0;
    virtual void startWaku(WakuStartCallback callback = nullptr) = 0;
    virtual void stopWaku(WakuStopCallback callback = nullptr) = 0;
    virtual void createContentTopic(const QString &appName, unsigned int appVersion, 
                                   const QString &contentTopicName, const QString &encoding,
                                   WakuContentTopicCallback callback = nullptr) = 0;
    virtual void createPubSubTopic(const QString &topicName, 
                                  WakuPubSubTopicCallback callback = nullptr) = 0;
    virtual void getDefaultPubSubTopic(WakuPubSubTopicCallback callback = nullptr) = 0;
    virtual void relayPublish(const QString &pubSubTopic, const QString &jsonWakuMessage,
                             unsigned int timeoutMs, WakuPublishCallback callback = nullptr) = 0;
    virtual void relayAddProtectedShard(int clusterId, int shardId, const QString &publicKey,
                                      WakuProtectedShardCallback callback = nullptr) = 0;
    virtual void relaySubscribe(const QString &pubSubTopic, 
                               WakuSubscribeCallback callback = nullptr) = 0;
    virtual void relayUnsubscribe(const QString &pubSubTopic, 
                                 WakuSubscribeCallback callback = nullptr) = 0;
    virtual void filterSubscribe(const QString &pubSubTopic, const QString &contentTopics, 
                               WakuFilterSubscribeCallback callback = nullptr) = 0;
    virtual void connectPeer(const QString &peerMultiAddr, unsigned int timeoutMs, 
                            WakuConnectCallback callback = nullptr) = 0;
    virtual void storeQuery(const QString &jsonQuery, const QString &peerAddr, 
                           unsigned int timeoutMs, WakuStoreQueryCallback callback = nullptr) = 0;
    virtual void destroyWaku(WakuDestroyCallback callback = nullptr) = 0;
    virtual void setEventCallback(WakuEventCallback callback) = 0;
};

#define WakuInterface_iid "com.logos.WakuInterface"
Q_DECLARE_INTERFACE(WakuInterface, WakuInterface_iid) 