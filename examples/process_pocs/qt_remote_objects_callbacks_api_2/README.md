# Qt Remote Objects Example

## Components

1. **Counter Interface (counter.rep)**: Defines the interface for the remote object using Qt's .rep file format.

2. **Server (server.cpp)**: Implements the counter object and hosts it along with a registry.

3. **Client (client.cpp)**: Provides a GUI to interact with the remote counter object.

## Qt Remote Objects Features Demonstrated

1. **Remote Object Registry**: The server hosts a registry to allow clients to discover available remote objects.

2. **Remote Object Replica**: The client acquires a replica of the counter object from the registry.

3. **Properties, Signals, and Slots**: The example shows how properties, signals, and slots are exposed and used across process boundaries.

4. **State Management**: The client monitors the connection state to the remote object.

## Building and Running

```bash
./compile.sh
```

then on one terminal

```bash
./build/counter_server
```

then on another

```bash
./build/counter_client
```

## Architecture

```
┌────────────────┐                  ┌────────────────┐
│    Server      │                  │     Client     │
│  ┌──────────┐  │                  │  ┌──────────┐  │
│  │ Registry │◄─┼──────────────────┼──┤ Registry │  │
│  └──────────┘  │                  │  │  Node    │  │
│       ▲        │                  │  └──────────┘  │
│       │        │                  │       │        │
│  ┌──────────┐  │                  │       ▼        │
│  │ Counter  │  │                  │  ┌──────────┐  │
│  │ Source   │◄─┼──────────────────┼──┤ Counter  │  │
│  └──────────┘  │                  │  │ Replica  │  │
└────────────────┘                  │  └──────────┘  │
                                    │       ▲        │
                                    │       │        │
                                    │  ┌──────────┐  │
                                    │  │   GUI    │  │
                                    │  └──────────┘  │
                                    └────────────────┘
``` 
