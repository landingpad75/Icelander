#include "icelander.hpp"
#include <stdexcept>
#include <cstring>
#include <algorithm>

namespace icelander {
    Packet::Packet(Span<const uint8_t> packetData, PacketFlagsT flags)
        : Packet(packetData.data(), packetData.size(), flags) {}

    Packet::Packet(const void* packetData, size_t size, PacketFlagsT flags) {
        nativePacket_ = enet_packet_create(packetData, size, flags);
        if (!nativePacket_) {
            throw std::runtime_error("Failed to create ENet packet");
        }
    }

    Packet::Packet(const std::string& packetData, PacketFlagsT flags)
        : Packet(packetData.data(), packetData.size(), flags) {}

    Packet::~Packet() {
        if (nativePacket_) {
            enet_packet_destroy(nativePacket_);
        }
    }

    Packet::Packet(Packet&& other) noexcept
        : nativePacket_(std::exchange(other.nativePacket_, nullptr)) {}

    Packet& Packet::operator=(Packet&& other) noexcept {
        if (this != &other) {
            if (nativePacket_) {
                enet_packet_destroy(nativePacket_);
            }
            nativePacket_ = std::exchange(other.nativePacket_, nullptr);
        }
        return *this;
    }

    auto Packet::create(Span<const uint8_t> packetData, PacketFlagsT flags) -> std::unique_ptr<Packet> {
        return std::make_unique<Packet>(packetData, flags);
    }

    auto Packet::create(const void* packetData, size_t size, PacketFlagsT flags) -> std::unique_ptr<Packet> {
        return std::make_unique<Packet>(packetData, size, flags);
    }

    auto Packet::create(const std::string& packetData, PacketFlagsT flags) -> std::unique_ptr<Packet> {
        return std::make_unique<Packet>(packetData, flags);
    }

    auto Packet::data() const -> Span<const uint8_t> {
        if (!nativePacket_) {
            return Span<const uint8_t>(nullptr, 0);
        }
        return Span<const uint8_t>(nativePacket_->data, nativePacket_->dataLength);
    }

    auto Packet::size() const -> size_t {
        return nativePacket_ ? nativePacket_->dataLength : 0;
    }

    auto Packet::flags() const -> PacketFlagsT {
        return nativePacket_ ? nativePacket_->flags : 0;
    }

    auto Packet::referenceCount() const -> size_t {
        return nativePacket_ ? nativePacket_->referenceCount : 0;
    }

    auto Packet::asString() const -> std::string {
        auto packetData = data();
        return std::string(reinterpret_cast<const char*>(packetData.data()), packetData.size());
    }

    auto Packet::toVector() const -> std::vector<uint8_t> {
        auto packetData = data();
        return std::vector<uint8_t>(packetData.begin(), packetData.end());
    }

    void Packet::resize(size_t newSize) {
        if (!nativePacket_) {
            throw std::runtime_error("Cannot resize null packet");
        }

        if (enet_packet_resize(nativePacket_, newSize) != 0) {
            throw std::runtime_error("Failed to resize packet");
        }
    }

    bool Packet::hasFlag(PacketFlag flag) const {
        return nativePacket_ && (nativePacket_->flags & static_cast<PacketFlagsT>(flag)) != 0;
    }

    auto Packet::nativeHandle() const -> ENetPacket* {
        return nativePacket_;
    }

    auto Packet::fromNative(ENetPacket* nativePacket) -> std::unique_ptr<Packet> {
        if (!nativePacket) {
            return nullptr;
        }
        return std::unique_ptr<Packet>(new Packet(nativePacket));
    }

    Packet::Packet(ENetPacket* nativePacket) : nativePacket_(nativePacket) {}

    // PacketBuilder implementation
    PacketBuilder::PacketBuilder() = default;

    PacketBuilder::PacketBuilder(size_t initialCapacity) {
        buffer_.reserve(initialCapacity);
    }

    auto PacketBuilder::write(Span<const uint8_t> packetData) -> PacketBuilder& {
        buffer_.insert(buffer_.end(), packetData.begin(), packetData.end());
        return *this;
    }

    auto PacketBuilder::write(const std::string& packetData) -> PacketBuilder& {
        auto bytes = reinterpret_cast<const uint8_t*>(packetData.data());
        buffer_.insert(buffer_.end(), bytes, bytes + packetData.size());
        return *this;
    }

