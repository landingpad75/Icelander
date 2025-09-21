#include "icelander.hpp"
#include <iostream>
#include <string>
#include <thread>

using namespace icelander;

void test_packet_operations() {
    std::cout << "=== Testing Packet Operations ===\n";
    
    // Test packet creation with string
    auto pkt1 = Packet::create("Hello, World!");
    std::cout << "Created packet with size: " << pkt1->size() << " bytes\n";
    std::cout << "Packet content: " << pkt1->asString() << "\n";
    
    // Test packet builder
    PacketBuilder builder;
    builder.writeUint32(0x12345678)
           .writeString("Test Message")
           .writeUint16(42)
           .writeUint8(255);
    
    auto pkt2 = builder.build();
    std::cout << "Built packet with size: " << pkt2->size() << " bytes\n";
    
    // Test packet reader
    PacketReader reader(*pkt2);
    auto magic = reader.readUint32();
    auto str_len = reader.readUint32();
    auto message = reader.readString(str_len);
    auto number = reader.readUint16();
    auto byte_val = reader.readUint8();
    
    std::cout << "Read magic: 0x" << std::hex << magic << std::dec << "\n";
    std::cout << "Read message: " << message << "\n";
    std::cout << "Read number: " << number << "\n";
    std::cout << "Read byte: " << static_cast<int>(byte_val) << "\n";
}

void test_library_functions() {
    std::cout << "=== Testing Library Functions ===\n";
    
    std::cout << "ENet version: 0x" << std::hex << Library::version() << std::dec << "\n";
    
    if (Library::initialize()) {
        std::cout << "Library initialized successfully\n";
        std::cout << "Library is initialized: " << (Library::isInitialized() ? "Yes" : "No") << "\n";
        Library::deinitialize();
        std::cout << "Library deinitialized\n";
    } else {
        std::cout << "Failed to initialize library\n";
    }
}

void test_endpoint_operations() {
    std::cout << "=== Testing Endpoint Operations ===\n";
    
    Endpoint ep1{"localhost", 8080};
    std::cout << "Created endpoint: " << ep1.host << ":" << ep1.port << "\n";
    
    Endpoint ep2{"127.0.0.1", 12345};
    std::cout << "Created endpoint: " << ep2.host << ":" << ep2.port << "\n";
}

void test_async_scheduler() {
    std::cout << "=== Testing Async Scheduler ===\n";
    
    auto& scheduler = async::TaskScheduler::instance();
    
    bool task_executed = false;
    scheduler.schedule([&task_executed]() {
        std::cout << "Task executed!\n";
        task_executed = true;
    });
    
    scheduler.start();
    
    // Wait a bit for task to execute
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    scheduler.stop();
    
    std::cout << "Task was " << (task_executed ? "" : "not ") << "executed\n";
}

int main() {
    std::cout << "Icelander Library Test Suite\n";
    std::cout << "============================\n\n";
    
    try {
        test_library_functions();
        std::cout << "\n";
        
        test_endpoint_operations();
        std::cout << "\n";
        
        test_packet_operations();
        std::cout << "\n";
        
        test_async_scheduler();
        std::cout << "\n";
        
        std::cout << "All tests completed successfully!\n";
        
    } catch (const std::exception& e) {
        std::cerr << "Test failed with exception: " << e.what() << "\n";
        return 1;
    }
    
    return 0;
}