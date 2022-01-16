#include "base/Logging.h"
#include "base/ThreadPool.h"
#include <chrono>
#include <iostream>
#include <stdio.h>
#include <thread>
using namespace muduo::base;

void print() {
    printf("tid=print");
}

void printString(const std::string &str) {
    printf("str=%s\n", str.c_str());
    usleep(100 * 1000);
}

int main(int argc, char const *argv[]) {

    ThreadPool myPool;
    myPool.setMaxQueueSize(20);
    myPool.start(10);

    return 0;
}
