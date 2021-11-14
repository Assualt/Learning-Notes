//
// Created by Sunshine on 2021/4/17.
//
#include "base/Logging.h"
#include "net/EventLoop.h"
#include <iostream>
using namespace std;
using namespace muduo;
using namespace muduo::net;
using namespace muduo::base;

int main(int argc, char **argv) {

    auto &log = Logger::getLogger();
    log.BasicConfig(Logger::Debug, "T:%(process)[%(asctime):%(levelname)][%(filename):%(lineno)-%(funcname)] %(message)", "", "");
    std::shared_ptr<LogHandle> _au(new StdOutLogHandle);
    log.addLogHandle(_au.get());

    EventLoop loop;

    loop.loop();

    return 0;
}