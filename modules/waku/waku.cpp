#include "waku.h"
#include <QDebug>
#include <QThread>
#include "lib/libwaku.h"

namespace {
    // Structure to hold version data
    struct VersionData {
        Waku* waku;
        WakuVersionCallback callback;
    };

    // Static callback for waku_version
    void version_callback(int callerRet, const char* msg, size_t len, void* userData) {
        auto* data = static_cast<VersionData*>(userData);
        QString version;
        
        if (callerRet == RET_OK && msg != nullptr) {
            version = QString::fromUtf8(msg, len);
        } else {
            version = "Error getting version";
        }
        
        // Call the user callback if provided
        if (data->callback) {
            data->callback(version);
        }
        
        delete data;
    }

    // Structure to hold init data
    struct InitData {
        Waku* waku;
        WakuInitCallback callback;
    };

    // Static callback for waku_new
    void init_callback(int callerRet, const char* msg, size_t len, void* userData) {
        bool success = (callerRet == RET_OK);
        QString message;
        
        if (success) {
            message = "Waku initialized successfully";
            qDebug() << message;
        } else {
            message = msg ? QString::fromUtf8(msg, len) : "Unknown error";
            qDebug() << "Waku initialization failed:" << message;
        }
        
        // Call user callback if provided
        if (userData) {
            auto* data = static_cast<InitData*>(userData);
            if (data->callback) {
                data->callback(success, message);
            }
            delete data;
        }
    }

    // Structure to hold start data
    struct StartData {
        Waku* waku;
        WakuStartCallback callback;
    };

    // Static callback for waku_start
    void start_callback(int callerRet, const char* msg, size_t len, void* userData) {
        bool success = (callerRet == RET_OK);
        QString message;
        
        if (success) {
            message = "Waku started successfully";
            qDebug() << message;
        } else {
            message = msg ? QString::fromUtf8(msg, len) : "Unknown error";
            qDebug() << "Waku start failed:" << message;
        }
        
        // Call user callback if provided
        if (userData) {
            auto* data = static_cast<StartData*>(userData);
            if (data->callback) {
                data->callback(success, message);
            }
            delete data;
        }
    }

    // Structure to hold stop data
    struct StopData {
        Waku* waku;
        WakuStopCallback callback;
    };

    // Static callback for waku_stop
    void stop_callback(int callerRet, const char* msg, size_t len, void* userData) {
        bool success = (callerRet == RET_OK);
        QString message;
        
        if (success) {
            message = "Waku stopped successfully";
            qDebug() << message;
        } else {
            message = msg ? QString::fromUtf8(msg, len) : "Unknown error";
            qDebug() << "Waku stop failed:" << message;
        }
        
        // Call user callback if provided
        if (userData) {
            auto* data = static_cast<StopData*>(userData);
            if (data->callback) {
                data->callback(success, message);
            }
            delete data;
        }
    }

    // Structure to hold content topic data
    struct ContentTopicData {
        Waku* waku;
        WakuContentTopicCallback callback;
    };

    // Static callback for waku_content_topic
    void content_topic_callback(int callerRet, const char* msg, size_t len, void* userData) {
        bool success = (callerRet == RET_OK);
        QString contentTopic;
        
        if (success && msg != nullptr) {
            contentTopic = QString::fromUtf8(msg, len);
            qDebug() << "Content topic created:" << contentTopic;
        } else {
            contentTopic = msg ? QString::fromUtf8(msg, len) : "Unknown error";
            qDebug() << "Content topic creation failed:" << contentTopic;
        }
        
        // Call user callback if provided
        if (userData) {
            auto* data = static_cast<ContentTopicData*>(userData);
            if (data->callback) {
                data->callback(success, contentTopic);
            }
            delete data;
        }
    }

    // Structure to hold pubsub topic data
    struct PubSubTopicData {
        Waku* waku;
        WakuPubSubTopicCallback callback;
    };

