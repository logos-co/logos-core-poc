#include "chat_api.h"
#include <unordered_set> // Add for storing message hashes
#include <QObject> // For QMetaObject and Q_ARG

// Constants
const std::string TOY_CHAT_CONTENT_TOPIC = "/toy-chat/2/huilong/proto";
const std::string DEFAULT_PUBSUB_TOPIC = "/waku/2/rs/16/32";
const std::string STORE_NODE = "/dns4/store-01.do-ams3.status.staging.status.im/tcp/30303/p2p/16Uiu2HAm3xVDaz6SRJ6kErwC21zBJEZjavVXg7VSkoWzaV1aMA3F";
const std::string CONTENT_TOPIC_PREFIX = "/toy-chat/2/";
const std::string CONTENT_TOPIC_SUFFIX = "/proto";

// Global variables
void* userData = nullptr;
std::vector<std::string> subscribedChannels;

// Set to store message hashes we've already processed
std::unordered_set<std::string> processedMessageHashes;

// Global app state
AppState appState;

// Function declarations
const int RET_OK = 0; // Define RET_OK since we no longer have libwaku.h

// Helper function to format a channel name into a content topic
std::string formatContentTopic(const std::string& channelName) {
    // Return the formatted content topic
    return CONTENT_TOPIC_PREFIX + channelName + CONTENT_TOPIC_SUFFIX;
}

// Get the current UTC timestamp in seconds
uint64_t getCurrentTimestampProto() {
  using namespace std::chrono;
  return duration_cast<seconds>(system_clock::now().time_since_epoch()).count();
}

// Format a timestamp to human-readable format
std::string formatTimestampProto(uint64_t timestamp) {
  time_t time = static_cast<time_t>(timestamp);
  std::tm tm = *std::gmtime(&time);
  std::stringstream ss;
  ss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S UTC");
  return ss.str();
}

// Create a new Chat2Message
chat::Chat2Message createMessage(const std::string& nick, const std::string& message) {
  chat::Chat2Message chat_msg;
  chat_msg.set_timestamp(getCurrentTimestampProto());
  chat_msg.set_nick(nick);
  chat_msg.set_payload(message);
  return chat_msg;
}

// Print a Chat2Message
void printMessage(const chat::Chat2Message& message) {
  std::cout << "Timestamp: " << formatTimestampProto(message.timestamp()) << std::endl;
  std::cout << "Nick: " << message.nick() << std::endl;
  std::cout << "Message: " << message.payload() << std::endl;
}

// Create a string from a vector of bytes
std::string bytesToStringProto(const std::vector<uint8_t>& bytes) {
  std::string result;
  for (size_t i = 0; i < bytes.size(); ++i) {
    if (i > 0) result += ",";
    result += std::to_string(bytes[i]);
  }
  return "[" + result + "]";
}

// Decode a binary payload into a DecodedMessage
DecodedMessage decodeProto(const std::vector<uint8_t>& payload) {
  DecodedMessage result;
  result.success = false;
  
  std::string binary_data(payload.begin(), payload.end());
  chat::Chat2Message message;
  
  if (message.ParseFromString(binary_data)) {
    result.success = true;
    result.timestamp = formatTimestampProto(message.timestamp());
    result.nick = message.nick();
    result.payload = message.payload();
  }
  
  return result;
}

// Print a decoded message
void printDecodedMessage(const DecodedMessage& message, const std::vector<uint8_t>& originalPayload) {
  if (message.success) {
    std::cout << "Successfully decoded message:" << std::endl;
    std::cout << "Timestamp: " << message.timestamp << std::endl;
    std::cout << "Nick: " << message.nick << std::endl;
    std::cout << "Message: " << message.payload << std::endl;
  } else {
    std::cout << "Failed to decode message from payload: " << bytesToStringProto(originalPayload) << std::endl;
  }
  std::cout << std::endl;
}

// Decode and print a Chat2Message from a binary payload (combined operation)
void decodePayloadProto(const std::vector<uint8_t>& payload) {
  auto decodedMsg = decodeProto(payload);
  printDecodedMessage(decodedMsg, payload);
}

