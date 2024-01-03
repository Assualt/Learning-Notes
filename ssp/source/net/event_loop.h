//
// Created by 侯鑫 on 2024/1/3.
//

#ifndef SSP_TOOLS_EVENT_LOOP_H
#define SSP_TOOLS_EVENT_LOOP_H

namespace ssp::net {

class EventLoop {
public:
    void Loop();

private:
    void DispatchEvent();

private:
    bool quit_{false};
};

}
#endif //SSP_TOOLS_EVENT_LOOP_H
