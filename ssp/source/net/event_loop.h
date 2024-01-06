//
// Created by 侯鑫 on 2024/1/3.
//

#ifndef SSP_TOOLS_EVENT_LOOP_H
#define SSP_TOOLS_EVENT_LOOP_H

#include <memory>
#include "poller.h"
namespace ssp::net {

enum LoopState {
    FREE,
    APPLIED,
};

class EventLoop {
public:
    EventLoop();
    ~EventLoop();

public:
    void Loop();

    void Quit();

    void UpdateChannel(Channel *channel);

    static EventLoop *ApplyLoop();

    [[nodiscard]] LoopState GetState() const;

    void RemoveChannel(Channel *channel);

private:
    static void DispatchEvent(const TimeStamp& stamp, ChannelList &channels);

private:
    bool quit_{false};

    std::unique_ptr<Poller> poller_{nullptr};
    static std::vector<EventLoop> loopList_;
    LoopState state_{FREE};
};

}
#endif //SSP_TOOLS_EVENT_LOOP_H
