//
// Created by 侯鑫 on 2024/1/2.
//

//
// Created by 侯鑫 on 2024/1/1.
//

#include <gtest/gtest.h>
#include "base/thread_pool.h"
#include "base/log.h"
#include "base/system.h"

using namespace ssp::base;

void thread_pool_callback(uintptr_t)
{
    for (size_t idx = 0; idx < 100; ++idx) {
        logger.Info("%d run thread init call back..... ", idx);
    }
}

TEST(ThreadPool, push_mission_and_stop_now)
{
    System::SetThreadName("main");
    std::shared_ptr<LogImpl> _au(new StdoutLog);
    auto &                     Log = Logger::GetLogger();
    Log.BasicConfig(LogLevel::Debug, "%(levelname) %(threadName) %(asctime)[%(tid)-%(threadname)] %(message)", nullptr, nullptr);
    Log.AddLogHandle(_au.get());

    ThreadPool myPool("test_thread_pool");
    myPool.SetMaxQueueSize(20);

    myPool.Start(10, "user-defined");

    for (size_t idx = 0; idx < 100; ++idx) {
        myPool.Run([](auto) {
            usleep(1000 * 10);
        });
    }
    myPool.Dump(std::cout);
    std::cout.flush();

    logger.Info("main thread begin to exit...");
    myPool.Stop();
    logger.Info("main thread begin to exit ..1...");
}