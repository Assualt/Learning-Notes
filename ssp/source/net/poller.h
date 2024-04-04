//
// Created by 侯鑫 on 2024/1/7.
//

#ifndef SSP_TOOLS_POLLER_H
#define SSP_TOOLS_POLLER_H

#include "base/timestamp.h"
#include <map>
#include <vector>
#include <chrono>

using namespace ssp::base;
namespace ssp::net {
class EventLoop;
class Channel;

using ChannelList = std::vector<Channel*>;
class Poller {
public:
    explicit Poller(EventLoop *loop);

    virtual ~Poller() = default;

public:
    static Poller* NewPoller(EventLoop *loop);

public:
    virtual TimeStamp Poll(std::chrono::seconds timeout, ChannelList &channels) = 0;

    virtual void UpdateChannel(Channel *channel) = 0;

    virtual void RemoveChannel(Channel *channel) = 0;

protected:
    std::map<int32_t, Channel*> channelMapper_;
};
}

#endif //SSP_TOOLS_POLLER_H
