---
title: How to Create a Module
description: Steps for authoring new Logos modules with liblogos.
---

# How to Create a Logos Module

This guide walks you through creating a C++ Logos module (Qt plugin) that exposes methods and emits events over Qt Remote Objects using the Logos SDK. Use `modules/template_module` as a reference implementation.

Prerequisites

- Qt Core and Qt RemoteObjects available to CMake
- CMake and a C++17 toolchain
- Built core and generator once (the scripts handle this)

## 1) Create the Module Skeleton

Directory layout

```
modules/your_module/
  ├─ CMakeLists.txt
  ├─ metadata.json
  ├─ your_module_interface.h
  ├─ your_module_plugin.h
  └─ your_module_plugin.cpp
```

Use `modules/template_module` as a starting point.

## 2) Define the Interface

Your interface inherits from `PluginInterface` (from `logos-liblogos/interface.h`). Declare methods as `Q_INVOKABLE` so other modules can call them remotely, and keep the standard `eventResponse` signal for events.

Example: `your_module_interface.h`

```cpp
#pragma once
#include <QtCore/QObject>
#include <QtCore/QVariant>
#include "../../logos-liblogos/interface.h"

class YourModuleInterface : public PluginInterface {
public:
    virtual ~YourModuleInterface() {}

    Q_INVOKABLE virtual bool doWork(const QString& input) = 0;

signals:
    // Required for event delivery via replicas
    void eventResponse(const QString& eventName, const QVariantList& data);
};

#define YourModuleInterface_iid "org.logos.YourModuleInterface"
Q_DECLARE_INTERFACE(YourModuleInterface, YourModuleInterface_iid)
```

Notes

- Methods marked `Q_INVOKABLE` are callable via the SDK and over Remote Objects.
- Keep method signatures Qt-friendly (Qt types or simple PODs).

## 3) Implement the Plugin (with generated C++ API)

Subclass `QObject` and your interface, add the required Qt plugin macros, and implement your logic. Implement `initLogos(LogosAPI*)` to receive the SDK handle and construct the generated wrappers.

Header: `your_module_plugin.h`

```cpp
#pragma once
#include <QtCore/QObject>
#include "your_module_interface.h"
#include "../../logos-cpp-sdk/cpp/logos_api.h"
#include "../../logos-cpp-sdk/cpp/generated/logos_sdk.h"

class YourModulePlugin : public QObject, public YourModuleInterface {
    Q_OBJECT
    Q_PLUGIN_METADATA(IID YourModuleInterface_iid FILE "metadata.json")
    Q_INTERFACES(YourModuleInterface PluginInterface)
public:
    QString name() const override { return "your_module"; }
    QString version() const override { return "1.0.0"; }

    Q_INVOKABLE bool doWork(const QString& input) override;
    Q_INVOKABLE void initLogos(LogosAPI* logosAPIInstance);

signals:
    void eventResponse(const QString& eventName, const QVariantList& data);

private:
    LogosModules* logos = nullptr; // generated wrappers aggregator
};
```

Implementation: `your_module_plugin.cpp`

```cpp
#include "your_module_plugin.h"
#include <QDebug>

void YourModulePlugin::initLogos(LogosAPI* logosAPIInstance) {
    // Provided by PluginInterface base
    logosAPI = logosAPIInstance;
    delete logos; // if re-initializing
    logos = logosAPI ? new LogosModules(logosAPI) : nullptr;
    if (logos) {
        // Enable event helper for this module
        logos->your_module.setEventSource(this);
    }
}

bool YourModulePlugin::doWork(const QString& input) {
    qDebug() << "doWork called with" << input;

    // Example: call another module via generated wrappers
    // Ensure that module appears in metadata.json "dependencies"
    // bool ok = logos->waku_module.relayPublish("/topic", someJson);

    // Emit an event using the generated helper
    if (logos) {
        logos->your_module.trigger("workDone", input);
    }
    return true;
}
```

Notes

- You do not register the object yourself; the `logos_host` loads your plugin and the provider calls `initLogos(LogosAPI*)` before exposing it.
- Prefer the generated wrappers (`logos->module.method(...)`) over `getClient(...)` for readability and type-safety.

## 4) Add `metadata.json` (declare dependencies for generation)

This file declares the module’s identity and capabilities.

Example: `metadata.json`

```json
{
  "name": "your_module",
  "version": "1.0.0",
  "description": "My sample module",
  "author": "Your Name",
  "type": "core",
  "category": "example",
  "main": "your_module_plugin",
  "dependencies": ["waku_module"],
  "capabilities": []
}
```

Fields and generator behavior

- `name`, `version`, `description`, `author`: basic metadata
- `type`: typically `core`
- `category`: grouping tag
- `main`: the plugin class base name used by the loader (e.g., `template_module_plugin`)
- `dependencies`: modules your plugin needs to call. The C++ generator reads this list to emit typed wrappers so you can call them as `logos-><module>.<method>(...)`. It also emits a wrapper for your own module and always includes `core_manager`.
- `capabilities`: feature flags used by the capability module

## 5) CMake Configuration (enable generated API)

Follow the template’s CMake setup. Key points:

- Find Qt Core and RemoteObjects
- Run the Logos C++ generator on your `metadata.json` before building
- Build a shared library without the `lib` prefix
- Link Qt and include the SDK sources