// Function to format a timestamp to a human-readable string
std::string formatTimestamp(uint64_t timestamp) {
  time_t time = static_cast<time_t>(timestamp);
  std::tm tm = *std::gmtime(&time);
  std::stringstream ss;
  ss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S UTC");
  return ss.str();
}

// Signal handler
void signalHandler(int signal) {
    std::cout << "Received signal " << signal << ", shutting down..." << std::endl;
    appState.running = false;
}

// Store query callback
void storeQueryCallback(int callerRet, const char* msg, size_t len, void* userData) {
    std::cout << "\n\n\nstoreQueryCallback called with callerRet: " << callerRet << std::endl;

    // Get the message callback from the context
    StoreQueryContext* context = static_cast<StoreQueryContext*>(userData);
    MessageCallback callback = nullptr;
    if (context != nullptr) {
        callback = context->callback;
    }

    if (callerRet == RET_OK && msg != nullptr && len > 0) {
        std::string jsonStr(msg, len);
        // Find all payloads in the JSON
        size_t pos = 0;
        size_t messageCount = 0;
        while ((pos = jsonStr.find("\"payload\":[", pos)) != std::string::npos) {
            messageCount++;
            pos += 11; // Skip "payload":[ part
            // Find end of payload array
            size_t endPos = jsonStr.find("]", pos);
            if (endPos != std::string::npos) {
                std::string payloadStr = jsonStr.substr(pos, endPos - pos);
                std::cout << "Raw payload " << messageCount << ": [" << payloadStr << "]" << std::endl;
                // Convert payload string to vector of bytes
                std::vector<uint8_t> payloadBytes;
                std::stringstream ss(payloadStr);
                std::string numberStr;
                while (std::getline(ss, numberStr, ',')) {
                    payloadBytes.push_back(static_cast<uint8_t>(std::stoi(numberStr)));
                }
                // Decode the payload
                std::cout << "Attempting to decode payload " << messageCount << ":" << std::endl;
                auto decodedMsg = decodeProto(payloadBytes);
                printDecodedMessage(decodedMsg, payloadBytes);
                
                // Call the user callback if provided and message was decoded successfully
                if (callback && decodedMsg.success) {
                    callback(decodedMsg.timestamp, decodedMsg.nick, decodedMsg.payload);
                }
                
                std::cout << "----------------------------------------" << std::endl;
            }
        }
        std::cout << "Total messages found: " << messageCount << std::endl;
    }
    else if (callerRet != RET_OK) {
        std::cerr << "Store query error: " << callerRet;
        if (msg != nullptr && len > 0) {
            std::cerr << " - " << std::string(msg, len);
        }
        std::cerr << std::endl;
    }

    // Clean up the context
    if (context != nullptr) {
        delete context;
    }
}

