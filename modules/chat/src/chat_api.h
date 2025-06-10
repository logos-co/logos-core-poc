#ifndef CHAT_API_H
#define CHAT_API_H

#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <chrono>
#include <mutex>
#include <deque>
#include <atomic>
#include <cstring>
#include <csignal>
#include <sstream>
#include <functional>
#include <iomanip>
#include <fstream>
#include "protocol/protocol.h"
#include "message.pb.h"
#include "../../modules/waku_module/waku_module_interface.h"

// Forward declaration
class LogosAPI;

// Constants
extern const std::string TOY_CHAT_CONTENT_TOPIC;
extern const std::string DEFAULT_PUBSUB_TOPIC;
extern const std::string STORE_NODE;
extern const std::string CONTENT_TOPIC_PREFIX;
extern const std::string CONTENT_TOPIC_SUFFIX;

// Global variables
extern void* userData;
extern std::vector<std::string> subscribedChannels;

// Define a callback type for message handling
using MessageCallback = std::function<void(const std::string&, const std::string&, const std::string&)>;

// Structure to hold decoded message results
struct DecodedMessage {
    bool success;
    std::string timestamp;
    std::string nick;
    std::string payload;
};

// Store query context to hold callback function
struct StoreQueryContext {
    MessageCallback callback;
    
    StoreQueryContext(MessageCallback cb) : callback(cb) {}
};

// Event handler context to hold callback function
struct EventHandlerContext {
    MessageCallback callback;
    
    EventHandlerContext(MessageCallback cb) : callback(cb) {}
};

// Message history storage
struct AppState {
    std::deque<ChatMessage> messages;
    std::mutex messagesMutex;
    std::atomic<bool> running{true};
};

// Global app state
extern AppState appState;

// Function declarations
std::string formatContentTopic(const std::string& channelName);
uint64_t getCurrentTimestampProto();
std::string formatTimestampProto(uint64_t timestamp);
chat::Chat2Message createMessage(const std::string& nick, const std::string& message);
void printMessage(const chat::Chat2Message& message);
std::string bytesToStringProto(const std::vector<uint8_t>& bytes);
DecodedMessage decodeProto(const std::vector<uint8_t>& payload);
void printDecodedMessage(const DecodedMessage& message, const std::vector<uint8_t>& originalPayload);
void decodePayloadProto(const std::vector<uint8_t>& payload);
std::string formatTimestamp(uint64_t timestamp);
std::vector<uint8_t> base64Decode(const std::string& encoded);
std::string base64Encode(const std::vector<uint8_t>& data);
ChatMessage createChatMessage(const std::string& username, const std::string& message);
bool encodeProto(const ChatMessage& msg, std::vector<uint8_t>& output);
void sendMessage(LogosAPI* logosAPI, const std::string& channelName, const std::string& username, const std::string& message);
void signalHandler(int signal);
void relayTopicHealthCallback(int callerRet, const char* msg, size_t len, void* userData);
void connectionChangeCallback(int callerRet, const char* msg, size_t len, void* userData);
void storeQueryCallback(int callerRet, const char* msg, size_t len, void* userData);
void nodeOperationCallback(int callerRet, const char* msg, size_t len, void* userData);
void retrieveHistory(LogosAPI* logosAPI, const std::string& channelName, MessageCallback callback = nullptr);
void event_handler(int callerRet, const char* msg, size_t len, void* userData);
void* initAndStart(LogosAPI* logosAPI, const std::string& relayTopic, MessageCallback messageCallback = nullptr);
bool joinChannel(LogosAPI* logosAPI, const std::string& channelName, const std::string& relayTopic);

#endif // CHAT_API_H 