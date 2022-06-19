#include "base/Logging.h"
#include "base/System.h"
#include "base/ThreadPool.h"
#include <chrono>
#include <iostream>
#include <stdio.h>
#include <thread>
using namespace muduo::base;

void print() {
    printf("tid=print %s\n", System::GetCurrentThreadName().c_str());
}

void printString(const std::string &str) {
    logger.info("==>Start threadName:%s str=%s", System::GetCurrentThreadName().c_str(), str.c_str());
    usleep(100 * 1000);
    logger.info("==>End   threadName:%s str=%s", System::GetCurrentThreadName().c_str(), str.c_str());
}

int main(int argc, char const *argv[]) {
    std::shared_ptr<LogHandle> _au(new StdOutLogHandle);
    auto &                     Log = Logger::getLogger();
    Log.BasicConfig(LogLevel::Debug, "%(levelname) %(asctime)[%(tid)-%(threadname)] %(message)", nullptr, nullptr);
    Log.addLogHandle(_au.get());

    ThreadPool myPool;
    myPool.SetMaxQueueSize(20);
    myPool.Start(10);

    myPool.Run(print);
    myPool.Run(print);

    for (int i = 0; i < 100; ++i) {
        myPool.Run(std::bind(printString, FmtString("Task_%").arg(i).str()));
    }
    return 0;
}
