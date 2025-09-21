#include "icelander.hpp"
#include <stdexcept>

namespace icelander {
    auto Host::createServer(const Endpoint& bindEndpoint, const HostConfig& config) -> std::shared_ptr<Host> {
        ENetAddress address = bindEndpoint.toEnetAddress();

        ENetHost* nativeHost = enet_host_create(
            &address,
            config.maxPeers,
            config.maxChannels,
            config.incomingBandwidth,
            config.outgoingBandwidth
        );

        if (!nativeHost) {
            throw std::runtime_error("Failed to create ENet server host");
        }

        if (config.enableCompression) {
            enet_host_compress_with_range_coder(nativeHost);
        }

        return std::shared_ptr<Host>(new Host(nativeHost, true));
    }

    auto Host::createClient(const HostConfig& config) -> std::shared_ptr<Host> {
        ENetHost* nativeHost = enet_host_create(
            nullptr,
            config.maxPeers,
            config.maxChannels,
            config.incomingBandwidth,
            config.outgoingBandwidth
        );

        if (!nativeHost) {
            throw std::runtime_error("Failed to create ENet client host");
        }

        if (config.enableCompression) {
            enet_host_compress_with_range_coder(nativeHost);
        }

        return std::shared_ptr<Host>(new Host(nativeHost, false));
    }

    Host::Host(ENetHost* nativeHost, bool isServer)
        : nativeHost_(nativeHost)
        , isServer_(isServer)
        , dispatcher_(std::make_unique<EventDispatcher>())
        , serviceThreadRunning_(false) {}

    Host::~Host() {
        stopServiceThread();
        if (nativeHost_) {
            enet_host_destroy(nativeHost_);
        }
    }

    auto Host::connect(const Endpoint& remoteEndpoint, size_t channels, uint32_t connectData) -> std::shared_ptr<Peer> {
        if (!nativeHost_) {
            throw std::runtime_error("Invalid host");
        }

        ENetAddress address = remoteEndpoint.toEnetAddress();
        ENetPeer* nativePeer = enet_host_connect(nativeHost_, &address, channels, connectData);

        if (!nativePeer) {
            throw std::runtime_error("Failed to initiate connection");
        }

        auto peerWrapper = std::make_shared<Peer>(nativePeer, shared_from_this());

        {
            std::lock_guard<std::mutex> lock(peersMutex_);
            peers_.push_back(peerWrapper);
        }

        return peerWrapper;
    }

    auto Host::service(TimeoutMs timeout) -> int {
        if (!nativeHost_) {
            return 0;
        }

        ENetEvent event;
        int result = enet_host_service(nativeHost_, &event, static_cast<uint32_t>(timeout.count()));

        if (result > 0) {
            processEvent(event);
        }

        return result;
    }

    auto Host::flush() -> void {
        if (nativeHost_) {
            enet_host_flush(nativeHost_);
        }
    }

    auto Host::broadcast(ChannelIdT channel, std::unique_ptr<Packet> pkt) -> void {
        if (!nativeHost_ || !pkt) {
            return;
        }

        auto nativePacket = pkt->nativeHandle();
        pkt.release();

        enet_host_broadcast(nativeHost_, channel, nativePacket);
    }

    auto Host::broadcast(std::unique_ptr<Packet> pkt) -> void {
        broadcast(0, std::move(pkt));
    }

    auto Host::startServiceThread() -> void {
        if (serviceThreadRunning_) {
            return;
        }

        serviceThreadRunning_ = true;
        serviceThread_ = std::make_unique<std::thread>(&Host::serviceThreadLoop, this);
    }

    auto Host::stopServiceThread() -> void {
        if (!serviceThreadRunning_) {
            return;
        }

        serviceThreadRunning_ = false;
        if (serviceThread_ && serviceThread_->joinable()) {
            serviceThread_->join();
        }
        serviceThread_.reset();
    }

    auto Host::isServiceThreadRunning() const -> bool {
        return serviceThreadRunning_;
    }

    auto Host::peerCount() const -> size_t {
        std::lock_guard<std::mutex> lock(peersMutex_);
        size_t count = 0;
        for (const auto& weakPeer : peers_) {
            if (!weakPeer.expired()) {
                ++count;
            }
        }
        return count;
    }

    auto Host::isServer() const -> bool {
        return isServer_;
    }

    auto Host::isClient() const -> bool {
        return !isServer_;
    }

    auto Host::getPeers() -> std::vector<std::shared_ptr<Peer>> {
        std::lock_guard<std::mutex> lock(peersMutex_);
        std::vector<std::shared_ptr<Peer>> activePeers;

        for (auto it = peers_.begin(); it != peers_.end();) {
            if (auto peerPtr = it->lock()) {
                activePeers.push_back(peerPtr);
                ++it;
            } else {
                it = peers_.erase(it);
            }
        }

        return activePeers;
    }

    auto Host::findPeer(const Endpoint& remoteEndpoint) -> std::shared_ptr<Peer> {
        auto peers = getPeers();
        for (const auto& peerPtr : peers) {
            if (peerPtr->endpoint().host == remoteEndpoint.host &&
                peerPtr->endpoint().port == remoteEndpoint.port) {
                return peerPtr;
            }
        }
        return nullptr;
    }

    auto Host::getDispatcher() -> EventDispatcher& {
        return *dispatcher_;
    }

    auto Host::nativeHandle() const -> ENetHost* {
        return nativeHost_;
    }

    void Host::serviceThreadLoop() {
        while (serviceThreadRunning_) {
            service(std::chrono::milliseconds(10));
        }
    }

    auto Host::processEvent(const ENetEvent& event) -> void {
        if (!dispatcher_) {
            return;
        }

        switch (event.type) {
            case ENET_EVENT_TYPE_CONNECT: {
                auto peerWrapper = std::make_shared<Peer>(event.peer, shared_from_this());
                {
                    std::lock_guard<std::mutex> lock(peersMutex_);
                    peers_.push_back(peerWrapper);
                }

                ConnectEvent connectEvt;
                connectEvt.peerHandle = peerWrapper;
                connectEvt.remoteEndpoint = Endpoint::fromEnetAddress(event.peer->address);
                connectEvt.data = event.data;
                dispatcher_->dispatchConnect(connectEvt);
                break;
            }

            case ENET_EVENT_TYPE_DISCONNECT: {
                auto peerWrapper = findPeerByNative(event.peer);
                if (peerWrapper) {
                    DisconnectEvent disconnectEvt;
                    disconnectEvt.peerHandle = peerWrapper;
                    disconnectEvt.remoteEndpoint = Endpoint::fromEnetAddress(event.peer->address);
                    disconnectEvt.data = event.data;
                    dispatcher_->dispatchDisconnect(disconnectEvt);
                }
                break;
            }

            case ENET_EVENT_TYPE_RECEIVE: {
                auto peerWrapper = findPeerByNative(event.peer);
                if (peerWrapper && event.packet) {
                    auto packetWrapper = Packet::fromNative(event.packet);
                    ReceiveEvent receiveEvt;
                    receiveEvt.peerHandle = peerWrapper;
                    receiveEvt.packetData = std::move(packetWrapper);
                    receiveEvt.channel = event.channelID;
                    dispatcher_->dispatchReceive(receiveEvt);
                }
                break;
            }

            default:
                break;
        }
    }

    auto Host::findPeerByNative(ENetPeer* nativePeer) -> std::shared_ptr<Peer> {
        auto peers = getPeers();
        for (const auto& peerPtr : peers) {
            if (peerPtr->nativeHandle() == nativePeer) {
                return peerPtr;
            }
        }
        return nullptr;
    }
}