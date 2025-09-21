#include "../../include/icelander.hpp"
#include <iostream>

using namespace icelander;

int main() {
    std::cout << "=== Packet Builder Example ===\n";

    PacketBuilder builder(1024);

    builder.writeUint32(0x12345678)
           .writeString("Hello, World!")
           .writeUint32(314159)
           .writeUint8(255);

    auto pkt = builder.build(static_cast<PacketFlagsT>(PacketFlag::reliable));

    PacketReader reader(*pkt);
    auto magic = reader.readUint32();
    auto str_len = reader.readUint32();
    auto message = reader.readString(str_len);
    auto number = reader.readUint32();
    auto max_byte = reader.readUint8();

    std::cout << "Magic: 0x" << std::hex << magic << std::dec << "\n";
    std::cout << "Message: " << message << "\n";
    std::cout << "Number: " << number << "\n";
    std::cout << "Max byte: " << static_cast<int>(max_byte) << "\n";
    std::cout << "Packet size: " << pkt->size() << " bytes\n";

    return 0;
}