    auto PacketBuilder::write(const void* packetData, size_t size) -> PacketBuilder& {
        auto bytes = static_cast<const uint8_t*>(packetData);
        buffer_.insert(buffer_.end(), bytes, bytes + size);
        return *this;
    }

    auto PacketBuilder::writeUint8(uint8_t value) -> PacketBuilder& {
        buffer_.push_back(value);
        return *this;
    }

    auto PacketBuilder::writeUint16(uint16_t value) -> PacketBuilder& {
        return write(&value, sizeof(value));
    }

    auto PacketBuilder::writeUint32(uint32_t value) -> PacketBuilder& {
        return write(&value, sizeof(value));
    }

    auto PacketBuilder::writeUint64(uint64_t value) -> PacketBuilder& {
        return write(&value, sizeof(value));
    }

    auto PacketBuilder::writeString(const std::string& str) -> PacketBuilder& {
        writeUint32(static_cast<uint32_t>(str.size()));
        return write(str);
    }

    auto PacketBuilder::reserve(size_t capacity) -> PacketBuilder& {
        buffer_.reserve(capacity);
        return *this;
    }

    auto PacketBuilder::clear() -> PacketBuilder& {
        buffer_.clear();
        return *this;
    }

    auto PacketBuilder::size() const -> size_t {
        return buffer_.size();
    }

    auto PacketBuilder::capacity() const -> size_t {
        return buffer_.capacity();
    }

    auto PacketBuilder::empty() const -> bool {
        return buffer_.empty();
    }

    auto PacketBuilder::build(PacketFlagsT flags) -> std::unique_ptr<Packet> {
        return Packet::create(Span<const uint8_t>(buffer_.data(), buffer_.size()), flags);
    }

    auto PacketBuilder::data() const -> Span<const uint8_t> {
        return Span<const uint8_t>(buffer_.data(), buffer_.size());
    }

    // PacketReader implementation
    PacketReader::PacketReader(const Packet& pkt) : PacketReader(pkt.data()) {}

    PacketReader::PacketReader(Span<const uint8_t> packetData) : data_(packetData), position_(0) {}

    auto PacketReader::read(void* buffer, size_t size) -> bool {
        if (position_ + size > data_.size()) {
            return false;
        }
        std::memcpy(buffer, data_.data() + position_, size);
        position_ += size;
        return true;
    }

    auto PacketReader::readUint8() -> uint8_t {
        if (position_ >= data_.size()) {
            throw std::runtime_error("Not enough data to read uint8");
        }
        return data_[position_++];
    }

    auto PacketReader::readUint16() -> uint16_t {
        uint16_t value;
        if (!read(&value, sizeof(value))) {
            throw std::runtime_error("Not enough data to read uint16");
        }
        return value;
    }

    auto PacketReader::readUint32() -> uint32_t {
        uint32_t value;
        if (!read(&value, sizeof(value))) {
            throw std::runtime_error("Not enough data to read uint32");
        }
        return value;
    }

    auto PacketReader::readUint64() -> uint64_t {
        uint64_t value;
        if (!read(&value, sizeof(value))) {
            throw std::runtime_error("Not enough data to read uint64");
        }
        return value;
    }

    auto PacketReader::readString(size_t length) -> std::string {
        if (position_ + length > data_.size()) {
            throw std::runtime_error("Not enough data to read string");
        }
        std::string result(reinterpret_cast<const char*>(data_.data() + position_), length);
        position_ += length;
        return result;
    }

    auto PacketReader::remaining() const -> size_t {
        return data_.size() - position_;
    }

    auto PacketReader::position() const -> size_t {
        return position_;
    }

    auto PacketReader::size() const -> size_t {
        return data_.size();
    }

    auto PacketReader::atEnd() const -> bool {
        return position_ >= data_.size();
    }

    void PacketReader::seek(size_t position) {
        position_ = position < data_.size() ? position : data_.size();
    }

    void PacketReader::skip(size_t bytes) {
        size_t newPos = position_ + bytes;
        position_ = newPos < data_.size() ? newPos : data_.size();
    }

    void PacketReader::reset() {
        position_ = 0;
    }

    auto PacketReader::asString() const -> std::string {
        return std::string(reinterpret_cast<const char*>(data_.data()), data_.size());
    }
}