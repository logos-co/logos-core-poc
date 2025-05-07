# logos-core-poc

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
 ┣ 📂 core/                    # Logos Core Library
 ┃
 ┣ 📂 logos_app/               # Application Layer
 ┃ ┣ 📂 app/                   # Logos App POC
 ┃ ┣ 📂 logos_dapps/           # Applications for the Logos App
 ┃   ┗ 📂 chat_ui/             # Simple Chat App
 ┃
 ┣ 📂 modules/                 # Modules for Logos Core
 ┃ ┗ 📂 waku/                  # Waku Module
 ┃
 ┣ 📂 scripts/                 # Scripts
 ┣ 📄 scripts/run_app.sh               # Script to build and run the application
 ┣ 📄 scripts/run_core.sh              # Script to build and run the core
```
