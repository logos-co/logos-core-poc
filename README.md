# logos-core-poc

Setup:

```bash
git submodule update --init --recursive
```

Build & run all:

```bash
./run_app.sh all
```

For core & plugins:

```bash
./run_core.sh all
```

Build Core only:

```bash
./run_core.sh build
```

## Requirements

- QT 6.4
- CMake

For some plugins
- Rust
- protobuf
