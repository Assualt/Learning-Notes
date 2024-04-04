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

TEST(ThreadPool, push_mission_with_wait_task_exit)
{
    System::SetThreadName("main");
    std::shared_ptr<LogImpl> _au(new StdoutLog);
    auto &                     Log = Logger::GetLogger();
    Log.BasicConfig(LogLevel::Debug, "%(levelname) %(threadName) %(asctime)[%(tid)] %(message)", nullptr, nullptr);
    Log.AddLogHandle(_au.get());

    ThreadPool myPool("test_thread_pool");
    myPool.SetMaxQueueSize(20);
    myPool.SetThreadInitCallBack(thread_pool_callback);
    myPool.Start(2);

    for (size_t idx = 0; idx < 100; ++idx) {
        myPool.Run([](auto) {
            usleep(10 * 1000);
        });
    }

    sleep(5); // 等待任务有足够多时间完成
    logger.Info("main thread begin to exit...");
    myPool.Stop();
    logger.Info("main thread begin to exit ..1...");
}
