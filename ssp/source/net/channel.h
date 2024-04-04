//
// Created by 侯鑫 on 2024/1/7.
//

#ifndef SSP_TOOLS_CHANNEL_H
#define SSP_TOOLS_CHANNEL_H

#include "event_loop.h"
#include "socket.h"
#include "base/timestamp.h"
#include "callback.h"
#include <cstdint>

namespace ssp::net {
class Channel {
public:
    explicit Channel(EventLoop *loop, Socket *socket);

    ~Channel() = default;

public:

    void EnableRead();

    void EnableWrite();

    void Disabled();

    void Remove();

    [[nodiscard]] int32_t Index() const;

    void SetIndex(int32_t val);

    [[nodiscard]] int32_t Fd() const;

    [[nodiscard]] int32_t Events() const;

    void HandleEvent(const TimeStamp& stamp) const;

    void SetEvents(int32_t event);

    void SetChannelCallback(EventCallback readFn, EventCallback writeFn, EventCallback errFn, EventCallback closeFn);

private:
    void Update();

private:
    EventLoop* loop_{nullptr};
    int32_t channelFd_{-1};
    int32_t eventFlag_{};
    int32_t index_{-1};
    int32_t rcvEvent_{};

    NetCallback netFn_;
};

}

#endif //SSP_TOOLS_CHANNEL_H
