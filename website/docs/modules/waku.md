---
title: Waku Module
description: Messaging transport powered by Waku within Logos.
---

# Waku Module

## Overview

The Waku Module wraps `nwaku` via `libwaku` and exposes functions to configure, start and interact with a Waku node. Other modules (e.g., `chat`) use it to publish, subscribe and query messages over Waku Relay, Filter and Store.

## API

### Methods

- `initWaku(cfg: string)` → `bool`
  - Creates a Waku context using the provided JSON configuration.
  - Returns `true` if the context was created.

- `startWaku()` → `bool`
  - Starts the Waku node asynchronously.
  - Returns `true` if the start was initiated successfully.

- `setEventCallback()` → `bool`
  - Registers a callback with Waku so incoming messages are forwarded as events.
  - Returns `true` when the callback is set.

- `relaySubscribe(pubSubTopic: string)` → `bool`
  - Subscribes to a pub/sub topic using Waku Relay.
  - Returns `true` if subscription was initiated.

- `relayPublish(pubSubTopic: string, jsonWakuMessage: string)` → `bool`
  - Publishes a JSON‑encoded `WakuMessage` to a pub/sub topic.
  - Returns `true` if publish was initiated.

- `filterSubscribe(pubSubTopic: string, contentTopics: string)` → `bool`
  - Subscribes to messages matching specific content topics using Waku Filter.
  - `contentTopics` is a JSON string (array) as expected by `libwaku`.
  - Returns `true` if subscription was initiated.

- `storeQuery(jsonQuery: string, peerAddr: string)` → `bool`
  - Performs a Store query for historical messages.
  - `jsonQuery` is a JSON string per `libwaku`'s API.
  - Returns `true` if the query was initiated.

- `foo(bar: string)` → `bool`
  - Test method that emits a debug event for wiring verification.

### Events

- `wakuMessage`
  - Emitted when an incoming live message is received via the registered event callback.
  - Payload (`QVariantList` / JSON array): `[messageJson: string, timestamp: string]`.

- `storeQueryResponse`
  - Emitted when a Store query completes.
  - Payload (`QVariantList` / JSON array): `[resultJson: string, timestamp: string]`.

- `fooTriggered`
  - Emitted by `foo()` for testing.
  - Payload (`QVariantList` / JSON array): `[bar: string, timestamp: string]`.

### Notes

- Call `initWaku()` before `startWaku()` and event‑related operations.
- Use `setEventCallback()` before expecting live `wakuMessage` events.
- `jsonWakuMessage`, `contentTopics`, and `jsonQuery` must follow `libwaku` inputs.

## Examples

### C++

Using generated `C++` wrappers from another module

```cpp
// Initialize and start Waku
logos.waku_module.initWaku(R"({"listenAddress":"0.0.0.0","port":60000})");
logos.waku_module.startWaku();
logos.waku_module.setEventCallback();

// Subscribe to events
logos.waku_module.on("wakuMessage", [](const QVariantList& data) {
    const auto json = data[0].toString();
    const auto ts = data[1].toString();
    qInfo() << "wakuMessage" << ts << json;
});

// Relay subscribe and publish
logos.waku_module.relaySubscribe("/waku/2/default-waku/proto");
logos.waku_module.relayPublish(
  "/waku/2/default-waku/proto",
  R"({"payload":"SGVsbG8gV2FrdSE=","contentTopic":"/toy-chat/2/general/proto"})"
);

// Store query
logos.waku_module.storeQuery(
  R"({"contentTopics":["/toy-chat/2/general/proto"],"pageSize":10})",
  "/ip4/127.0.0.1/tcp/15555/p2p/16Uiu2HAm..."
);
```

### Javascript

Using the `JavaScript` SDK (reflective API)

```javascript
const LogosAPI = require('logos-api');

const logos = new LogosAPI();
logos.start();
logos.processAndLoadPlugins(["waku_module"]);
logos.startEventProcessing();

// Subscribe to events
logos.waku_module.onWakuMessage((evt) => {
  console.log('wakuMessage:', evt);
});

(async () => {
  await logos.waku_module.initWaku('{"listenAddress":"0.0.0.0","port":60000}');
  await logos.waku_module.startWaku();
  await logos.waku_module.setEventCallback();
  await logos.waku_module.relaySubscribe('/waku/2/default-waku/proto');
  await logos.waku_module.relayPublish('/waku/2/default-waku/proto', '{"payload":"SGVsbG8=","contentTopic":"/toy-chat/2/general/proto"}');
  await logos.waku_module.storeQuery('{"contentTopics":["/toy-chat/2/general/proto"],"pageSize":5}', '/ip4/127.0.0.1/tcp/15555/p2p/16U...');
})();
```
