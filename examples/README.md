# Icelander Examples

This directory contains a comprehensive collection of examples and tests demonstrating the usage of the Icelander networking library. The examples are organized by complexity and functionality to provide a clear learning path.

## Directory Structure

```
examples/
├── basic/                          # Simple, focused examples
│   ├── packet_builder_example.cpp  # Packet construction and parsing
│   └── sync_example.cpp           # Basic synchronous networking
├── networking/                     # Network communication examples
│   ├── async_server.cpp           # Asynchronous server implementation
│   └── async_client.cpp           # Asynchronous client implementation
├── advanced/                       # Complex, feature-rich examples
│   ├── comprehensive_example.cpp   # Multi-client server with broadcasting
│   └── performance_benchmark.cpp   # Throughput and latency testing
└── tests/                         # Unit tests and validation
    └── unit_tests.cpp             # Library functionality tests
```

## Basic Examples (`basic/`)

### PacketBuilderExample
**File:** `packet_builder_example.cpp`

Demonstrates fundamental packet operations:
- Creating packets with `PacketBuilder`
- Writing different data types (uint32, string, uint8)
- Reading packets with `PacketReader`
- Binary data serialization

**Usage:**
```bash
./PacketBuilderExample
```

### SyncExample
**File:** `sync_example.cpp`

Shows basic synchronous networking:
- Server and client creation
- Connection establishment
- Event-driven communication
- Graceful connection management
- Synchronous service loop

**Usage:**
```bash
./SyncExample
```

## Networking Examples (`networking/`)

### AsyncServer
**File:** `async_server.cpp`

A complete asynchronous server implementation:
- Multi-client support (up to 32 clients)
- Event-driven architecture
- Connection/disconnection handling
- Message echoing
- Graceful shutdown
- Service threading

**Usage:**
```bash
./AsyncServer
# Server will listen on localhost:12345
# Press Enter to stop the server
```

### AsyncClient
**File:** `async_client.cpp`

Interactive asynchronous client:
- Connects to AsyncServer
- Interactive message sending
- Real-time server response display
- Clean disconnection
- Service threading

**Usage:**
```bash
# First, start AsyncServer in another terminal
./AsyncClient
# Type messages and press Enter
# Type 'quit' to disconnect
```

## Advanced Examples (`advanced/`)

### ComprehensiveExample
**File:** `comprehensive_example.cpp`

A sophisticated multi-client networking demonstration:
- **Multi-client Architecture**: Supports multiple simultaneous clients
- **Broadcasting**: Messages sent to all connected clients
- **Event System**: Connect/disconnect notifications
- **Packet Types**: Different message types (CHAT, PING, WELCOME, etc.)
- **Task Scheduling**: Automated periodic activities
- **Connection Management**: Graceful handling of client lifecycle

**Features Demonstrated:**
- Server with up to 10 clients
- Real-time chat broadcasting
- Ping/pong latency measurement
- Client join/leave notifications
- Automated message generation
- Comprehensive logging

**Usage:**
```bash
./ComprehensiveExample
# Runs a 15-second automated demo
# Creates server + 3 clients automatically
# Shows real-time multi-client communication
```

### PerformanceBenchmark
**File:** `performance_benchmark.cpp`

Professional networking performance testing suite:
- **Throughput Testing**: Measures packets/second and bandwidth
- **Latency Testing**: RTT measurements with percentile analysis
- **Burst Testing**: High-frequency packet transmission
- **Detailed Statistics**: Min/avg/max/percentile reporting
- **Memory Efficiency**: Optimized for performance measurement

**Metrics Collected:**
- Packets per second
- Bandwidth (KB/sec, Mbps)
- Round-trip time (min/avg/max/P50/P95/P99)
- Packet loss percentage
- Burst transmission rates

**Usage:**
```bash
./PerformanceBenchmark
# Runs automated performance tests
# Outputs detailed performance metrics
# Tests throughput, latency, and burst performance
```

## Tests (`tests/`)

