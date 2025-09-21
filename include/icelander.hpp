#pragma once

#include "../outside/enet/include/enet/enet.h"
#include <memory>
#include <string>
#include <vector>
#include <functional>
#include <chrono>
#include <atomic>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <unordered_map>
#include <optional>
#include <type_traits>
#include <memory>
#include <string>
#include <string_view>
#include <cstdint>
#include <stdexcept>

namespace icelander {
    using AddressT = ENetAddress;
    using SocketT = ENetSocket;
    using VersionT = ENetVersion;
    using ChannelIdT = uint8_t;
    using PeerIdT = uint16_t;
    using PacketFlagsT = uint32_t;
    using TimeoutMs = std::chrono::milliseconds;
    using Timestamp = std::chrono::steady_clock::time_point;

    constexpr size_t MAX_CHANNELS = ENET_PROTOCOL_MAXIMUM_CHANNEL_COUNT;
    constexpr size_t MAX_PEERS = 4096;
    constexpr PacketFlagsT DEFAULT_FLAGS = ENET_PACKET_FLAG_RELIABLE;
    constexpr TimeoutMs DEFAULT_TIMEOUT{1000};

    enum class PacketFlag : PacketFlagsT {
        reliable = ENET_PACKET_FLAG_RELIABLE,
        unsequenced = ENET_PACKET_FLAG_UNSEQUENCED,
        noAllocate = ENET_PACKET_FLAG_NO_ALLOCATE,
        unreliableFragment = ENET_PACKET_FLAG_UNRELIABLE_FRAGMENT,
        sent = ENET_PACKET_FLAG_SENT
    };

    enum class PeerState {
        disconnected = ENET_PEER_STATE_DISCONNECTED,
        connecting = ENET_PEER_STATE_CONNECTING,
        acknowledgingConnect = ENET_PEER_STATE_ACKNOWLEDGING_CONNECT,
        connectionPending = ENET_PEER_STATE_CONNECTION_PENDING,
        connectionSucceeded = ENET_PEER_STATE_CONNECTION_SUCCEEDED,
        connected = ENET_PEER_STATE_CONNECTED,
        disconnectLater = ENET_PEER_STATE_DISCONNECT_LATER,
        disconnecting = ENET_PEER_STATE_DISCONNECTING,
        acknowledgingDisconnect = ENET_PEER_STATE_ACKNOWLEDGING_DISCONNECT,
        zombie = ENET_PEER_STATE_ZOMBIE
    };

    enum class EventType {
        none = ENET_EVENT_TYPE_NONE,
        connect = ENET_EVENT_TYPE_CONNECT,
        disconnect = ENET_EVENT_TYPE_DISCONNECT,
        receive = ENET_EVENT_TYPE_RECEIVE
    };

    struct Endpoint {
        std::string host;
        uint16_t port;

        auto toEnetAddress() const -> AddressT;
        static auto fromEnetAddress(const AddressT& addr) -> Endpoint;
    };

    template<typename T>
    concept Serializable = std::is_trivially_copyable_v<T> && std::is_standard_layout_v<T>;

    template<typename T>
    struct Span {
        const T* data_;
        size_t size_;
        
        Span(const T* data, size_t size) : data_(data), size_(size) {}
        
        const T* data() const { return data_; }
        size_t size() const { return size_; }
        const T* begin() const { return data_; }
        const T* end() const { return data_ + size_; }
        const T& operator[](size_t i) const { return data_[i]; }
    };

    class Library {
    public:
        static bool initialize();
        static void deinitialize();
        static auto version() -> uint32_t;
        static bool isInitialized();

    private:
        static inline bool initialized_ = false;
    };

    class Packet {
    public:
        Packet(Span<const uint8_t> packetData, PacketFlagsT flags = DEFAULT_FLAGS);
        Packet(const void* packetData, size_t size, PacketFlagsT flags = DEFAULT_FLAGS);
        Packet(const std::string& packetData, PacketFlagsT flags = DEFAULT_FLAGS);

        template<Serializable T>
        Packet(const T& data, PacketFlagsT flags = DEFAULT_FLAGS);

        ~Packet();