    // Static callback for waku_pubsub_topic
    void pubsub_topic_callback(int callerRet, const char* msg, size_t len, void* userData) {
        bool success = (callerRet == RET_OK);
        QString pubSubTopic;
        
        if (success && msg != nullptr) {
            pubSubTopic = QString::fromUtf8(msg, len);
            qDebug() << "PubSub topic created:" << pubSubTopic;
        } else {
            pubSubTopic = msg ? QString::fromUtf8(msg, len) : "Unknown error";
            qDebug() << "PubSub topic creation failed:" << pubSubTopic;
        }
        
        // Call user callback if provided
        if (userData) {
            auto* data = static_cast<PubSubTopicData*>(userData);
            if (data->callback) {
                data->callback(success, pubSubTopic);
            }
            delete data;
        }
    }

    // Static callback for waku_default_pubsub_topic
    void default_pubsub_topic_callback(int callerRet, const char* msg, size_t len, void* userData) {
        bool success = (callerRet == RET_OK);
        QString pubSubTopic;
        
        if (success && msg != nullptr) {
            pubSubTopic = QString::fromUtf8(msg, len);
            qDebug() << "Default PubSub topic:" << pubSubTopic;
        } else {
            pubSubTopic = msg ? QString::fromUtf8(msg, len) : "Unknown error";
            qDebug() << "Failed to get default PubSub topic:" << pubSubTopic;
        }
        
        // Call user callback if provided
        if (userData) {
            auto* data = static_cast<PubSubTopicData*>(userData);
            if (data->callback) {
                data->callback(success, pubSubTopic);
            }
            delete data;
        }
    }

    // Structure to hold publish data
    struct PublishData {
        Waku* waku;
        WakuPublishCallback callback;
    };

    // Static callback for waku_relay_publish
    void relay_publish_callback(int callerRet, const char* msg, size_t len, void* userData) {
        bool success = (callerRet == RET_OK);
        QString message;
        
        if (success) {
            message = "Message published successfully";
            qDebug() << message;
        } else {
            message = msg ? QString::fromUtf8(msg, len) : "Unknown error";
            qDebug() << "Message publication failed:" << message;
        }
        
        // Call user callback if provided
        if (userData) {
            auto* data = static_cast<PublishData*>(userData);
            if (data->callback) {
                data->callback(success, message);
            }
            delete data;
        }
    }

    // Structure to hold protected shard data
    struct ProtectedShardData {
        Waku* waku;
        WakuProtectedShardCallback callback;
    };

    // Static callback for waku_relay_add_protected_shard
    void protected_shard_callback(int callerRet, const char* msg, size_t len, void* userData) {
        bool success = (callerRet == RET_OK);
        QString message;
        
        if (success) {
            message = "Protected shard added successfully";
            qDebug() << message;
        } else {
            message = msg ? QString::fromUtf8(msg, len) : "Unknown error";
            qDebug() << "Failed to add protected shard:" << message;
        }
        
        // Call user callback if provided
        if (userData) {
            auto* data = static_cast<ProtectedShardData*>(userData);
            if (data->callback) {
                data->callback(success, message);
            }
            delete data;
        }
    }

    // Structure to hold subscribe data
    struct SubscribeData {
        Waku* waku;
        WakuSubscribeCallback callback;
    };

    // Static callback for waku_relay_subscribe
    void relay_subscribe_callback(int callerRet, const char* msg, size_t len, void* userData) {
        bool success = (callerRet == RET_OK);
        QString message;
        
        if (success) {
            message = "Successfully subscribed to topic";
            qDebug() << message;
        } else {
            message = msg ? QString::fromUtf8(msg, len) : "Unknown error";
            qDebug() << "Failed to subscribe to topic:" << message;
        }
        
        // Call user callback if provided
        if (userData) {
            auto* data = static_cast<SubscribeData*>(userData);
            if (data->callback) {
                data->callback(success, message);
            }
            delete data;
        }
    }

