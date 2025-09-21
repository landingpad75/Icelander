#include "../../include/icelander.hpp"
#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <atomic>
#include <vector>
#include <numeric>
#include <algorithm>
#include <iomanip>

using namespace icelander;
using namespace icelander::async;

class PerformanceBenchmark {
private:
    std::shared_ptr<Host> server_host_;
    std::shared_ptr<Host> client_host_;
    std::shared_ptr<Peer> server_peer_;
    
    std::atomic<bool> benchmark_running_{false};
    std::atomic<uint64_t> packets_sent_{0};
    std::atomic<uint64_t> packets_received_{0};
    std::atomic<uint64_t> bytes_sent_{0};
    std::atomic<uint64_t> bytes_received_{0};
    
    std::vector<double> latency_measurements_;
    std::mutex latency_mutex_;
    
    std::chrono::steady_clock::time_point benchmark_start_;

public:
    void runBenchmark() {
        std::cout << "=== Icelander Performance Benchmark ===\n";
        std::cout << "This benchmark tests:\n";
        std::cout << "- Throughput (packets/second, bytes/second)\n";
        std::cout << "- Latency (round-trip time)\n";
        std::cout << "- Memory efficiency\n";
        std::cout << "- Connection stability\n\n";

        if (!Library::initialize()) {
            std::cerr << "Failed to initialize Icelander library\n";
            return;
        }

        try {
            setupNetworking();
            
            // Run different benchmark tests
            runThroughputTest();
            runLatencyTest();
            runBurstTest();
            
        } catch (const std::exception& e) {
            std::cerr << "Benchmark error: " << e.what() << "\n";
        }

        cleanup();
        Library::deinitialize();
        std::cout << "\nBenchmark completed.\n";
    }

private:
    void setupNetworking() {
        std::cout << "Setting up benchmark environment...\n";
        
        // Create server with optimized settings
        Endpoint bind_addr{"localhost", 12348};
        HostConfig server_config{
            .maxPeers = 1,
            .maxChannels = 1,
            .enableCompression = false  // Disable compression for raw performance
        };
        
        server_host_ = Host::createServer(bind_addr, server_config);
        
        // Server event handlers
        server_host_->getDispatcher().onConnect([this](const ConnectEvent& event) {
            std::cout << "Client connected for benchmark\n";
        });
        
        server_host_->getDispatcher().onReceive([this](const ReceiveEvent& event) {
            bytes_received_ += event.packetData->size();
            packets_received_++;
            
            // Echo the packet back to client for latency testing
            PacketBuilder echo_builder;
            PacketReader reader(*event.packetData);
            
            // Check if this is a latency test packet
            if (event.packetData->size() >= 8) {
                echo_builder.write(event.packetData->data().data(), event.packetData->size());
                auto echo_packet = echo_builder.build(static_cast<PacketFlagsT>(PacketFlag::reliable));
                event.peerHandle->send(std::move(echo_packet));
            }
        });
        
        // Create client
        HostConfig client_config{
            .maxPeers = 1,
            .maxChannels = 1,
            .enableCompression = false
        };
        
        client_host_ = Host::createClient(client_config);
        
        // Client event handlers
        client_host_->getDispatcher().onReceive([this](const ReceiveEvent& event) {
            bytes_received_ += event.packetData->size();
            packets_received_++;
            
            // Check if this is a latency measurement echo
            if (event.packetData->size() >= 16) {
                PacketReader reader(*event.packetData);
                try {
                    auto sent_time = reader.readUint64();
                    auto now = std::chrono::steady_clock::now().time_since_epoch().count();
                    auto latency = (now - sent_time) / 1000000.0; // Convert to milliseconds
                    
                    std::lock_guard<std::mutex> lock(latency_mutex_);
                    latency_measurements_.push_back(latency);
                } catch (...) {
                    // Ignore malformed packets
                }
            }
        });
        
        // Start service threads
        server_host_->startServiceThread();
        client_host_->startServiceThread();
        
        // Connect client to server
        server_peer_ = client_host_->connect(bind_addr);
        
        // Wait for connection to establish
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        
        if (!server_peer_->isConnected()) {
            throw std::runtime_error("Failed to establish connection for benchmark");
        }
        
        std::cout << "Benchmark environment ready\n";
    }