Minimal `CMakeLists.txt` excerpt

```cmake
set(CMAKE_AUTOMOC ON)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core RemoteObjects)

set(CPP_GENERATOR "${CMAKE_SOURCE_DIR}/../build/cpp-generator/bin/logos-cpp-generator")
set(REPO_ROOT "${CMAKE_SOURCE_DIR}/..")
set(PLUGINS_OUTPUT_DIR "${CMAKE_BINARY_DIR}/modules")
set(METADATA_JSON "${CMAKE_CURRENT_SOURCE_DIR}/metadata.json")

add_custom_target(run_cpp_generator_your_module
  COMMAND "${CPP_GENERATOR}" --metadata "${METADATA_JSON}" --module-dir "${PLUGINS_OUTPUT_DIR}"
  WORKING_DIRECTORY "${REPO_ROOT}")
add_dependencies(run_cpp_generator_your_module cpp_generator_build)

set(PLUGIN_SOURCES
  your_module_plugin.cpp
  your_module_plugin.h
  your_module_interface.h
  ${CMAKE_SOURCE_DIR}/../logos-liblogos/interface.h
  ${CMAKE_CURRENT_SOURCE_DIR}/../../logos-cpp-sdk/cpp/generated/logos_sdk.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/../../logos-cpp-sdk/cpp/logos_api.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/../../logos-cpp-sdk/cpp/logos_api.h
  ${CMAKE_CURRENT_SOURCE_DIR}/../../logos-cpp-sdk/cpp/logos_api_client.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/../../logos-cpp-sdk/cpp/logos_api_client.h
  ${CMAKE_CURRENT_SOURCE_DIR}/../../logos-cpp-sdk/cpp/logos_api_consumer.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/../../logos-cpp-sdk/cpp/logos_api_consumer.h
  ${CMAKE_CURRENT_SOURCE_DIR}/../../logos-cpp-sdk/cpp/logos_api_provider.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/../../logos-cpp-sdk/cpp/logos_api_provider.h
  ${CMAKE_CURRENT_SOURCE_DIR}/../../logos-cpp-sdk/cpp/token_manager.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/../../logos-cpp-sdk/cpp/token_manager.h
  ${CMAKE_CURRENT_SOURCE_DIR}/../../logos-cpp-sdk/cpp/module_proxy.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/../../logos-cpp-sdk/cpp/module_proxy.h)

add_library(your_module_plugin SHARED ${PLUGIN_SOURCES})
add_dependencies(your_module_plugin run_cpp_generator_your_module)
set_target_properties(your_module_plugin PROPERTIES PREFIX "")

target_link_libraries(your_module_plugin PRIVATE Qt${QT_VERSION_MAJOR}::Core Qt${QT_VERSION_MAJOR}::RemoteObjects)
target_include_directories(your_module_plugin PRIVATE 
  ${CMAKE_CURRENT_SOURCE_DIR} 
  ${CMAKE_SOURCE_DIR}/../logos-liblogos 
  ${CMAKE_CURRENT_SOURCE_DIR}/../../logos-cpp-sdk/cpp 
  ${CMAKE_CURRENT_SOURCE_DIR}/../../logos-cpp-sdk/cpp/generated)

set_target_properties(your_module_plugin PROPERTIES
  LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/modules"
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/modules")
```

For a full example, see `modules/template_module/CMakeLists.txt`.

## 6) Build and Run

- Build core (also builds the generator): `./scripts/run_core.sh build`
- Build modules: `./scripts/build_core_modules.sh`
- Run the core app (optional): `./scripts/run_core.sh`

Artifacts

- Built plugins are placed under `modules/build/modules` and copied to `logos-liblogos/build/modules`. The logos-liblogos/FFI loaders look in `logos-liblogos/build/modules` by default.

## 7) Expose Methods and Events (using wrappers)

- Methods: mark as `Q_INVOKABLE` in your interface and implement them in the plugin. They become available to other modules and to external clients (via `liblogos_core` FFI) and as generated wrapper methods.
- Events: in `initLogos`, call `logos->your_module.setEventSource(this)`, then emit with `logos->your_module.trigger("eventName", args...)` or `trigger(eventName, QVariantList)`.

## 8) Calling Other Modules (using wrappers)

Use the generated wrappers. The SDK handles connections, tokens, and forwarding via `ModuleProxy` under the hood.

```cpp
// Subscribe/publish via Waku (declare "waku_module" in metadata.dependencies)
bool subOk = logos->waku_module.relaySubscribe("/waku/2/rs/16/32");
bool pubOk = logos->waku_module.relayPublish("/waku/2/rs/16/32", jsonWakuMessage);

// Listen to another module's events
logos->chat.on("chatMessage", [this](const QVariantList& data) {
    qDebug() << "chat event" << data;
});
```

## Troubleshooting

- Module not discovered: ensure your library ends up in `logos-liblogos/build/modules` and `metadata.json` is valid.
- Methods not callable: check `Q_INVOKABLE` on interface methods and that the plugin implements `Q_INTERFACES(YourModuleInterface PluginInterface)`.
- No events received: verify you call `onEventResponse(...)` and that clients are processing Qt events.
