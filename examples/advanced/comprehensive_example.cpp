#include "../../include/icelander.hpp"
#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <vector>
#include <atomic>
#include <random>

using namespace icelander;
using namespace icelander::async;

class NetworkingDemo {
private:
    std::atomic<bool> running_{false};
    std::shared_ptr<Host> server_host_;
    std::vector<std::shared_ptr<Host>> client_hosts_;
    std::vector<std::shared_ptr<Peer>> client_peers_;
    
    std::random_device rd_;
    std::mt19937 gen_;
    std::uniform_int_distribution<> delay_dist_;

public:
    NetworkingDemo() : gen_(rd_()), delay_dist_(100, 500) {}

    void runDemo() {
        std::cout << "=== Comprehensive Networking Example ===\n";
        std::cout << "This example demonstrates:\n";
        std::cout << "- Multi-client server architecture\n";
        std::cout << "- Event handling and broadcasting\n";
        std::cout << "- Packet serialization and compression\n";
        std::cout << "- Connection management\n";
        std::cout << "- Async task scheduling\n\n";

        if (!Library::initialize()) {
            std::cerr << "Failed to initialize Icelander library\n";
            return;
        }

        try {
            setupServer();
            setupClients();
            runCommunicationDemo();
        } catch (const std::exception& e) {
            std::cerr << "Demo error: " << e.what() << "\n";
        }

        cleanup();
        Library::deinitialize();
        std::cout << "\nDemo completed.\n";
    }

private:
    void setupServer() {
        std::cout << "Setting up server...\n";
        
        Endpoint bind_addr{"localhost", 12347};
        HostConfig server_config{
            .maxPeers = 10,
            .maxChannels = 3,
            .enableCompression = true
        };

        server_host_ = Host::createServer(bind_addr, server_config);
        
        server_host_->getDispatcher().onConnect([this](const ConnectEvent& event) {
            std::cout << "[SERVER] Client connected from " 
                      << event.remoteEndpoint.host << ":" << event.remoteEndpoint.port
                      << " (Total peers: " << server_host_->peerCount() << ")\n";
            
            // Send welcome message with client ID
            PacketBuilder welcome_builder;
            welcome_builder.writeString("WELCOME");
            welcome_builder.writeUint32(static_cast<uint32_t>(server_host_->peerCount()));
            auto welcome_packet = welcome_builder.build(static_cast<PacketFlagsT>(PacketFlag::reliable));
            event.peerHandle->send(std::move(welcome_packet));
            
            // Broadcast new client notification to all other clients
            broadcastMessage("NEW_CLIENT_JOINED", event.peerHandle);
        });

        server_host_->getDispatcher().onReceive([this](const ReceiveEvent& event) {
            PacketReader reader(*event.packetData);
            
            try {
                auto msg_type = reader.readString(reader.readUint32());
                
                if (msg_type == "CHAT") {
                    auto client_id = reader.readUint32();
                    auto message = reader.readString(reader.readUint32());
                    
                    std::cout << "[SERVER] Chat from client " << client_id << ": " << message << "\n";
                    
                    // Broadcast chat message to all clients
                    PacketBuilder broadcast_builder;
                    broadcast_builder.writeString("BROADCAST_CHAT");
                    broadcast_builder.writeUint32(client_id);
                    broadcast_builder.writeString(message);
                    
                    auto broadcast_packet = broadcast_builder.build(static_cast<PacketFlagsT>(PacketFlag::reliable));
                    server_host_->broadcast(std::move(broadcast_packet));
                    
                } else if (msg_type == "PING") {
                    auto timestamp = reader.readUint64();
                    
                    // Send pong response
                    PacketBuilder pong_builder;
                    pong_builder.writeString("PONG");
                    pong_builder.writeUint64(timestamp);
                    pong_builder.writeUint64(std::chrono::steady_clock::now().time_since_epoch().count());
                    
                    auto pong_packet = pong_builder.build(static_cast<PacketFlagsT>(PacketFlag::reliable));
                    event.peerHandle->send(std::move(pong_packet));
                }
                
            } catch (const std::exception& e) {
                std::cerr << "[SERVER] Error processing packet: " << e.what() << "\n";
            }
        });

        server_host_->getDispatcher().onDisconnect([this](const DisconnectEvent& event) {
            std::cout << "[SERVER] Client disconnected from " 
                      << event.remoteEndpoint.host << ":" << event.remoteEndpoint.port
                      << " (Remaining peers: " << server_host_->peerCount() << ")\n";
            
            broadcastMessage("CLIENT_DISCONNECTED", nullptr);
        });

        server_host_->startServiceThread();
        std::cout << "Server started on " << bind_addr.host << ":" << bind_addr.port << "\n";
    }