    void runThroughputTest() {
        std::cout << "\n=== Throughput Test ===\n";
        
        resetCounters();
        const int test_duration_seconds = 10;
        const int packet_size = 1024; // 1KB packets
        
        benchmark_running_ = true;
        benchmark_start_ = std::chrono::steady_clock::now();
        
        // Start sender thread
        std::thread sender_thread([this, packet_size, test_duration_seconds]() {
            std::vector<uint8_t> test_data(packet_size, 0x42); // Fill with test pattern
            
            auto end_time = std::chrono::steady_clock::now() + std::chrono::seconds(test_duration_seconds);
            
            while (std::chrono::steady_clock::now() < end_time && benchmark_running_) {
                PacketBuilder builder;
                builder.write(test_data.data(), test_data.size());
                auto packet = builder.build(static_cast<PacketFlagsT>(PacketFlag::reliable));
                
                if (server_peer_->send(std::move(packet))) {
                    packets_sent_++;
                    bytes_sent_ += packet_size;
                }
                
                // Small delay to prevent overwhelming the network
                std::this_thread::sleep_for(std::chrono::microseconds(10));
            }
        });
        
        // Wait for test duration
        std::this_thread::sleep_for(std::chrono::seconds(test_duration_seconds));
        benchmark_running_ = false;
        sender_thread.join();
        
        // Allow time for final packets to arrive
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        
        // Calculate results
        auto elapsed = std::chrono::steady_clock::now() - benchmark_start_;
        auto elapsed_seconds = std::chrono::duration<double>(elapsed).count();
        
        auto packets_per_second = packets_received_ / elapsed_seconds;
        auto bytes_per_second = bytes_received_ / elapsed_seconds;
        auto mbps = (bytes_per_second * 8) / (1024 * 1024); // Convert to Mbps
        
        std::cout << std::fixed << std::setprecision(2);
        std::cout << "Test Duration: " << elapsed_seconds << " seconds\n";
        std::cout << "Packets Sent: " << packets_sent_ << "\n";
        std::cout << "Packets Received: " << packets_received_ << "\n";
        std::cout << "Packet Loss: " << std::setprecision(4) 
                  << ((double)(packets_sent_ - packets_received_) / packets_sent_ * 100.0) << "%\n";
        std::cout << "Throughput: " << std::setprecision(0) << packets_per_second << " packets/sec\n";
        std::cout << "Bandwidth: " << std::setprecision(2) << (bytes_per_second / 1024) << " KB/sec (" 
                  << mbps << " Mbps)\n";
    }

