#include "icelander.hpp"
#include <stdexcept>

namespace icelander {
    auto Endpoint::toEnetAddress() const -> AddressT {
        AddressT addr;

        if (enet_address_set_host(&addr, host.c_str()) != 0) {
            throw std::runtime_error("Failed to resolve hostname: " + host);
        }

        addr.port = port;
        return addr;
    }

    auto Endpoint::fromEnetAddress(const AddressT& addr) -> Endpoint {
        char hostname[256];

        if (enet_address_get_host(&addr, hostname, sizeof(hostname)) != 0) {
            throw std::runtime_error("Failed to get hostname from address");
        }

        return Endpoint{std::string(hostname), addr.port};
    }
}