    // Static callback for waku_relay_unsubscribe
    void relay_unsubscribe_callback(int callerRet, const char* msg, size_t len, void* userData) {
        bool success = (callerRet == RET_OK);
        QString message;
        
        if (success) {
            message = "Successfully unsubscribed from topic";
            qDebug() << message;
        } else {
            message = msg ? QString::fromUtf8(msg, len) : "Unknown error";
            qDebug() << "Failed to unsubscribe from topic:" << message;
        }
        
        // Call user callback if provided
        if (userData) {
            auto* data = static_cast<SubscribeData*>(userData);
            if (data->callback) {
                data->callback(success, message);
            }
            delete data;
        }
    }

    // Structure to hold filter subscribe data
    struct FilterSubscribeData {
        Waku* waku;
        WakuFilterSubscribeCallback callback;
    };

    // Static callback for waku_filter_subscribe
    void filter_subscribe_callback(int callerRet, const char* msg, size_t len, void* userData) {
        bool success = (callerRet == RET_OK);
        QString message;
        
        if (success) {
            message = "Successfully subscribed to filter";
            qDebug() << message;
        } else {
            message = msg ? QString::fromUtf8(msg, len) : "Unknown error";
            qDebug() << "Failed to subscribe to filter:" << message;
        }
        
        // Call user callback if provided
        if (userData) {
            auto* data = static_cast<FilterSubscribeData*>(userData);
            if (data->callback) {
                data->callback(success, message);
            }
            delete data;
        }
    }

    // Structure to hold connect data
    struct ConnectData {
        Waku* waku;
        WakuConnectCallback callback;
    };

    // Static callback for waku_connect
    void connect_callback(int callerRet, const char* msg, size_t len, void* userData) {
        qDebug() << "Connect callback called";
        bool success = (callerRet == RET_OK);
        QString message;
        
        if (success) {
            message = "Successfully connected to peer";
            qDebug() << message;
        } else {
            message = msg ? QString::fromUtf8(msg, len) : "Unknown error";
            qDebug() << "Failed to connect to peer:" << message;
        }
        
        // Call user callback if provided
        if (userData) {
            auto* data = static_cast<ConnectData*>(userData);
            if (data->callback) {
                data->callback(success, message);
            }
            delete data;
        }
    }

    // Structure to hold event callback data
    struct EventData {
        Waku* waku;
        WakuEventCallback callback;
    };

    // Static callback for waku_set_event_callback
    void event_callback(int callerRet, const char* msg, size_t len, void* userData) {
        auto* data = static_cast<EventData*>(userData);
        
        // Only process if callback exists
        if (data && data->callback && msg != nullptr) {
            QString event = QString::fromUtf8(msg, len);
            // qDebug() << "Waku event received:" << event;
            
            // Call the registered callback with the event data
            data->callback(event);
        }
        
        // Note: We don't delete the data here since this is a persistent callback
        // that will be called multiple times throughout the lifetime of the waku node
    }

    // Structure to hold store query data
    struct StoreQueryData {
        Waku* waku;
        WakuStoreQueryCallback callback;
    };

    // Static callback for waku_store_query
    void store_query_callback(int callerRet, const char* msg, size_t len, void* userData) {
        bool success = (callerRet == RET_OK);
        QString message;
        
        if (success && msg != nullptr) {
            message = QString::fromUtf8(msg, len);
            // qDebug() << "Store query successful, response:" << message;
        } else {
            message = msg ? QString::fromUtf8(msg, len) : "Unknown error";
            qDebug() << "Store query failed:" << message;
        }
        
        // Call user callback if provided
        if (userData) {
            auto* data = static_cast<StoreQueryData*>(userData);
            if (data->callback) {
                data->callback(success, message);
            }
            delete data;
        }
    }

    // Structure to hold destroy data
    struct DestroyData {
        Waku* waku;
        WakuDestroyCallback callback;
    };

