#include "icelander.hpp"
#include <stdexcept>

namespace icelander {
    bool Library::initialize() {
        if (initialized_) {
            return true;
        }

        if (enet_initialize() == 0) {
            initialized_ = true;
            return true;
        }

        return false;
    }

    void Library::deinitialize() {
        if (initialized_) {
            enet_deinitialize();
            initialized_ = false;
        }
    }

    auto Library::version() -> uint32_t {
        return ENET_VERSION;
    }

    bool Library::isInitialized() {
        return initialized_;
    }

    // Static member definition - remove since it's inline in header
}