    void setupClients() {
        std::cout << "Setting up clients...\n";
        
        const int num_clients = 3;
        client_hosts_.reserve(num_clients);
        client_peers_.reserve(num_clients);
        
        for (int i = 0; i < num_clients; ++i) {
            HostConfig client_config{
                .maxPeers = 1,
                .maxChannels = 3
            };
            
            auto client_host = Host::createClient(client_config);
            const int client_id = i + 1;
            
            client_host->getDispatcher().onConnect([client_id](const ConnectEvent& event) {
                std::cout << "[CLIENT " << client_id << "] Connected to server\n";
            });
            
            client_host->getDispatcher().onReceive([client_id](const ReceiveEvent& event) {
                PacketReader reader(*event.packetData);
                
                try {
                    auto msg_type = reader.readString(reader.readUint32());
                    
                    if (msg_type == "WELCOME") {
                        auto assigned_id = reader.readUint32();
                        std::cout << "[CLIENT " << client_id << "] Received welcome, assigned ID: " << assigned_id << "\n";
                        
                    } else if (msg_type == "BROADCAST_CHAT") {
                        auto sender_id = reader.readUint32();
                        auto message = reader.readString(reader.readUint32());
                        if (sender_id != client_id) {  // Don't show own messages
                            std::cout << "[CLIENT " << client_id << "] Chat from client " << sender_id << ": " << message << "\n";
                        }
                        
                    } else if (msg_type == "PONG") {
                        auto sent_time = reader.readUint64();
                        auto server_time = reader.readUint64();
                        auto now = std::chrono::steady_clock::now().time_since_epoch().count();
                        
                        auto rtt = now - sent_time;
                        std::cout << "[CLIENT " << client_id << "] Ping RTT: " 
                                  << (rtt / 1000000.0) << " ms\n";
                                  
                    } else if (msg_type == "NEW_CLIENT_JOINED") {
                        std::cout << "[CLIENT " << client_id << "] New client joined the server\n";
                        
                    } else if (msg_type == "CLIENT_DISCONNECTED") {
                        std::cout << "[CLIENT " << client_id << "] A client disconnected from the server\n";
                    }
                    
                } catch (const std::exception& e) {
                    std::cerr << "[CLIENT " << client_id << "] Error processing packet: " << e.what() << "\n";
                }
            });
            
            client_host->getDispatcher().onDisconnect([client_id](const DisconnectEvent& event) {
                std::cout << "[CLIENT " << client_id << "] Disconnected from server\n";
            });
            
            client_host->startServiceThread();
            
            Endpoint server_addr{"localhost", 12347};
            auto server_peer = client_host->connect(server_addr);
            
            client_hosts_.push_back(client_host);
            client_peers_.push_back(server_peer);
            
            // Small delay between client connections
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
        }
        
        std::cout << "All clients connected\n";
    }

