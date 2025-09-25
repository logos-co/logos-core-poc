---
title: Architecture
description: Internal architecture and design goals of liblogos.
---

... needs a lot more detail than this...

### Publishing Your Own Module

Modules expose their Qt objects through the SDK provider. The generated wrappers automatically enforce token checking by routing calls through the module proxy.

```cpp
#include "logos_api.h"
#include "logos_api_provider.h"

class ChatModule : public QObject {
    Q_OBJECT
    Q_INVOKABLE bool initialize();
    // ...
};

ChatModule module;
LogosAPI logos{"chat"};
logos.getProvider()->registerObject("chat", &module);
```

Once registered, other modules can use the generated `Chat` proxy to call `initialize`, `joinChannel`, `sendMessage`, and subscribe to `chatMessage` events.

## Core Concepts

### LogosAPI Runtime

`LogosAPI` is the entry point a module instantiates with its own name. It creates:
- a `LogosAPIProvider` that publishes locally owned objects via Qt Remote Objects and wraps them with `ModuleProxy` for token enforcement and event forwarding.
- a cached set of `LogosAPIClient` instances, one per remote module, that handle replica acquisition, method invocation, and callback dispatching.
- a shared `TokenManager` responsible for negotiating and storing inter-module tokens.

### Generated Module Wrappers

Each header under `logos-cpp-sdk/cpp/generated/` corresponds to a Logos module (for example `chat_api.h`, `core_manager_api.h`, `package_manager_api.h`, `waku_module_api.h`). Wrappers:
- Provide typed C++ member functions that forward to `invokeRemoteMethod` with the correct module name.
- Offer `on` helpers for connecting lambdas to remote `eventResponse` signals, and `trigger` helpers for emitting events from your module back to clients.
- Cache remote replicas for event wiring, reducing boilerplate around `requestObject` and Qt signal signatures.
- Are grouped by `LogosModules`, a convenience struct that builds all known wrappers from a shared `LogosAPI` instance.

### Event Handling

All generated classes expose:
- `bool on(const QString&, RawEventCallback)` and a convenience overload that unwraps only the payload `QVariantList`.
- `setEventSource(QObject*)` to tell the wrapper which local QObject should emit events when you call `trigger`.
- `trigger(...)` overloads that package arguments into `QVariantList` and forward them via `LogosAPIClient::onEventResponse` so that subscribers receive `eventResponse` notifications.

## API Reference

You can interact diretly with the Logos API without using the generated API if you so wish

### LogosAPI

| Member | Description |
|--------|-------------|
| `explicit LogosAPI(const QString& moduleName, QObject* parent = nullptr)` | Sets up provider, token manager, and client cache for `moduleName`. |
| `LogosAPIProvider* getProvider() const` | Returns the provider used to register local objects. |
| `LogosAPIClient* getClient(const QString& targetModule) const` | Lazily creates and returns a client for the target module. |

### LogosAPIProvider

| Member | Description |
|--------|-------------|
| `bool registerObject(const QString& name, QObject* object)` | Publishes `object` under `local:logos_<module>` and wraps it in `ModuleProxy`. Calls `initLogos` on the object when available. |
| `void unregisterObject()` | Stops exposing the previously registered object. |
| `ModuleProxy* proxy() const` | Access the proxy to inform tokens or inspect metadata. |

### LogosAPIClient

| Member | Description |
|--------|-------------|
| `QObject* requestObject(const QString& objectName, int timeoutMs = 20000)` | Retrieves a replica for events or direct method invocation. |
| `QVariant invokeRemoteMethod(...)` | Overloaded for 0–5 arguments; forwards the call together with the caller token. |
| `void onEvent(QObject* origin, QObject* destination, const QString& event, Callback)` | Subscribes to remote events and dispatches results. |
| `void onEventResponse(QObject* replica, const QString& eventName, const QVariantList& data)` | Emits an event to connected replicas (used when your module is the event source). |
| `bool informModuleToken(...)` | Shares capability and module tokens with dependents. |