### UnitTests
**File:** `unit_tests.cpp`

Comprehensive unit testing for library components:
- **Library Functions**: Initialization, versioning, cleanup
- **Packet Operations**: Creation, building, reading, serialization
- **Endpoint Operations**: Network address handling
- **Async Scheduler**: Task scheduling system validation

**Usage:**
```bash
./UnitTests
# Runs all unit tests
# Reports success/failure for each test category
```

## Building Examples

### Build All Examples
```bash
# From build directory
cmake --build . --target examples
```

### Build by Category
```bash
# Basic examples only
cmake --build . --target basic_examples

# Networking examples only
cmake --build . --target networking_examples

# Advanced examples only
cmake --build . --target advanced_examples

# Tests only
cmake --build . --target tests
```

### Build Individual Examples
```bash
cmake --build . --target PacketBuilderExample
cmake --build . --target AsyncServer
cmake --build . --target ComprehensiveExample
cmake --build . --target UnitTests
# ... etc
```

## Running Examples

### Executable Locations
After building, executables are located in:
```
build/examples/basic/Debug/         # PacketBuilderExample, SyncExample
build/examples/networking/Debug/    # AsyncServer, AsyncClient
build/examples/advanced/Debug/      # ComprehensiveExample, PerformanceBenchmark
build/examples/tests/Debug/         # UnitTests
```

### Server-Client Examples
For networking examples that require server-client interaction:

1. **Start the server:**
   ```bash
   cd build/examples/networking/Debug
   ./AsyncServer
   ```

2. **In another terminal, start the client:**
   ```bash
   cd build/examples/networking/Debug
   ./AsyncClient
   ```

## Example Output

### PacketBuilderExample
```
=== Packet Builder Example ===
Magic: 0x12345678
Message: Hello, World!
Number: 314159
Max byte: 255
Packet size: 26 bytes
```

### UnitTests
```
Icelander Library Test Suite
============================

=== Testing Library Functions ===
ENet version: 0x10304
Library initialized successfully
Library is initialized: Yes
Library deinitialized

=== Testing Endpoint Operations ===
Created endpoint: localhost:8080
Created endpoint: 127.0.0.1:12345

=== Testing Packet Operations ===
Created packet with size: 14 bytes
Packet content: Hello, World!
Built packet with size: 23 bytes
Read magic: 0x12345678
Read message: Test Message
Read number: 42
Read byte: 255

=== Testing Async Scheduler ===
Task executed!
Task was executed

All tests completed successfully!
```

## Learning Path

1. **Start with Basic Examples:**
   - Run `PacketBuilderExample` to understand packet manipulation
   - Run `SyncExample` to see basic networking concepts

2. **Move to Networking Examples:**
   - Try `AsyncServer` and `AsyncClient` for client-server architecture
   - Understand asynchronous event handling

3. **Explore Advanced Examples:**
   - Run `ComprehensiveExample` for multi-client scenarios
   - Use `PerformanceBenchmark` to understand performance characteristics

4. **Validate with Tests:**
   - Run `UnitTests` to see all components working together

## Requirements

- **C++20** compatible compiler
- **ENet library** (included as submodule)
- **CMake 3.20** or higher
- **Multi-threading support** for async examples

## Tips

- **Firewall**: Ensure localhost connections are allowed
- **Port Usage**: Examples use ports 12345-12348
- **Concurrent Testing**: Multiple examples can run simultaneously on different ports
- **Logging**: All examples include comprehensive logging for debugging
- **Error Handling**: Examples demonstrate proper exception handling and resource cleanup

## Troubleshooting

### Connection Issues
- Ensure server is started before client
- Check if ports are already in use
- Verify firewall settings allow localhost connections

### Build Issues
- Ensure all dependencies are properly linked
- Check that C++20 is supported by your compiler
- Verify ENet submodule is properly initialized

### Runtime Issues
- Check library initialization in error messages
- Ensure proper cleanup order (clients before servers)
- Monitor console output for detailed error information