// Event handler for incoming messages
void event_handler(int callerRet, const char* msg, size_t len, void* userData) {
    if (msg == nullptr) {
        std::cerr << "event_handler received null message" << std::endl;
        return;
    }
    
    std::string jsonStr(msg);
    
    // Check for message hash to avoid duplicates
    size_t hashPos = jsonStr.find("\"messageHash\":");
    if (hashPos != std::string::npos) {
        size_t hashStart = jsonStr.find("\"", hashPos + 14) + 1;
        size_t hashEnd = jsonStr.find("\"", hashStart);
        if (hashStart != std::string::npos && hashEnd != std::string::npos) {
            std::string messageHash = jsonStr.substr(hashStart, hashEnd - hashStart);
            
            // If we've already processed this message, skip it
            if (processedMessageHashes.find(messageHash) != processedMessageHashes.end()) {
                std::cout << "Skipping duplicate message with hash: " << messageHash << std::endl;
                return;
            }
            
            // Otherwise, add it to our set of processed hashes
            processedMessageHashes.insert(messageHash);
            std::cout << "Processing new message with hash: " << messageHash << std::endl;
        }
    }

    // Debug log the message
    std::cout << "event_handler called with callerRet: " << callerRet << std::endl;
    

    EventHandlerContext* context = static_cast<EventHandlerContext*>(userData);
    MessageCallback callback = nullptr;
    if (context != nullptr) {
        callback = context->callback;
    }

    // Check if the message contains "contentTopic" field
    size_t contentTopicPos = jsonStr.find("\"contentTopic\":");
    if (contentTopicPos != std::string::npos) {
        // Find the start and end of the content topic value
        size_t valueStart = jsonStr.find("\"", contentTopicPos + 14) + 1;
        size_t valueEnd = jsonStr.find("\"", valueStart);
        if (valueStart != std::string::npos && valueEnd != std::string::npos) {
            std::string contentTopic = jsonStr.substr(valueStart, valueEnd - valueStart);

            // Check if the content topic is in our list of subscribed channels
            bool isSubscribed = false;
            for (const auto& channel : subscribedChannels) {
                if (contentTopic == channel) {
                    isSubscribed = true;
                    break;
                }
            }

            // Only process if the content topic matches one of our subscribed channels
            if (isSubscribed) {
                std::cout << "\nReceived message with matching content topic: " << contentTopic << std::endl;
                // Extract the payload
                size_t payloadPos = jsonStr.find("\"payload\":\"");
                if (payloadPos != std::string::npos) {
                    size_t payloadStart = payloadPos + 11; // Skip "payload":"
                    size_t payloadEnd = jsonStr.find("\"", payloadStart);
                    if (payloadStart != std::string::npos && payloadEnd != std::string::npos) {
                        std::string encodedPayload = jsonStr.substr(payloadStart, payloadEnd - payloadStart);
                        std::cout << "Encoded payload: " << encodedPayload << std::endl;
                        // Decode the base64 payload
                        std::vector<uint8_t> decodedBytes = base64Decode(encodedPayload);
                        // Decode the protobuf message
                        std::cout << "Decoding protobuf payload:" << std::endl;
                        auto decodedMsg = decodeProto(decodedBytes);
                        printDecodedMessage(decodedMsg, decodedBytes);
                        
                        // Call the user callback if provided and message was decoded successfully
                        if (callback && decodedMsg.success) {
                            callback(decodedMsg.timestamp, decodedMsg.nick, decodedMsg.payload);
                        }
                    }
                }
            }
        }
    }
}

// Base64 decoding function
std::vector<uint8_t> base64Decode(const std::string& encoded) {
    std::string base64_chars =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
        "0123456789+/";
    std::vector<uint8_t> decoded;
    int val = 0, valb = -8;
    for (char c : encoded) {
        if (c == '=') break;
        size_t pos = base64_chars.find(c);
        if (pos == std::string::npos) continue;
        val = (val << 6) + static_cast<int>(pos);
        valb += 6;
        if (valb >= 0) {
            decoded.push_back(static_cast<uint8_t>((val >> valb) & 0xFF));
            valb -= 8;
        }
    }
    return decoded;
}

// Base64 encoding function
std::string base64Encode(const std::vector<uint8_t>& data) {
    std::string base64_chars =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
        "0123456789+/";
    std::string encoded;
    int val = 0, valb = -6;
    for (uint8_t c : data) {
        val = (val << 8) + c;
        valb += 8;
        while (valb >= 0) {
            encoded.push_back(base64_chars[(val >> valb) & 0x3F]);
            valb -= 6;
        }
    }
    if (valb > -6)
        encoded.push_back(base64_chars[((val << 8) >> (valb + 8)) & 0x3F]);
    // Add padding
    while (encoded.size() % 4)
        encoded.push_back('=');
    return encoded;
}

// Function to create a chat message
ChatMessage createChatMessage(const std::string& username, const std::string& message) {
    // Use ChatMessage constructor from protocol.h
    return ChatMessage(username, message);
}

// Function to encode a chat message using protobuf
bool encodeProto(const ChatMessage& msg, std::vector<uint8_t>& output) {
    // Use the ChatMessage's serialize method
    output = msg.serialize();
    return true;
}

