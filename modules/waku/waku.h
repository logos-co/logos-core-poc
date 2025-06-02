#pragma once

#include <QtCore/QObject>
#include <functional>
#include "waku_interface.h"

class Waku : public QObject, public WakuInterface {
    Q_OBJECT
    Q_PLUGIN_METADATA(IID WakuInterface_iid FILE "metadata.json")
    Q_INTERFACES(WakuInterface PluginInterface)

public:
    Waku();
    ~Waku();

    // PluginInterface
    QString name() const override { return "waku"; }
    QString version() const override { return "0.1.0"; }

    // WakuInterface implementation
    Q_INVOKABLE void initWaku(const QString &cfg = "{}") override;
    Q_INVOKABLE void getVersion(WakuVersionCallback callback = nullptr) override;
    Q_INVOKABLE void startWaku(WakuStartCallback callback = nullptr) override;
    Q_INVOKABLE void stopWaku(WakuStopCallback callback = nullptr) override;
    Q_INVOKABLE void createContentTopic(const QString &appName, unsigned int appVersion, 
                                       const QString &contentTopicName, const QString &encoding,
                                       WakuContentTopicCallback callback = nullptr) override;
    Q_INVOKABLE void createPubSubTopic(const QString &topicName, 
                                      WakuPubSubTopicCallback callback = nullptr) override;
    Q_INVOKABLE void getDefaultPubSubTopic(WakuPubSubTopicCallback callback = nullptr) override;
    Q_INVOKABLE void relayPublish(const QString &pubSubTopic, const QString &jsonWakuMessage,
                                 unsigned int timeoutMs, WakuPublishCallback callback = nullptr) override;
    Q_INVOKABLE void relayAddProtectedShard(int clusterId, int shardId, const QString &publicKey,
                                          WakuProtectedShardCallback callback = nullptr) override;
    Q_INVOKABLE void relaySubscribe(const QString &pubSubTopic, 
                                  WakuSubscribeCallback callback = nullptr) override;
    Q_INVOKABLE void relayUnsubscribe(const QString &pubSubTopic, 
                                     WakuSubscribeCallback callback = nullptr) override;
    Q_INVOKABLE void filterSubscribe(const QString &pubSubTopic, const QString &contentTopics, 
                                   WakuFilterSubscribeCallback callback = nullptr) override;
    Q_INVOKABLE void connectPeer(const QString &peerMultiAddr, unsigned int timeoutMs, 
                                WakuConnectCallback callback = nullptr) override;
    Q_INVOKABLE void storeQuery(const QString &jsonQuery, const QString &peerAddr, 
                              unsigned int timeoutMs, WakuStoreQueryCallback callback = nullptr) override;
    Q_INVOKABLE void destroyWaku(WakuDestroyCallback callback = nullptr) override;
    Q_INVOKABLE void setEventCallback(WakuEventCallback callback) override;

private:
    void* wakuCtx;
    WakuInitCallback initCallback;
    WakuVersionCallback versionCallback;
    WakuStartCallback startCallback;
    WakuStopCallback stopCallback;
    WakuContentTopicCallback contentTopicCallback;
    WakuPubSubTopicCallback pubSubTopicCallback;
    WakuPublishCallback publishCallback;
    WakuProtectedShardCallback protectedShardCallback;
    WakuSubscribeCallback subscribeCallback;
    WakuFilterSubscribeCallback filterSubscribeCallback;
    WakuConnectCallback connectCallback;
    WakuStoreQueryCallback storeQueryCallback;
    WakuDestroyCallback destroyCallback;
    WakuEventCallback eventCallback;
}; 