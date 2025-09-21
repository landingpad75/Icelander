#include "../../include/icelander.hpp"
#include <iostream>
#include <string>
#include <thread>
#include <chrono>

using namespace icelander;
using namespace icelander::async;

int main() {
    std::cout << "=== Async Client Example ===\n";
    
    if (!Library::initialize()) {
        std::cerr << "Failed to initialize Icelander\n";
        return 1;
    }

    try {
        HostConfig config{
            .maxPeers = 1,
            .maxChannels = 2
        };

        auto client_host = Host::createClient(config);
        Endpoint server_addr{"localhost", 12345};

        client_host->getDispatcher().onReceive([](const ReceiveEvent& event) {
            PacketReader reader(*event.packetData);
            auto message = reader.asString();
            std::cout << "Server says: " << message << "\n";
        });

        client_host->getDispatcher().onDisconnect([](const DisconnectEvent& event) {
            std::cout << "Disconnected from server\n";
        });

        std::cout << "Connecting to server at " << server_addr.host << ":" << server_addr.port << "\n";
        auto server_peer = client_host->connect(server_addr);
        std::cout << "Connected to server\n";

        TaskScheduler::instance().start();
        client_host->startServiceThread();

        std::string input;
        std::cout << "Enter messages (type 'quit' to exit):\n";
        while (std::getline(std::cin, input)) {
            if (input.empty()) continue;

            PacketBuilder builder;
            builder.writeString(input);
            auto pkt = builder.build(static_cast<PacketFlagsT>(PacketFlag::reliable));

            if (!server_peer->send(std::move(pkt))) {
                std::cerr << "Failed to send message\n";
            }

            if (input == "quit") {
                break;
            }
        }

        server_peer->disconnect();
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        
        client_host->stopServiceThread();
        TaskScheduler::instance().stop();

    } catch (const std::exception& e) {
        std::cerr << "Client error: " << e.what() << "\n";
        Library::deinitialize();
        return 1;
    }

    Library::deinitialize();
    std::cout << "Client disconnected.\n";
    return 0;
}