    // Static callback for waku_destroy
    void destroy_callback(int callerRet, const char* msg, size_t len, void* userData) {
        bool success = (callerRet == RET_OK);
        QString message;
        
        if (success) {
            message = "Waku destroyed successfully";
            qDebug() << message;
        } else {
            message = msg ? QString::fromUtf8(msg, len) : "Unknown error";
            qDebug() << "Waku destruction failed:" << message;
        }
        
        // Call user callback if provided
        if (userData) {
            auto* data = static_cast<DestroyData*>(userData);
            if (data->callback) {
                data->callback(success, message);
            }
            delete data;
        }
    }
}

Waku::Waku() : wakuCtx(nullptr) {
    qDebug() << "Waku Plugin initialized!";
}

Waku::~Waku() {
    qDebug() << "Waku Plugin destroyed!";
    if (wakuCtx) {
        // Use our new destroyWaku method with a null callback
        destroyWaku(nullptr);
    }
}

void Waku::initWaku(const QString &cfg, WakuInitCallback callback) {
    qDebug() << "Initializing Waku...";
    // Clean up existing instance if any
    if (wakuCtx) {
        waku_destroy(wakuCtx, nullptr, nullptr);
        wakuCtx = nullptr;
    }

    // Save the callback
    InitData* userData = new InitData{this, callback};

    // Initialize Waku - passing the callback data as userData and configuration
    QByteArray cfgUtf8 = cfg.toUtf8();
    wakuCtx = waku_new(cfgUtf8.constData(), init_callback, userData);
    if (!wakuCtx) {
        qDebug() << "Failed to initialize Waku";
        // Call callback for failure case
        if (callback) {
            callback(false, "Failed to initialize Waku");
        }
        delete userData;
    }
}

void Waku::getVersion(WakuVersionCallback callback) {
    qDebug() << "Getting Waku version...";
    if (!wakuCtx) {
        QString errorMsg = "Waku not initialized";
        qDebug() << errorMsg;
        if (callback) {
            callback(errorMsg);
        }
        return;
    }

    // Create version data for the callback, including callback function
    auto* data = new VersionData{this, callback};

    // Get version
    int ret = waku_version(wakuCtx, version_callback, data);
    if (ret != RET_OK) {
        QString errorMsg = "Failed to get version";
        if (callback) {
            callback(errorMsg);
        }
        delete data;
    }
}

void Waku::startWaku(WakuStartCallback callback) {
    qDebug() << "Starting Waku...";
    if (!wakuCtx) {
        QString errorMsg = "Waku not initialized";
        qDebug() << errorMsg;
        if (callback) {
            callback(false, errorMsg);
        }
        return;
    }

    // Create start data for the callback, including callback function
    auto* data = new StartData{this, callback};

    // Start waku
    int ret = waku_start(wakuCtx, start_callback, data);
    if (ret != RET_OK) {
        QString errorMsg = "Failed to start Waku";
        qDebug() << errorMsg;
        if (callback) {
            callback(false, errorMsg);
        }
        delete data;
    }
}

void Waku::stopWaku(WakuStopCallback callback) {
    qDebug() << "Stopping Waku...";
    if (!wakuCtx) {
        QString errorMsg = "Waku not initialized";
        qDebug() << errorMsg;
        if (callback) {
            callback(false, errorMsg);
        }
        return;
    }

    // Create stop data for the callback, including callback function
    auto* data = new StopData{this, callback};

    // Stop waku
    int ret = waku_stop(wakuCtx, stop_callback, data);
    if (ret != RET_OK) {
        QString errorMsg = "Failed to stop Waku";
        qDebug() << errorMsg;
        if (callback) {
            callback(false, errorMsg);
        }
        delete data;
    }
}

