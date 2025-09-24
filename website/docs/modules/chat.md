---
title: Chat Module
description: Overview and usage of the Logos chat module.
---

# Chat Module

## Overview

The Chat Module is a simple proof of concept Chat built on top of `Waku`.

## API

### Methods

- `initialize()` → `bool`
  - Starts internal wiring with `waku_module` and installs an event handler that decodes incoming chat payloads and emits `chatMessage` events.
  - Returns `true` on success.

- `joinChannel(channelName: string)` → `bool`
  - Subscribes to the channel's content topic via `Waku Filter` and tracks it for incoming event dispatch.
  - Accepts either a bare channel (e.g. `"general"`) or a full content topic (e.g. `"/toy-chat/2/general/proto"`).
  - Returns `true` if subscription succeeded.

- `sendMessage(channelName: string, username: string, message: string)`
  - Encodes a protobuf chat message and publishes it on the default pubsub topic with the channel's content topic.
  - Accepts either bare channel or full content topic.
  - Does not return a value; logs errors if publish fails.

- `retrieveHistory(channelName: string)` → `bool`
  - Queries a `Waku Store` node for recent messages on the channel's content topic and emits a `historyMessage` event for each decoded entry.
  - Returns `true` if the query wiring starts successfully (results arrive via events).

### Events

- `chatMessage`
  - Emitted for every decoded live message on any joined channel.
  - Payload (`QVariantList` / JSON array): `[timestamp: string, nick: string, message: string]`.
  - FFI event JSON example: `{ "event": "chatMessage", "data": ["2025-01-01 12:00:00 UTC", "alice", "hello"] }`.

- `historyMessage`
  - Emitted for each decoded message returned by `retrieveHistory`.
  - Payload (`QVariantList` / JSON array): `[timestamp: string, nick: string, message: string]`.

### Notes

- Call `initialize()` once before `joinChannel()` and `sendMessage()`.

## Examples

### C++

Using generated `C++` wrappers from another module

```cpp
// Subscribe to live messages
logos.chat.on("chatMessage", [](const QVariantList& data) {
    const auto ts = data[0].toString();
    const auto nick = data[1].toString();
    const auto text = data[2].toString();
    qInfo() << ts << nick << ":" << text;
});

// Initialize and join a channel
logos.chat.initialize();
logos.chat.joinChannel("general");

// Send a message using the generated API
logos.chat.sendMessage("general", "alice", "Hello from another module!");
```

### Javascript

Using the `JavaScript` SDK (reflective API)

```javascript
const LogosAPI = require('logos-api');

const logos = new LogosAPI();
logos.start();
logos.processAndLoadPlugins(["waku_module", "chat"]);
logos.startEventProcessing();

// Subscribe to events
logos.chat.onChatMessage((evt) => {
  console.log('chatMessage:', evt);
});

(async () => {
  await logos.chat.initialize();
  await logos.chat.joinChannel('general');
  await logos.chat.sendMessage('general', 'alice', 'Hello Logos!');
  await logos.chat.retrieveHistory('general');
})();
```