        Packet(const Packet&) = delete;
        Packet& operator=(const Packet&) = delete;
        Packet(Packet&& other) noexcept;
        Packet& operator=(Packet&& other) noexcept;

        static auto create(Span<const uint8_t> packetData, PacketFlagsT flags = DEFAULT_FLAGS) -> std::unique_ptr<Packet>;
        static auto create(const void* packetData, size_t size, PacketFlagsT flags = DEFAULT_FLAGS) -> std::unique_ptr<Packet>;
        static auto create(const std::string& packetData, PacketFlagsT flags = DEFAULT_FLAGS) -> std::unique_ptr<Packet>;

        template<Serializable T>
        static auto create(const T& packetData, PacketFlagsT flags = DEFAULT_FLAGS) -> std::unique_ptr<Packet>;

        auto data() const -> Span<const uint8_t>;
        auto size() const -> size_t;
        auto flags() const -> PacketFlagsT;
        auto referenceCount() const -> size_t;

        template<Serializable T>
        auto as() const -> const T&;

        auto asString() const -> std::string;
        auto toVector() const -> std::vector<uint8_t>;

        void resize(size_t newSize);
        bool hasFlag(PacketFlag flag) const;

        auto nativeHandle() const -> ENetPacket*;
        static auto fromNative(ENetPacket* nativePacket) -> std::unique_ptr<Packet>;

    private:
        explicit Packet(ENetPacket* nativePacket);
        ENetPacket* nativePacket_;
    };

    class PacketBuilder {
    public:
        PacketBuilder();
        explicit PacketBuilder(size_t initialCapacity);

        template<Serializable T>
        auto write(const T& value) -> PacketBuilder&;

        auto write(Span<const uint8_t> packetData) -> PacketBuilder&;
        auto write(const std::string& packetData) -> PacketBuilder&;
        auto write(const void* packetData, size_t size) -> PacketBuilder&;

        auto writeUint8(uint8_t value) -> PacketBuilder&;
        auto writeUint16(uint16_t value) -> PacketBuilder&;
        auto writeUint32(uint32_t value) -> PacketBuilder&;
        auto writeUint64(uint64_t value) -> PacketBuilder&;

        auto writeString(const std::string& str) -> PacketBuilder&;

        auto reserve(size_t capacity) -> PacketBuilder&;
        auto clear() -> PacketBuilder&;

        auto size() const -> size_t;
        auto capacity() const -> size_t;
        auto empty() const -> bool;

        auto build(PacketFlagsT flags = DEFAULT_FLAGS) -> std::unique_ptr<Packet>;
        auto data() const -> Span<const uint8_t>;

    private:
        std::vector<uint8_t> buffer_;
    };

    class PacketReader {
    public:
        explicit PacketReader(const Packet& pkt);
        explicit PacketReader(Span<const uint8_t> packetData);

        template<Serializable T>
        auto read() -> T;

        auto read(void* buffer, size_t size) -> bool;
        auto readUint8() -> uint8_t;
        auto readUint16() -> uint16_t;
        auto readUint32() -> uint32_t;
        auto readUint64() -> uint64_t;
        auto readString(size_t length) -> std::string;

        auto remaining() const -> size_t;
        auto position() const -> size_t;
        auto size() const -> size_t;
        auto atEnd() const -> bool;

        void seek(size_t position);
        void skip(size_t bytes);
        void reset();

        auto asString() const -> std::string;

    private:
        Span<const uint8_t> data_;
        size_t position_;
    };

    class Host;
    class Peer {
    public:
        Peer(ENetPeer* nativePeer, std::shared_ptr<Host> hostPtr);
        ~Peer();

        Peer(const Peer&) = delete;
        Peer& operator=(const Peer&) = delete;
        Peer(Peer&& other) noexcept;
        Peer& operator=(Peer&& other) noexcept;

        auto send(ChannelIdT channel, std::unique_ptr<Packet> pkt) -> bool;
        auto send(std::unique_ptr<Packet> pkt) -> bool;

        template<typename T>
        auto send(ChannelIdT channel, const T& packetData, PacketFlagsT flags = DEFAULT_FLAGS) -> bool;

        template<typename T>
        auto send(const T& packetData, PacketFlagsT flags = DEFAULT_FLAGS) -> bool;

