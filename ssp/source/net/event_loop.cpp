//
// Created by 侯鑫 on 2024/1/3.
//

#include "event_loop.h"

using namespace ssp::net;

void EventLoop::Loop()
{
    while (!quit_) {


        DispatchEvent();
    }
}

void EventLoop::DispatchEvent()
{
}