void Waku::createContentTopic(const QString &appName, unsigned int appVersion, 
                             const QString &contentTopicName, const QString &encoding,
                             WakuContentTopicCallback callback) {
    qDebug() << "Creating content topic...";
    if (!wakuCtx) {
        QString errorMsg = "Waku not initialized";
        qDebug() << errorMsg;
        if (callback) {
            callback(false, errorMsg);
        }
        return;
    }

    // Create content topic data for the callback
    auto* data = new ContentTopicData{this, callback};

    // Convert QString to UTF-8 C string
    QByteArray appNameUtf8 = appName.toUtf8();
    QByteArray contentTopicNameUtf8 = contentTopicName.toUtf8();
    QByteArray encodingUtf8 = encoding.toUtf8();

    // Call the waku_content_topic function
    int ret = waku_content_topic(
        wakuCtx,
        appNameUtf8.constData(),
        appVersion,
        contentTopicNameUtf8.constData(),
        encodingUtf8.constData(),
        content_topic_callback,
        data
    );

    if (ret != RET_OK) {
        QString errorMsg = "Failed to create content topic";
        qDebug() << errorMsg;
        if (callback) {
            callback(false, errorMsg);
        }
        delete data;
    }
}

void Waku::createPubSubTopic(const QString &topicName, WakuPubSubTopicCallback callback) {
    qDebug() << "Creating pubsub topic...";
    if (!wakuCtx) {
        QString errorMsg = "Waku not initialized";
        qDebug() << errorMsg;
        if (callback) {
            callback(false, errorMsg);
        }
        return;
    }

    // Create pubsub topic data for the callback
    auto* data = new PubSubTopicData{this, callback};

    // Convert QString to UTF-8 C string
    QByteArray topicNameUtf8 = topicName.toUtf8();

    // Call the waku_pubsub_topic function
    int ret = waku_pubsub_topic(
        wakuCtx,
        topicNameUtf8.constData(),
        pubsub_topic_callback,
        data
    );

    if (ret != RET_OK) {
        QString errorMsg = "Failed to create pubsub topic";
        qDebug() << errorMsg;
        if (callback) {
            callback(false, errorMsg);
        }
        delete data;
    }
}

void Waku::getDefaultPubSubTopic(WakuPubSubTopicCallback callback) {
    qDebug() << "Getting default pubsub topic...";
    if (!wakuCtx) {
        QString errorMsg = "Waku not initialized";
        qDebug() << errorMsg;
        if (callback) {
            callback(false, errorMsg);
        }
        return;
    }

    // Create pubsub topic data for the callback
    auto* data = new PubSubTopicData{this, callback};

    // Call the waku_default_pubsub_topic function
    int ret = waku_default_pubsub_topic(
        wakuCtx,
        default_pubsub_topic_callback,
        data
    );

    if (ret != RET_OK) {
        QString errorMsg = "Failed to get default pubsub topic";
        qDebug() << errorMsg;
        if (callback) {
            callback(false, errorMsg);
        }
        delete data;
    }
}

void Waku::relayPublish(const QString &pubSubTopic, const QString &jsonWakuMessage,
                       unsigned int timeoutMs, WakuPublishCallback callback) {
    qDebug() << "Publishing message...";
    if (!wakuCtx) {
        QString errorMsg = "Waku not initialized";
        qDebug() << errorMsg;
        if (callback) {
            callback(false, errorMsg);
        }
        return;
    }

    // Create publish data for the callback
    auto* data = new PublishData{this, callback};

    // Convert QString to UTF-8 C string
    QByteArray pubSubTopicUtf8 = pubSubTopic.toUtf8();
    QByteArray jsonWakuMessageUtf8 = jsonWakuMessage.toUtf8();

    // Call the waku_relay_publish function
    int ret = waku_relay_publish(
        wakuCtx,
        pubSubTopicUtf8.constData(),
        jsonWakuMessageUtf8.constData(),
        timeoutMs,
        relay_publish_callback,
        data
    );

    if (ret != RET_OK) {
        QString errorMsg = "Failed to publish message";
        qDebug() << errorMsg;
        if (callback) {
            callback(false, errorMsg);
        }
        delete data;
    }
}

