//
// Created by 侯鑫 on 2024/1/28.
//

#ifndef SSP_TOOLS_KQUEUE_POLLER_H
#define SSP_TOOLS_KQUEUE_POLLER_H

#include "net/poller.h"
#include "net/socket.h"
#include <sys/event.h>
#include <vector>

namespace ssp::net {

class KQueuePoller : public Poller {
public:
    explicit KQueuePoller(EventLoop *loop);

    ~KQueuePoller() override;

public:
    TimeStamp Poll(std::chrono::seconds, ChannelList &list) override;

    void UpdateChannel(Channel *channel) override;

    void RemoveChannel(Channel *channel) override;

private:
    void FillActiveChannel(int32_t nums, ChannelList &channels);

    void Start();

private:
    int32_t kq_{-1};
    std::unique_ptr<Socket> socket_;
    std::vector<struct kevent> events_;
};
}

#endif //SSP_TOOLS_KQUEUE_POLLER_H
