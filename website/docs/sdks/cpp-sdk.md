---
title: C++ SDK
slug: /sdks/cpp-sdk
description: Generated C++ wrappers for building Logos modules and applications.
---

# C++ SDK

## Quick Start

### Basic Usage

```cpp
#include <QCoreApplication>
#include "logos_api.h"
#include "generated/logos_sdk.h"

int main(int argc, char** argv) {
    QCoreApplication app(argc, argv);

    // Expose this module as "chat_client".
    LogosAPI logos{"chat_client"};
    LogosModules modules{&logos};

    modules.core_manager.initialize(argc, argv);
    modules.core_manager.setPluginsDirectory("./build/plugins");
    modules.core_manager.start();
    // Provide the built plugin path (shared library extension varies per platform).
    modules.core_manager.processPlugin("./modules/waku_module.dylib");
    modules.core_manager.processPlugin("./modules/chat_module.dylib");
    modules.core_manager.loadPlugin("waku_module");
    modules.core_manager.loadPlugin("chat");

    modules.chat.on("chatMessage", [](const QVariantList& data) {
        const auto timestamp = data[0].toString();
        const auto nick = data[1].toString();
        const auto text = data[2].toString();
        qInfo() << timestamp << nick << ":" << text;
    });

    modules.chat.initialize();
    modules.chat.joinChannel("general");
    modules.chat.sendMessage("general", "alice", "Hello Logos!");

    return app.exec();
}
```

## Examples

### Subscribing to Chat Events from a UI Layer

```cpp
// Acquire replicas only when needed and reuse them via the generated wrapper.
modules.chat.on("chatMessage", [this](const QVariantList& data) {
    const QString timestamp = data[0].toString();
    const QString nick = data[1].toString();
    const QString message = data[2].toString();
    QMetaObject::invokeMethod(this, "appendMessage", Qt::QueuedConnection,
                              Q_ARG(QString, timestamp),
                              Q_ARG(QString, nick),
                              Q_ARG(QString, message));
});
```

### Triggering Events from Your Module

```cpp
class ChatBridge : public QObject {
    Q_OBJECT
public:
    explicit ChatBridge(LogosModules& modules)
        : m_modules(modules) {
        m_modules.chat.setEventSource(this);
    }

    void forwardToRemote(const QString& timestamp, const QString& nick, const QString& message) {
        m_modules.chat.trigger("chatMessage", this, timestamp, nick, message);
    }

private:
    LogosModules& m_modules;
};
```

## Troubleshooting

- **Cannot connect to registry**: Ensure the target module is running and has registered its provider. The client will connect to `local:logos_<module>`; verify that socket exists and that `LOGOS_HOST_PATH` points to a built `logos_host` binary.
- **Plugin methods return invalid QVariant**: The core must be started (`core_manager.start()`) and the module must be fully loaded. Missing capability tokens also cause silent failures—check the capability module logs.
- **Events never arrive**: Confirm you called `on(...)` after the remote module published its object, and keep the Qt event loop alive (`QCoreApplication::exec()` or `logos_core_process_events()` in embedding scenarios).
- **Generated headers missing**: Re-run the build from the repository root; the generated wrappers in `logos-cpp-sdk/cpp/generated/` are committed but expect CMake to add the directory to include paths.
