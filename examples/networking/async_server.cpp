#include "../../include/icelander.hpp"
#include <iostream>
#include <string>
#include <thread>
#include <chrono>

using namespace icelander;
using namespace icelander::async;

int main() {
    std::cout << "=== Async Server Example ===\n";
    
    if (!Library::initialize()) {
        std::cerr << "Failed to initialize Icelander\n";
        return 1;
    }

    try {
        Endpoint bind_addr{"localhost", 12345};
        HostConfig config{
            .maxPeers = 32,
            .maxChannels = 2,
            .enableCompression = true
        };

        auto server_host = Host::createServer(bind_addr, config);
        std::cout << "Server started on " << bind_addr.host << ":" << bind_addr.port << "\n";

        server_host->getDispatcher().onConnect([](const ConnectEvent& event) {
            std::cout << "Client connected from " << event.remoteEndpoint.host
                      << ":" << event.remoteEndpoint.port << "\n";
            event.peerHandle->send(Packet::create("Welcome to the server!", 
                static_cast<PacketFlagsT>(PacketFlag::reliable)));
        });

        server_host->getDispatcher().onReceive([](const ReceiveEvent& event) {
            PacketReader reader(*event.packetData);
            auto message = reader.asString();
            std::cout << "Received: " << message << "\n";

            if (message == "quit") {
                event.peerHandle->disconnect();
                return;
            }

            auto response = "Echo: " + std::string(message);
            event.peerHandle->send(Packet::create(response, 
                static_cast<PacketFlagsT>(PacketFlag::reliable)));
        });

        server_host->getDispatcher().onDisconnect([](const DisconnectEvent& event) {
            std::cout << "Client disconnected from " << event.remoteEndpoint.host
                      << ":" << event.remoteEndpoint.port << "\n";
        });

        TaskScheduler::instance().start();
        server_host->startServiceThread();

        std::cout << "Server running. Press Enter to stop...\n";
        std::cin.get();

        server_host->stopServiceThread();
        TaskScheduler::instance().stop();

    } catch (const std::exception& e) {
        std::cerr << "Server error: " << e.what() << "\n";
        Library::deinitialize();
        return 1;
    }

    Library::deinitialize();
    std::cout << "Server stopped.\n";
    return 0;
}