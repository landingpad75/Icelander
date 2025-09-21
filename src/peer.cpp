#include "icelander.hpp"
#include <stdexcept>

namespace icelander {
    Peer::Peer(ENetPeer* nativePeer, std::shared_ptr<Host> hostPtr)
        : nativePeer_(nativePeer), host_(hostPtr), userData_(nullptr) {}

    Peer::~Peer() {
        userData_ = nullptr;
    }

    Peer::Peer(Peer&& other) noexcept
        : nativePeer_(std::exchange(other.nativePeer_, nullptr))
        , host_(std::move(other.host_))
        , userData_(std::exchange(other.userData_, nullptr)) {}

    Peer& Peer::operator=(Peer&& other) noexcept {
        if (this != &other) {
            nativePeer_ = std::exchange(other.nativePeer_, nullptr);
            host_ = std::move(other.host_);
            userData_ = std::exchange(other.userData_, nullptr);
        }
        return *this;
    }

    auto Peer::send(ChannelIdT channel, std::unique_ptr<Packet> pkt) -> bool {
        if (!nativePeer_ || !pkt) {
            return false;
        }

        auto nativePacket = pkt->nativeHandle();
        pkt.release();

        return enet_peer_send(nativePeer_, channel, nativePacket) == 0;
    }

    auto Peer::send(std::unique_ptr<Packet> pkt) -> bool {
        return send(0, std::move(pkt));
    }

    auto Peer::disconnect(uint32_t disconnectData) -> void {
        if (nativePeer_) {
            enet_peer_disconnect(nativePeer_, disconnectData);
        }
    }

    auto Peer::disconnectNow(uint32_t disconnectData) -> void {
        if (nativePeer_) {
            enet_peer_disconnect_now(nativePeer_, disconnectData);
        }
    }

    auto Peer::disconnectLater(uint32_t disconnectData) -> void {
        if (nativePeer_) {
            enet_peer_disconnect_later(nativePeer_, disconnectData);
        }
    }

    auto Peer::ping() -> void {
        if (nativePeer_) {
            enet_peer_ping(nativePeer_);
        }
    }

    auto Peer::timeout(TimeoutMs timeoutLimit, TimeoutMs timeoutMinimum, TimeoutMs timeoutMaximum) -> void {
        if (nativePeer_) {
            enet_peer_timeout(nativePeer_,
                static_cast<uint32_t>(timeoutLimit.count()),
                static_cast<uint32_t>(timeoutMinimum.count()),
                static_cast<uint32_t>(timeoutMaximum.count()));
        }
    }

    auto Peer::reset() -> void {
        if (nativePeer_) {
            enet_peer_reset(nativePeer_);
        }
    }

    auto Peer::state() const -> PeerState {
        return nativePeer_ ? static_cast<PeerState>(nativePeer_->state) : PeerState::disconnected;
    }

    auto Peer::endpoint() const -> icelander::Endpoint {
        if (!nativePeer_) {
            return {"", 0};
        }
        return icelander::Endpoint::fromEnetAddress(nativePeer_->address);
    }

    auto Peer::roundTripTime() const -> std::chrono::milliseconds {
        return nativePeer_ ? std::chrono::milliseconds(nativePeer_->roundTripTime) : std::chrono::milliseconds{0};
    }

    auto Peer::isConnected() const -> bool {
        return state() == PeerState::connected;
    }

    auto Peer::isConnecting() const -> bool {
        auto currentState = state();
        return currentState == PeerState::connecting ||
               currentState == PeerState::acknowledgingConnect ||
               currentState == PeerState::connectionPending ||
               currentState == PeerState::connectionSucceeded;
    }

    auto Peer::isDisconnecting() const -> bool {
        auto currentState = state();
        return currentState == PeerState::disconnectLater ||
               currentState == PeerState::disconnecting ||
               currentState == PeerState::acknowledgingDisconnect;
    }

    auto Peer::isDisconnected() const -> bool {
        return state() == PeerState::disconnected;
    }

    auto Peer::nativeHandle() const -> ENetPeer* {
        return nativePeer_;
    }

    auto Peer::hostHandle() const -> std::shared_ptr<Host> {
        return host_.lock();
    }

    auto Peer::fromNative(ENetPeer* nativePeer, std::shared_ptr<Host> hostPtr) -> std::shared_ptr<Peer> {
        if (!nativePeer) {
            return nullptr;
        }
        return std::make_shared<Peer>(nativePeer, hostPtr);
    }
}