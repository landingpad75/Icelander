#include "icelander.hpp"

namespace icelander {
    void EventDispatcher::onConnect(ConnectHandler handler) {
        connectHandlers_.push_back(std::move(handler));
    }

    void EventDispatcher::onDisconnect(DisconnectHandler handler) {
        disconnectHandlers_.push_back(std::move(handler));
    }

    void EventDispatcher::onReceive(ReceiveHandler handler) {
        receiveHandlers_.push_back(std::move(handler));
    }

    void EventDispatcher::onReceive(ChannelIdT channel, ReceiveHandler handler) {
        channelHandlers_[channel].push_back(std::move(handler));
    }

    void EventDispatcher::clearHandlers() {
        connectHandlers_.clear();
        disconnectHandlers_.clear();
        receiveHandlers_.clear();
        channelHandlers_.clear();
    }

    void EventDispatcher::dispatchConnect(const ConnectEvent& event) {
        for (const auto& handler : connectHandlers_) {
            handler(event);
        }
    }

    void EventDispatcher::dispatchDisconnect(const DisconnectEvent& event) {
        for (const auto& handler : disconnectHandlers_) {
            handler(event);
        }
    }

    void EventDispatcher::dispatchReceive(const ReceiveEvent& event) {
        for (const auto& handler : receiveHandlers_) {
            handler(event);
        }

        auto channelIt = channelHandlers_.find(event.channel);
        if (channelIt != channelHandlers_.end()) {
            for (const auto& handler : channelIt->second) {
                handler(event);
            }
        }
    }
}