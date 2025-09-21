#include "../../include/icelander.hpp"
#include <iostream>
#include <string>
#include <thread>
#include <chrono>

using namespace icelander;

int main() {
    std::cout << "=== Simple Sync Example ===\n";

    if (!Library::initialize()) {
        std::cerr << "Failed to initialize Icelander\n";
        return 1;
    }

    try {
        Endpoint bind_addr{"localhost", 12346};
        auto server_host = Host::createServer(bind_addr);
        auto client_host = Host::createClient();

        std::cout << "Server created on port 12346\n";

        server_host->getDispatcher().onConnect([](const ConnectEvent& event) {
            std::cout << "Server: Client connected\n";
            event.peerHandle->send(Packet::create("Hello from server!", 
                static_cast<PacketFlagsT>(PacketFlag::reliable)));
        });

        server_host->getDispatcher().onReceive([](const ReceiveEvent& event) {
            PacketReader reader(*event.packetData);
            std::cout << "Server received: " << reader.asString() << "\n";
        });

        client_host->getDispatcher().onConnect([](const ConnectEvent& event) {
            std::cout << "Client: Connected to server\n";
        });

        client_host->getDispatcher().onReceive([](const ReceiveEvent& event) {
            PacketReader reader(*event.packetData);
            std::cout << "Client received: " << reader.asString() << "\n";

            event.peerHandle->send(Packet::create("Hello from client!", 
                static_cast<PacketFlagsT>(PacketFlag::reliable)));
        });

        std::cout << "Starting synchronous communication...\n";
        auto server_peer = client_host->connect(bind_addr);

        // Service both hosts synchronously
        for (int i = 0; i < 10; ++i) {
            server_host->service(std::chrono::milliseconds(100));
            client_host->service(std::chrono::milliseconds(100));
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }

        std::cout << "Disconnecting...\n";
        server_peer->disconnect();

        // Allow time for graceful disconnect
        for (int i = 0; i < 5; ++i) {
            server_host->service(std::chrono::milliseconds(100));
            client_host->service(std::chrono::milliseconds(100));
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }

        std::cout << "Sync example completed successfully!\n";

    } catch (const std::exception& e) {
        std::cerr << "Sync example error: " << e.what() << "\n";
        Library::deinitialize();
        return 1;
    }

    Library::deinitialize();
    return 0;
}