//
// Created by 侯鑫 on 2024/1/1.
//

#include <gtest/gtest.h>
#include "base/thread.h"
#include "base/log.h"
#include "base/system.h"

using namespace ssp::base;

void my_thread_func(uintptr_t)
{
    for (uint32_t idx = 0; idx < 30; ++idx) {
        usleep(1000 * 100);
    }
}

TEST(thread, start_thread)
{
    System::SetThreadName("main");
    ThreadContext::SetThreadStateChangeHook([](auto old, auto newState) {
        logger.Info("state change:%d --> %d", old, newState);
    });

    std::shared_ptr<LogImpl> _au(new StdoutLog);
    auto &mainLog = Logger::GetLogger();
    mainLog.BasicConfig(LogLevel::Info,
                        "T:[%(appname)-%(threadName)][%(asctime):%(levelname)][%(filename)-%(lineno)] %(message)",
                        "filename", "%Y-%m-%d");
    mainLog.AddLogHandle(_au.get());

    Thread tmp(my_thread_func, "TestName");

    logger.Info("state is ==> %d", tmp.GetState());
    tmp.Start();

    sleep(1);
    tmp.Join();
    logger.Info("thread exit....");
}