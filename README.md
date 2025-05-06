# logos-core-poc

## Setup

```bash
git submodule update --init --recursive
```

Build & run all:

```bash
./run_app.sh all
```

For core & modules:

```bash
./run_core.sh all
```

Build Core only:

```bash
./run_core.sh build
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
 ┃ ┗ 📄 ...                    # Core components and utilities
 ┃
 ┣ 📂 logos_app/               # Application Layer
 ┃ ┣ 📂 app/                   # Logos App POC
 ┃ ┣ 📂 logos_dapps/           # Applications for the Logos App
 ┃   ┗ 📂 chat_ui/             # Simple Chat App
 ┃
 ┣ 📂 modules/                 # Modules for Logos Core
 ┃ ┗ 📂 waku/                  # Waku Module
 ┃
 ┣ 📄 run_app.sh               # Script to build and run the application
 ┣ 📄 run_core.sh              # Script to build and run the core
```