        auto disconnect(uint32_t disconnectData = 0) -> void;
        auto disconnectNow(uint32_t disconnectData = 0) -> void;
        auto disconnectLater(uint32_t disconnectData = 0) -> void;

        auto ping() -> void;
        auto timeout(TimeoutMs timeoutLimit, TimeoutMs timeoutMinimum, TimeoutMs timeoutMaximum) -> void;
        auto reset() -> void;

        auto state() const -> PeerState;
        auto endpoint() const -> icelander::Endpoint;
        auto roundTripTime() const -> std::chrono::milliseconds;

        auto isConnected() const -> bool;
        auto isConnecting() const -> bool;
        auto isDisconnecting() const -> bool;
        auto isDisconnected() const -> bool;

        auto nativeHandle() const -> ENetPeer*;
        auto hostHandle() const -> std::shared_ptr<Host>;

        static auto fromNative(ENetPeer* nativePeer, std::shared_ptr<Host> hostPtr) -> std::shared_ptr<Peer>;

    private:
        ENetPeer* nativePeer_;
        std::weak_ptr<Host> host_;
        void* userData_;
    };

    struct ConnectEvent {
        std::shared_ptr<Peer> peerHandle;
        Endpoint remoteEndpoint;
        uint32_t data;
    };

    struct DisconnectEvent {
        std::shared_ptr<Peer> peerHandle;
        Endpoint remoteEndpoint;
        uint32_t data;
    };

    struct ReceiveEvent {
        std::shared_ptr<Peer> peerHandle;
        std::unique_ptr<Packet> packetData;
        ChannelIdT channel;
    };

    using ConnectHandler = std::function<void(const ConnectEvent&)>;
    using DisconnectHandler = std::function<void(const DisconnectEvent&)>;
    using ReceiveHandler = std::function<void(const ReceiveEvent&)>;

    class EventDispatcher {
    public:
        void onConnect(ConnectHandler handler);
        void onDisconnect(DisconnectHandler handler);
        void onReceive(ReceiveHandler handler);
        void onReceive(ChannelIdT channel, ReceiveHandler handler);

        void clearHandlers();
        void dispatchConnect(const ConnectEvent& event);
        void dispatchDisconnect(const DisconnectEvent& event);
        void dispatchReceive(const ReceiveEvent& event);

    private:
        std::vector<ConnectHandler> connectHandlers_;
        std::vector<DisconnectHandler> disconnectHandlers_;
        std::vector<ReceiveHandler> receiveHandlers_;
        std::unordered_map<ChannelIdT, std::vector<ReceiveHandler>> channelHandlers_;
    };

    struct HostConfig {
        size_t maxPeers = 32;
        size_t maxChannels = 1;
        uint32_t incomingBandwidth = 0;
        uint32_t outgoingBandwidth = 0;
        bool enableCompression = false;
        TimeoutMs serviceTimeout = TimeoutMs{10};
    };

    class Host : public std::enable_shared_from_this<Host> {
    public:
        static auto createServer(const Endpoint& bindEndpoint, const HostConfig& config = {}) -> std::shared_ptr<Host>;
        static auto createClient(const HostConfig& config = {}) -> std::shared_ptr<Host>;

        ~Host();

        Host(const Host&) = delete;
        Host& operator=(const Host&) = delete;

        auto connect(const Endpoint& remoteEndpoint, size_t channels = 1, uint32_t connectData = 0) -> std::shared_ptr<Peer>;
        auto service(TimeoutMs timeout = DEFAULT_TIMEOUT) -> int;
        auto flush() -> void;

        auto broadcast(ChannelIdT channel, std::unique_ptr<Packet> pkt) -> void;
        auto broadcast(std::unique_ptr<Packet> pkt) -> void;

        template<typename T>
        auto broadcast(ChannelIdT channel, const T& packetData, PacketFlagsT flags = DEFAULT_FLAGS) -> void;

        template<typename T>
        auto broadcast(const T& packetData, PacketFlagsT flags = DEFAULT_FLAGS) -> void;

        auto startServiceThread() -> void;
        auto stopServiceThread() -> void;
        auto isServiceThreadRunning() const -> bool;