// Function to send a message
void sendMessage(void* wakuCtx, const std::string& channelName, const std::string& username, const std::string& message) {
    // Format the channel name into a content topic if not already formatted
    std::string contentTopic = channelName;
    if (channelName.find("/toy-chat/") == std::string::npos) {
        contentTopic = formatContentTopic(channelName);
    }
    
    std::cout << "Sending message to channel: " << channelName << std::endl;
    std::cout << "Using content topic: " << contentTopic << std::endl;

    // Get waku plugin (if available)
    WakuInterface* wakuPlugin = PluginRegistry::getPlugin<WakuInterface>("waku");

    // Create a new chat message
    ChatMessage chatMsg = createChatMessage(username, message);
    // Encode the message
    std::vector<uint8_t> encodedBytes = chatMsg.serialize();
    if (encodedBytes.empty()) {
        std::cerr << "Failed to encode message" << std::endl;
        return;
    }
    // Base64 encode the payload
    std::string base64Payload = base64Encode(encodedBytes);
    // Create the Waku message JSON
    std::string messageJson = R"({
        "payload": ")" + base64Payload + R"(",
        "contentTopic": ")" + contentTopic + R"(",
        "version": 1,
        "timestamp": )" + std::to_string(std::chrono::duration_cast<std::chrono::nanoseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count()) + R"(,
        "ephemeral": false
    })";
    std::cout << "Sending message as " << username << ": " << message << std::endl;
    std::cout << "Message JSON: " << messageJson << std::endl;

    // Publish using the waku plugin
    if (wakuPlugin) {
        wakuPlugin->relayPublish(
            QString::fromStdString(DEFAULT_PUBSUB_TOPIC),
            QString::fromStdString(messageJson),
            30000,  // timeout in ms
            [username, message](bool success, const QString &responseMsg) {
                std::cout << "Waku Plugin relay publish result for message from " << username << ": " 
                          << (success ? "Success" : "Failed") << " - " << responseMsg.toStdString() << std::endl;
            }
        );
    }
}

// Function to initialize and start a Waku node
void* initAndStart(const std::string& relayTopic, MessageCallback messageCallback) {
    // Create appropriate Waku config
    std::string configStr = R"({
        "host": "0.0.0.0",
        "tcpPort": 60010,
        "key": null,
        "clusterId": 16,
        "relay": true,
        "relayTopics": [")" + relayTopic + R"("],
        "shards": [1,32,64,128,256],
        "maxMessageSize": "1024KiB",
        "dnsDiscovery": true,
        "dnsDiscoveryUrl": "enrtree://AMOJVZX4V6EXP7NTJPMAYJYST2QP6AJXYW76IU6VGJS7UVSNDYZG4@boot.prod.status.nodes.status.im",
        "discv5Discovery": false,
        "discv5EnrAutoUpdate": false,
        "logLevel": "INFO",
        "keepAlive": true
    })";

    std::cout << "Waku node config: " << configStr << std::endl;

    // Get waku plugin
    WakuInterface* wakuPlugin = PluginRegistry::getPlugin<WakuInterface>("waku");
    if (!wakuPlugin) {
        std::cerr << "Failed to get Waku plugin" << std::endl;
        return nullptr;
    }
    
    std::cout << "Found Waku Plugin, initializing" << std::endl;
    // Call initWaku on the plugin using QMetaObject::invokeMethod
    // * wakuPlugin = PluginRegistry::getPlugin<WakuInterface>("waku");
    QObject* the_plugin = PluginRegistry::getPlugin<QObject>("waku");
    QMetaObject::invokeMethod(the_plugin, 
                             "initWaku",
                             Qt::DirectConnection,
                             Q_ARG(QString, QString::fromStdString(configStr)),
                             Q_ARG(std::function<void(bool, const QString&)>, [](bool success, const QString &message) {
                                 std::cout << "Waku Plugin init result: " << (success ? "Success" : "Failed") << " - " << message.toStdString() << std::endl;
                             }));

    std::this_thread::sleep_for(std::chrono::seconds(3));

    // Create event handler context
    EventHandlerContext* context = new EventHandlerContext(messageCallback);

    wakuPlugin->setEventCallback([context](const QString &event) {
        // Convert QString to std::string
        std::string eventStr = event.toStdString();
        // Convert to C-style string and call event_handler
        event_handler(RET_OK, eventStr.c_str(), eventStr.length(), context);
    });

    // Start Waku plugin
    wakuPlugin->startWaku(
        [](bool success, const QString &message) {
            std::cout << "Waku Plugin start result: " << (success ? "Success" : "Failed") << " - " << message.toStdString() << std::endl;
        }
    );

    std::this_thread::sleep_for(std::chrono::seconds(2));
    std::cout << "Waku node started successfully" << std::endl;

    // Subscribe to the relay topic
    wakuPlugin->relaySubscribe(
        QString::fromStdString(relayTopic), 
        [](bool success, const QString &message) {
            std::cout << "Waku Plugin relay subscribe result: " << (success ? "Success" : "Failed") << " - " << message.toStdString() << std::endl;
        }
    );
    
    // Return a non-null pointer to indicate success
    // We're not using this for anything meaningful anymore
    return (void*)1;
}

