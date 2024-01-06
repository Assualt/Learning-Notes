//
// Created by 侯鑫 on 2024/1/6.
//

#ifndef SSP_TOOLS_POLL_POLLER_H
#define SSP_TOOLS_POLL_POLLER_H

#include "net/poller.h"
#include <poll.h>
#include <vector>

namespace ssp::net {

class PollPoller : public Poller{
public:
    explicit PollPoller(EventLoop *loop);

    ~PollPoller() override;

public:
    TimeStamp Poll(std::chrono::seconds, ChannelList &list) override;

    void UpdateChannel(Channel *channel) override;

    void RemoveChannel(Channel *channel) override;

private:
    void FillActiveChannel(int32_t numEvents, ChannelList &channels);

private:
    using PollFdList = std::vector<struct pollfd>;
    PollFdList pollFdList_;

};
}

#endif //SSP_TOOLS_POLL_POLLER_H