        auto peerCount() const -> size_t;
        auto isServer() const -> bool;
        auto isClient() const -> bool;

        auto getPeers() -> std::vector<std::shared_ptr<Peer>>;
        auto findPeer(const Endpoint& remoteEndpoint) -> std::shared_ptr<Peer>;

        auto getDispatcher() -> EventDispatcher&;
        auto nativeHandle() const -> ENetHost*;

    protected:
        explicit Host(ENetHost* nativeHost, bool isServer);

    private:
        void serviceThreadLoop();
        auto processEvent(const ENetEvent& event) -> void;
        auto findPeerByNative(ENetPeer* nativePeer) -> std::shared_ptr<Peer>;

        ENetHost* nativeHost_;
        bool isServer_;
        std::unique_ptr<EventDispatcher> dispatcher_;
        std::vector<std::weak_ptr<Peer>> peers_;

        std::atomic<bool> serviceThreadRunning_;
        std::unique_ptr<std::thread> serviceThread_;

        mutable std::mutex peersMutex_;
    };

    namespace async {
        template<typename T = void>
        struct Task {
            Task() = default;
            ~Task() = default;

            Task(const Task&) = delete;
            Task& operator=(const Task&) = delete;
            Task(Task&& other) noexcept = default;
            Task& operator=(Task&& other) noexcept = default;

            template<typename U = T>
            auto get() -> typename std::enable_if<!std::is_void_v<U>, U>::type {
                return result_;
            }

            template<typename U = T>
            typename std::enable_if<std::is_void_v<U>, void>::type get() {
            }

            bool done() const { return true; }
            void resume() {}

        private:
            T result_{};
        };

        class TaskScheduler {
        public:
            static auto instance() -> TaskScheduler&;

            void schedule(std::function<void()> task);
            template<typename T>
            void scheduleTask(Task<T>&& t);

            void start();
            void stop();
            bool isRunning() const;

        private:
            TaskScheduler() = default;
            void workerLoop();

            std::atomic<bool> running_{false};
            std::vector<std::unique_ptr<std::thread>> workerThreads_;
            std::queue<std::function<void()>> taskQueue_;
            std::mutex queueMutex_;
            std::condition_variable queueCv_;
        };

        auto sleepMs(int milliseconds) -> void;
    }

    template<Serializable T>
    Packet::Packet(const T& packetData, PacketFlagsT flags) : Packet(&packetData, sizeof(T), flags) {}

    template<Serializable T>
    auto Packet::create(const T& packetData, PacketFlagsT flags) -> std::unique_ptr<Packet> {
        return create(&packetData, sizeof(T), flags);
    }

    template<Serializable T>
    auto Packet::as() const -> const T& {
        if (size() < sizeof(T)) {
            throw std::runtime_error("Packet too small for requested type");
        }
        return *reinterpret_cast<const T*>(data().data());
    }

    template<Serializable T>
    auto PacketBuilder::write(const T& value) -> PacketBuilder& {
        return write(&value, sizeof(T));
    }

    template<Serializable T>
    auto PacketReader::read() -> T {
        T value;
        if (!read(&value, sizeof(T))) {
            throw std::runtime_error("Not enough data to read requested type");
        }
        return value;
    }

    template<typename T>
    auto Peer::send(ChannelIdT channel, const T& packetData, PacketFlagsT flags) -> bool {
        auto pkt = Packet::create(packetData, flags);
        return send(channel, std::move(pkt));
    }

    template<typename T>
    auto Peer::send(const T& packetData, PacketFlagsT flags) -> bool {
        return send(0, packetData, flags);
    }

    template<typename T>
    auto Host::broadcast(ChannelIdT channel, const T& packetData, PacketFlagsT flags) -> void {
        auto pkt = Packet::create(packetData, flags);
        broadcast(channel, std::move(pkt));
    }

    template<typename T>
    auto Host::broadcast(const T& packetData, PacketFlagsT flags) -> void {
        broadcast(0, packetData, flags);
    }

    template<typename T>
    void async::TaskScheduler::scheduleTask(Task<T>&& t) {
        schedule([task = std::move(t)]() mutable {
            task.resume();
        });
    }
}