    void runCommunicationDemo() {
        std::cout << "\nStarting communication demo...\n";
        
        TaskScheduler::instance().start();
        running_ = true;
        
        // Schedule periodic activities
        schedulePeriodicPing();
        scheduleChatMessages();
        
        // Let the demo run for a while
        std::this_thread::sleep_for(std::chrono::seconds(15));
        
        running_ = false;
        TaskScheduler::instance().stop();
        
        std::cout << "Communication demo finished\n";
    }

    void schedulePeriodicPing() {
        TaskScheduler::instance().schedule([this]() {
            while (running_) {
                std::this_thread::sleep_for(std::chrono::seconds(3));
                
                if (!running_) break;
                
                // Send ping from random client
                if (!client_peers_.empty()) {
                    int client_idx = gen_() % client_peers_.size();
                    auto& peer = client_peers_[client_idx];
                    
                    if (peer && peer->isConnected()) {
                        PacketBuilder ping_builder;
                        ping_builder.writeString("PING");
                        ping_builder.writeUint64(std::chrono::steady_clock::now().time_since_epoch().count());
                        
                        auto ping_packet = ping_builder.build(static_cast<PacketFlagsT>(PacketFlag::reliable));
                        peer->send(std::move(ping_packet));
                    }
                }
            }
        });
    }

    void scheduleChatMessages() {
        TaskScheduler::instance().schedule([this]() {
            std::vector<std::string> sample_messages = {
                "Hello everyone!",
                "How's everyone doing?",
                "This is a test message",
                "Icelander is working great!",
                "Broadcasting to all clients",
                "Multi-client demo is successful"
            };
            
            int message_count = 0;
            while (running_ && message_count < 12) {
                std::this_thread::sleep_for(std::chrono::milliseconds(delay_dist_(gen_)));
                
                if (!running_) break;
                
                // Send chat from random client
                if (!client_peers_.empty()) {
                    int client_idx = gen_() % client_peers_.size();
                    auto& peer = client_peers_[client_idx];
                    
                    if (peer && peer->isConnected()) {
                        int msg_idx = gen_() % sample_messages.size();
                        auto& message = sample_messages[msg_idx];
                        
                        PacketBuilder chat_builder;
                        chat_builder.writeString("CHAT");
                        chat_builder.writeUint32(client_idx + 1);  // Client ID
                        chat_builder.writeString(message);
                        
                        auto chat_packet = chat_builder.build(static_cast<PacketFlagsT>(PacketFlag::reliable));
                        peer->send(std::move(chat_packet));
                        
                        message_count++;
                    }
                }
            }
        });
    }

    void broadcastMessage(const std::string& msg_type, std::shared_ptr<Peer> exclude_peer) {
        if (!server_host_) return;
        
        PacketBuilder builder;
        builder.writeString(msg_type);
        
        auto packet = builder.build(static_cast<PacketFlagsT>(PacketFlag::reliable));
        
        // Broadcast to all connected peers except the excluded one
        auto peers = server_host_->getPeers();
        for (const auto& peer : peers) {
            if (peer && peer != exclude_peer && peer->isConnected()) {
                // Create a copy of the packet for each peer
                PacketBuilder copy_builder;
                copy_builder.writeString(msg_type);
                auto copy_packet = copy_builder.build(static_cast<PacketFlagsT>(PacketFlag::reliable));
                peer->send(std::move(copy_packet));
            }
        }
    }

    void cleanup() {
        std::cout << "\nCleaning up...\n";
        
        // Disconnect all clients gracefully
        for (auto& peer : client_peers_) {
            if (peer && peer->isConnected()) {
                peer->disconnect();
            }
        }
        
        // Allow time for graceful disconnection
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        
        // Stop client service threads
        for (auto& host : client_hosts_) {
            if (host) {
                host->stopServiceThread();
            }
        }
        
        // Stop server service thread
        if (server_host_) {
            server_host_->stopServiceThread();
        }
        
        client_peers_.clear();
        client_hosts_.clear();
        server_host_.reset();
    }
};

int main() {
    NetworkingDemo demo;
    demo.runDemo();
    return 0;
}