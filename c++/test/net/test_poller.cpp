//
// Created by Sunshine on 2021/4/17.
//
#include "base/Logging.h"
#include "net/EventLoop.h"
#include "gtest/gtest.h"
#include <iostream>
using namespace std;
using namespace muduo::net;
using namespace muduo::base;

void TestLoop() {
    auto &log = Logger::getLogger();
    log.BasicConfig(Debug, "T:%(process)[%(asctime):%(levelname)][%(filename):%(lineno)-%(funcname)] %(message)", "",
                    "");
    std::shared_ptr<LogHandle> _au(new StdOutLogHandle);
    log.addLogHandle(_au.get());

    EventLoop loop;

    loop.loop();

    sleep(5);
    loop.quit();
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}