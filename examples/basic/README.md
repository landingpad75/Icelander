# Basic Examples

This directory contains simple, focused examples that demonstrate fundamental Icelander library concepts. These examples are designed to be your first introduction to the library's core functionality.

## Examples

### 1. PacketBuilderExample (`packet_builder_example.cpp`)

**Purpose:** Demonstrates packet construction, serialization, and parsing

**What it shows:**
- Creating packets with `PacketBuilder`
- Writing different data types (integers, strings, bytes)
- Reading packets with `PacketReader`
- Binary data serialization patterns

**Key concepts:**
- Packet creation and management
- Data type serialization
- Memory-efficient packet handling
- Type-safe data reading/writing

**Usage:**
```bash
./PacketBuilderExample
```

**Expected output:**
```
=== Packet Builder Example ===
Magic: 0x12345678
Message: Hello, World!
Number: 314159
Max byte: 255
Packet size: 26 bytes
```

---

### 2. SyncExample (`sync_example.cpp`)

**Purpose:** Shows basic synchronous networking with server-client communication

**What it shows:**
- Creating server and client hosts
- Establishing connections
- Event-driven communication patterns
- Synchronous service loops
- Graceful connection management

**Key concepts:**
- Host creation (server vs client)
- Event dispatchers and callbacks
- Connection lifecycle management
- Synchronous vs asynchronous operation modes
- Proper cleanup procedures

**Usage:**
```bash
./SyncExample
```

**Expected output:**
```
=== Simple Sync Example ===
Server created on port 12346
Starting synchronous communication...
Client: Connected to server
Server: Client connected
Client received: Hello from server!
Server received: Hello from client!
Disconnecting...
Sync example completed successfully!
```

## Building

### Build both basic examples:
```bash
cmake --build . --target basic_examples
```

### Build individual examples:
```bash
cmake --build . --target PacketBuilderExample
cmake --build . --target SyncExample
```

## Learning Path

1. **Start with PacketBuilderExample** to understand how data is serialized and transmitted
2. **Move to SyncExample** to see how networking connections work
3. **Understand the patterns** before moving to more complex networking examples

## Code Patterns Demonstrated

### Packet Building Pattern
```cpp
PacketBuilder builder;
builder.writeUint32(magic_number)
       .writeString(message)
       .writeUint8(flags);
auto packet = builder.build(PacketFlag::reliable);
```

### Packet Reading Pattern
```cpp
PacketReader reader(packet);
auto magic = reader.readUint32();
auto str_len = reader.readUint32();
auto message = reader.readString(str_len);
auto flags = reader.readUint8();
```

### Basic Server Setup Pattern
```cpp
auto server = Host::createServer(endpoint, config);
server->getDispatcher().onConnect([](const ConnectEvent& event) {
    // Handle new client connections
});
```

### Basic Client Setup Pattern
```cpp
auto client = Host::createClient(config);
auto peer = client->connect(server_endpoint);
// Use synchronous service calls
client->service(timeout);
```

## Next Steps

After understanding these basic examples:

1. **Move to networking examples** (`../networking/`) for asynchronous patterns
2. **Explore advanced examples** (`../advanced/`) for complex scenarios
3. **Run tests** (`../tests/`) to validate your understanding

## Common Issues

### Build Issues
- Ensure C++20 compiler support
- Verify all dependencies are linked properly
- Check that include paths are correct

### Runtime Issues
- **Library initialization**: Always call `Library::initialize()` before use
- **Cleanup order**: Disconnect peers before destroying hosts
- **Port conflicts**: Ensure ports aren't already in use

### Understanding the Code
- **Event-driven patterns**: Callbacks are executed when network events occur
- **Resource management**: Use RAII patterns for automatic cleanup
- **Error handling**: Always check return values and catch exceptions

## Tips

- **Start simple**: Run PacketBuilderExample first to understand data flow
- **Read the code**: These examples are heavily commented for learning
- **Experiment**: Modify the examples to see how changes affect behavior
- **Debug output**: All examples include console output to show what's happening

These basic examples provide the foundation for understanding more complex Icelander networking scenarios. Master these patterns before moving to advanced examples.