//
// Created by Sunshine on 2020/8/30.
//
#include <iostream>
#include "base/xstring.h"
#include "system/xlog.h"
#include <thread>
using namespace std;
using namespace hmt;

void func(const std::string& name, int cnt) {
    while (cnt--) {
        logger.info("threadName %s this is %d decrease ", name, cnt);
        usleep(1000);
    }
}


int main(int argc, char** argv) {
    Logger stdOutLogger("main");
    stdOutLogger.setLevel(detail::INFO);
    stdOutLogger.BasicConfig("T:(%(thread)s) %(asctime)s [%(threadname)s-%(levelname)s] [%(filename)s:%(funcname)s:%(lineno)d] %(message)s", "", "%04Y-%02m-%02d");
    logImpl::AppendLogger("main", &stdOutLogger);
//    std::thread t1(func, "thread1", 100), t2(func, "thread2", 100),t3(func, "thread3", 100),t4(func, "thread4", 100);
//    t1.join();t2.join();t3.join();t4.join();
    logger.debug("Hello World %s", 'a');
    logger.info("Hello World %04d ", 123);
    logger.warning("Hello World %4.3d ", 123);
    logger.error("Hello World %04.2f ", 123.31);
    logger.alert("Hello World %04.2f ", 123.3131);
    logger.emergency("Hello World %04.2f ", 123.0);

    LOG(INFO);

    return 0;
}