    void runLatencyTest() {
        std::cout << "\n=== Latency Test ===\n";
        
        resetCounters();
        latency_measurements_.clear();
        
        const int num_pings = 100;
        const int ping_interval_ms = 100;
        
        std::cout << "Sending " << num_pings << " ping packets...\n";
        
        for (int i = 0; i < num_pings; ++i) {
            auto timestamp = std::chrono::steady_clock::now().time_since_epoch().count();
            
            PacketBuilder ping_builder;
            ping_builder.writeUint64(timestamp);
            ping_builder.writeUint64(0); // Padding for echo
            auto ping_packet = ping_builder.build(static_cast<PacketFlagsT>(PacketFlag::reliable));
            
            if (server_peer_->send(std::move(ping_packet))) {
                packets_sent_++;
            }
            
            std::this_thread::sleep_for(std::chrono::milliseconds(ping_interval_ms));
        }
        
        // Wait for all responses
        std::this_thread::sleep_for(std::chrono::seconds(2));
        
        // Calculate latency statistics
        std::lock_guard<std::mutex> lock(latency_mutex_);
        
        if (latency_measurements_.empty()) {
            std::cout << "No latency measurements received!\n";
            return;
        }
        
        std::sort(latency_measurements_.begin(), latency_measurements_.end());
        
        auto min_latency = latency_measurements_.front();
        auto max_latency = latency_measurements_.back();
        auto avg_latency = std::accumulate(latency_measurements_.begin(), latency_measurements_.end(), 0.0) 
                          / latency_measurements_.size();
        
        auto p50_idx = latency_measurements_.size() / 2;
        auto p95_idx = (latency_measurements_.size() * 95) / 100;
        auto p99_idx = (latency_measurements_.size() * 99) / 100;
        
        auto p50_latency = latency_measurements_[p50_idx];
        auto p95_latency = latency_measurements_[p95_idx];
        auto p99_latency = latency_measurements_[p99_idx];
        
        std::cout << std::fixed << std::setprecision(2);
        std::cout << "Ping Results (" << latency_measurements_.size() << " responses):\n";
        std::cout << "  Min RTT: " << min_latency << " ms\n";
        std::cout << "  Avg RTT: " << avg_latency << " ms\n";
        std::cout << "  Max RTT: " << max_latency << " ms\n";
        std::cout << "  P50 RTT: " << p50_latency << " ms\n";
        std::cout << "  P95 RTT: " << p95_latency << " ms\n";
        std::cout << "  P99 RTT: " << p99_latency << " ms\n";
        std::cout << "  Packet Loss: " << std::setprecision(1) 
                  << ((double)(num_pings - latency_measurements_.size()) / num_pings * 100.0) << "%\n";
    }

    void runBurstTest() {
        std::cout << "\n=== Burst Test ===\n";
        
        resetCounters();
        const int burst_size = 1000;
        const int packet_size = 512;
        
        std::cout << "Sending burst of " << burst_size << " packets...\n";
        
        auto burst_start = std::chrono::steady_clock::now();
        
        // Send burst of packets as fast as possible
        std::vector<uint8_t> test_data(packet_size, 0x55);
        
        for (int i = 0; i < burst_size; ++i) {
            PacketBuilder builder;
            builder.writeUint32(i); // Sequence number
            builder.write(test_data.data(), test_data.size() - 4);
            auto packet = builder.build(static_cast<PacketFlagsT>(PacketFlag::reliable));
            
            if (server_peer_->send(std::move(packet))) {
                packets_sent_++;
                bytes_sent_ += packet_size;
            }
        }
        
        auto burst_end = std::chrono::steady_clock::now();
        auto burst_duration = std::chrono::duration<double>(burst_end - burst_start).count();
        
        // Wait for all packets to arrive
        std::this_thread::sleep_for(std::chrono::seconds(3));
        
        auto send_rate = burst_size / burst_duration;
        
        std::cout << std::fixed << std::setprecision(0);
        std::cout << "Burst sent in " << std::setprecision(4) << burst_duration << " seconds\n";
        std::cout << "Send rate: " << std::setprecision(0) << send_rate << " packets/sec\n";
        std::cout << "Packets received: " << packets_received_ << " / " << packets_sent_ << "\n";
        std::cout << "Reception rate: " << std::setprecision(2) 
                  << ((double)packets_received_ / packets_sent_ * 100.0) << "%\n";
    }

    void resetCounters() {
        packets_sent_ = 0;
        packets_received_ = 0;
        bytes_sent_ = 0;
        bytes_received_ = 0;
    }

    void cleanup() {
        std::cout << "\nCleaning up benchmark environment...\n";
        
        benchmark_running_ = false;
        
        if (server_peer_ && server_peer_->isConnected()) {
            server_peer_->disconnect();
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        
        if (client_host_) {
            client_host_->stopServiceThread();
        }
        
        if (server_host_) {
            server_host_->stopServiceThread();
        }
        
        server_peer_.reset();
        client_host_.reset();
        server_host_.reset();
    }
};

int main() {
    PerformanceBenchmark benchmark;
    benchmark.runBenchmark();
    return 0;
}