// Function to join a chat channel
bool joinChannel(void* wakuCtx, const std::string& channelName, const std::string& relayTopic) {
    // Format the channel name into a content topic if not already formatted
    std::string contentTopic = channelName;
    if (channelName.find("/toy-chat/") == std::string::npos) {
        contentTopic = formatContentTopic(channelName);
    }

    std::cout << "Joining channel: " << channelName << std::endl;
    std::cout << "Subscribing to content topic: " << contentTopic << std::endl;

    // Get waku plugin
    WakuInterface* wakuPlugin = PluginRegistry::getPlugin<WakuInterface>("waku");
    if (!wakuPlugin) {
        std::cerr << "Failed to get Waku plugin" << std::endl;
        return false;
    }

    std::string contentTopics = "[\"" + contentTopic + "\"]";
    // Call filterSubscribe on the waku plugin
    wakuPlugin->filterSubscribe(
        QString::fromStdString(relayTopic),
        QString::fromStdString(contentTopics),
        [contentTopic](bool success, const QString &message) {
            std::cout << "Waku Plugin filter subscribe result for " << contentTopic << ": " 
                      << (success ? "Success" : "Failed") << " - " << message.toStdString() << std::endl;
            if (success) {
                subscribedChannels.push_back(contentTopic);
            }
        }
    );
    
    return true;
}

// Function to retrieve message history from store node
void retrieveHistory(void* wakuCtx, const std::string& channelName, MessageCallback callback) {
    // Format the channel name into a content topic if not already formatted
    std::string contentTopic = channelName;
    if (channelName.find("/toy-chat/") == std::string::npos) {
        contentTopic = formatContentTopic(channelName);
    }
    
    std::cout << "Retrieving message history for channel: " << channelName << std::endl;
    std::cout << "Using content topic: " << contentTopic << std::endl;

    // Get waku plugin
    WakuInterface* wakuPlugin = PluginRegistry::getPlugin<WakuInterface>("waku");
    if (!wakuPlugin) {
        std::cerr << "Failed to get Waku plugin" << std::endl;
        return;
    }

    // Calculate timestamp for 24 hours ago (in nanoseconds)
    uint64_t oneDay = 60 * 60 * 24;
    auto now = std::chrono::system_clock::now();
    auto nowSeconds = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count();
    uint64_t timeStart = (nowSeconds - oneDay) * 1000000000ULL; // Convert to nanoseconds

   std::string queryJson = R"({
       "request_id": "15be8c48-55ce-4bf2-a34-8813d4da2dec",
       "include_data": true,
       "content_topics": [")" + contentTopic + R"("],
       "time_start": 1744123537000000000,
       "pagination_forward": true,
       "pagination_limit": 100
   })";

    std::cout << "Query JSON: " << queryJson.c_str() << std::endl;

    // Create a context to hold the callback
    StoreQueryContext* context = new StoreQueryContext(callback);
    
    // Pass the main storeQueryCallback to the waku plugin
    wakuPlugin->storeQuery(
        QString::fromStdString(queryJson),
        QString::fromStdString(STORE_NODE),
        30000,  // timeout in ms
        [context, channelName](bool success, const QString &message) {
            std::cout << "Waku Plugin store query response for channel " << channelName << std::endl;
            if (success && !message.isEmpty()) {
                // Convert QString to std::string and call storeQueryCallback
                std::string messageStr = message.toStdString();
                storeQueryCallback(RET_OK, messageStr.c_str(), messageStr.length(), context);
            } else {
                std::cout << "Waku Plugin store query failed or returned empty response" << std::endl;
                if (context != nullptr) {
                    delete context; // Clean up the context
                }
            }
        }
    );
    
    std::cout << "History query sent to store node" << std::endl;
} 