void Waku::relayAddProtectedShard(int clusterId, int shardId, const QString &publicKey,
                                 WakuProtectedShardCallback callback) {
    qDebug() << "Adding protected shard...";
    if (!wakuCtx) {
        QString errorMsg = "Waku not initialized";
        qDebug() << errorMsg;
        if (callback) {
            callback(false, errorMsg);
        }
        return;
    }

    // Create protected shard data for the callback
    auto* data = new ProtectedShardData{this, callback};

    // Convert QString to UTF-8 C string 
    // Make a copy since waku_relay_add_protected_shard requires non-const char*
    QByteArray publicKeyUtf8 = publicKey.toUtf8();
    char* publicKeyPtr = publicKeyUtf8.data(); // Use data() instead of constData() to get a non-const pointer

    // Call the waku_relay_add_protected_shard function
    int ret = waku_relay_add_protected_shard(
        wakuCtx,
        clusterId,
        shardId,
        publicKeyPtr,
        protected_shard_callback,
        data
    );

    if (ret != RET_OK) {
        QString errorMsg = "Failed to add protected shard";
        qDebug() << errorMsg;
        if (callback) {
            callback(false, errorMsg);
        }
        delete data;
    }
}

void Waku::relaySubscribe(const QString &pubSubTopic, WakuSubscribeCallback callback) {
    qDebug() << "Subscribing to topic...";
    if (!wakuCtx) {
        QString errorMsg = "Waku not initialized";
        qDebug() << errorMsg;
        if (callback) {
            callback(false, errorMsg);
        }
        return;
    }

    // Create subscribe data for the callback
    auto* data = new SubscribeData{this, callback};

    // Convert QString to UTF-8 C string
    QByteArray pubSubTopicUtf8 = pubSubTopic.toUtf8();

    // Call the waku_relay_subscribe function
    int ret = waku_relay_subscribe(
        wakuCtx,
        pubSubTopicUtf8.constData(),
        relay_subscribe_callback,
        data
    );

    if (ret != RET_OK) {
        QString errorMsg = "Failed to subscribe to topic";
        qDebug() << errorMsg;
        if (callback) {
            callback(false, errorMsg);
        }
        delete data;
    }
}

void Waku::relayUnsubscribe(const QString &pubSubTopic, WakuSubscribeCallback callback) {
    qDebug() << "Unsubscribing from topic...";
    if (!wakuCtx) {
        QString errorMsg = "Waku not initialized";
        qDebug() << errorMsg;
        if (callback) {
            callback(false, errorMsg);
        }
        return;
    }

    // Create subscribe data for the callback
    auto* data = new SubscribeData{this, callback};

    // Convert QString to UTF-8 C string
    QByteArray pubSubTopicUtf8 = pubSubTopic.toUtf8();

    // Call the waku_relay_unsubscribe function
    int ret = waku_relay_unsubscribe(
        wakuCtx,
        pubSubTopicUtf8.constData(),
        relay_unsubscribe_callback,
        data
    );

    if (ret != RET_OK) {
        QString errorMsg = "Failed to unsubscribe from topic";
        qDebug() << errorMsg;
        if (callback) {
            callback(false, errorMsg);
        }
        delete data;
    }
}

void Waku::filterSubscribe(const QString &pubSubTopic, const QString &contentTopics, 
                         WakuFilterSubscribeCallback callback) {
    qDebug() << "Subscribing to filter...";
    if (!wakuCtx) {
        QString errorMsg = "Waku not initialized";
        qDebug() << errorMsg;
        if (callback) {
            callback(false, errorMsg);
        }
        return;
    }

    // Create filter subscribe data for the callback
    auto* data = new FilterSubscribeData{this, callback};

    // Convert QString to UTF-8 C string
    QByteArray pubSubTopicUtf8 = pubSubTopic.toUtf8();
    QByteArray contentTopicsUtf8 = contentTopics.toUtf8();

    // Call the waku_filter_subscribe function
    int ret = waku_filter_subscribe(
        wakuCtx,
        pubSubTopicUtf8.constData(),
        contentTopicsUtf8.constData(),
        filter_subscribe_callback,
        data
    );

    if (ret != RET_OK) {
        QString errorMsg = "Failed to subscribe to filter";
        qDebug() << errorMsg;
        if (callback) {
            callback(false, errorMsg);
        }
        delete data;
    }
}

