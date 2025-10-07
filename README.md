# logos-core-poc

note: For a stable version use the master branch [https://github.com/logos-co/logos-core-poc/tree/master](https://github.com/logos-co/logos-core-poc/tree/master)

## Setup

```bash
git submodule update --init --recursive
```

Build & run all:

```bash
./scripts/run_app.sh all
```

For core & modules:

```bash
./scripts/run_core.sh all
```

Build Core only:

```bash
./scripts/run_core.sh build
```

Build Container:

```bash
docker build -t logos-core .
```

Run Container:

```bash
docker run -it logos-core
```

## Git Submodules

This repository includes the following git submodules:

| Directory | Repository | Description |
|-----------|------------|-------------|
| `logos-liblogos/` | [logos-co/logos-liblogos](https://github.com/logos-co/logos-liblogos) | Core library implementation |
| `logos-cpp-sdk/` | [logos-co/logos-cpp-sdk](https://github.com/logos-co/logos-cpp-sdk) | C++ SDK for building modules |
| `logos-js-sdk/` | [logos-co/logos-js-sdk](https://github.com/logos-co/logos-js-sdk) | JavaScript/Node.js SDK |
| `logos-nim-sdk/` | [logos-co/logos-nim-sdk](https://github.com/logos-co/logos-nim-sdk) | Nim language SDK |
| `modules/simple_module/` | [logos-co/logos-simple-module](https://github.com/logos-co/logos-simple-module) | Example module template |
| `modules/wallet_module/` | [logos-co/logos-wallet-module](https://github.com/logos-co/logos-wallet-module) | Wallet module with go-wallet-sdk integration |
| `modules/waku_module/` | [logos-co/logos-waku-module](https://github.com/logos-co/logos-waku-module) | Waku network protocol module |
| `modules/chat/` | [logos-co/logos-chat-module](https://github.com/logos-co/logos-chat-module) | Chat module with Waku integration |
| `modules/logos_irc/` | [logos-co/logos-irc-module](https://github.com/logos-co/logos-irc-module) | IRC protocol module |
| `website/` | [logos-co/logos-website](https://github.com/logos-co/logos-website) | Documentation website |

## Requirements

- QT 6.4

  Ubuntu
  ```
  apt-get install qt6-base-dev protobuf-compiler patchelf
  ```
- CMake

For some plugins
- Rust
- protobuf

## Structure

```
📦 logos-core-poc
 ┣ 📂 logos-liblogos/                    # Logos Core Library
 ┃
 ┣ 📂 logos_app/               # Application Layer
 ┃ ┣ 📂 app/                   # Logos App POC
 ┃ ┣ 📂 logos_dapps/           # Applications for the Logos App
 ┃   ┗ 📂 chat_ui/             # Simple Chat App
 ┃
 ┣ 📂 modules/                 # Modules for Logos Core
 ┃ ┗ 📂 chat/                  # POC Chat API, interacts with Waku Module
 ┃ ┗ 📂 package_manager/       # Package Manager Module
 ┃ ┗ 📂 capability_module/     # Coordinates permissions between modules
 ┃ ┗ 📂 template_module/       # Example Module
 ┃ ┗ 📂 waku/                  # Waku Module
 ┃
 ┣ 📂 scripts/                 # Scripts
 ┣ 📄 scripts/run_app.sh               # Script to build and run the application
 ┣ 📄 scripts/run_core.sh              # Script to build and run the core
```

## Documentation

Specs at docs/specs.md