void Waku::connectPeer(const QString &peerMultiAddr, unsigned int timeoutMs, 
                      WakuConnectCallback callback) {
    qDebug() << "Connecting to peer...";
    if (!wakuCtx) {
        QString errorMsg = "Waku not initialized";
        qDebug() << errorMsg;
        if (callback) {
            callback(false, errorMsg);
        }
        return;
    }

    // Create connect data for the callback
    auto* data = new ConnectData{this, callback};

    // Convert QString to UTF-8 C string
    QByteArray peerMultiAddrUtf8 = peerMultiAddr.toUtf8();

    qDebug() << "Connecting to peer..." << peerMultiAddrUtf8.constData();
    // Call the waku_connect function
    int ret = waku_connect(
        wakuCtx,
        peerMultiAddrUtf8.constData(),
        timeoutMs,
        connect_callback,
        data
    );

    if (ret != RET_OK) {
        QString errorMsg = "Failed to connect to peer";
        qDebug() << errorMsg;
        if (callback) {
            callback(false, errorMsg);
        }
        delete data;
    }
}

void Waku::storeQuery(const QString &jsonQuery, const QString &peerAddr, 
                     unsigned int timeoutMs, WakuStoreQueryCallback callback) {
    qDebug() << "Executing store query...";
    if (!wakuCtx) {
        QString errorMsg = "Waku not initialized";
        qDebug() << errorMsg;
        if (callback) {
            callback(false, errorMsg);
        }
        return;
    }

    // Create store query data for the callback
    auto* data = new StoreQueryData{this, callback};

    // Convert QString to UTF-8 C string
    QByteArray jsonQueryUtf8 = jsonQuery.toUtf8();
    QByteArray peerAddrUtf8 = peerAddr.toUtf8();

    qDebug() << "Querying store node..." << peerAddrUtf8.constData();
    // Call the waku_store_query function
    int ret = waku_store_query(
        wakuCtx,
        jsonQueryUtf8.constData(),
        peerAddrUtf8.constData(),
        timeoutMs,
        store_query_callback,
        data
    );

    if (ret != RET_OK) {
        QString errorMsg = "Failed to execute store query";
        qDebug() << errorMsg;
        if (callback) {
            callback(false, errorMsg);
        }
        delete data;
    }
}

void Waku::destroyWaku(WakuDestroyCallback callback) {
    qDebug() << "Destroying Waku...";
    if (!wakuCtx) {
        QString errorMsg = "Waku not initialized";
        qDebug() << errorMsg;
        if (callback) {
            callback(false, errorMsg);
        }
        return;
    }

    // Create destroy data for the callback
    auto* data = new DestroyData{this, callback};

    // Call the waku_destroy function
    int ret = waku_destroy(
        wakuCtx,
        destroy_callback,
        data
    );

    if (ret != RET_OK) {
        QString errorMsg = "Failed to destroy Waku";
        qDebug() << errorMsg;
        if (callback) {
            callback(false, errorMsg);
        }
        delete data;
    } else {
        // Set wakuCtx to null since we're destroying it
        wakuCtx = nullptr;
    }
}

void Waku::setEventCallback(WakuEventCallback callback) {
    qDebug() << "Setting event callback...";
    if (!wakuCtx) {
        qDebug() << "Waku not initialized, cannot set event callback";
        return;
    }

    // Store the callback in the class member
    eventCallback = callback;

    // Create event data to pass to the C callback
    // This needs to persist for the lifetime of the waku node,
    // so we'll let the waku node manage its lifecycle
    auto* data = new EventData{this, callback};

    // Set the event callback
    waku_set_event_callback(wakuCtx, event_callback, data);
    
    qDebug() << "Event callback set successfully";
} 
