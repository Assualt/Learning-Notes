//
// Created by 侯鑫 on 2023/12/26.
//

#include "base/log.h"
#include "gtest/gtest.h"
#include "base/timestamp.h"
#include <filesystem>
#include <memory>

using namespace ssp::base;
namespace fs = std::filesystem;
TEST(Logging, TestSuite) {
    std::shared_ptr<LogImpl> _au(new StdoutLog);
    auto &mainLog = Logger::GetLogger();
    mainLog.BasicConfig(LogLevel::Info,
                        "T:[%(appname)-%(process)][%(asctime):%(levelname)][%(filename)-%(funcname)-%(lineno)] %(message)",
                        "filename", "%Y-%m-%d");
    mainLog.AddLogHandle(_au.get());

    logger.Debug("Hello World [%2.1f] [%s] OK %08s %x", 123.421, "test Fallow", 123.123, 16);
    logger.Info("Hello World [%2.1f] [%s] OK %08s %x", 123.421, "test Fallow", 123.123, 16);
    logger.Warning("Hello World [%2.1f] [%s] OK %08s %x", 123.421, "test Fallow", 123.123, 16);
    logger.Error("Hello World [%2.1f] [%s] OK %08s %x", 123.421, "test Fallow", 123.123, 16);
    logger.Alert("Hello World [%2.1f] [%s] OK %08s %x", 123.421, "test Fallow", 123.123, 16);
    logger.Fatal("Hello World [%2.1f] [%s] OK %08s %x", 123.421, "test Fallow", 123.123, 16);
    logger.Emergency("Hello World [%2.1f] [%s] OK %08s %x", 123.421, "test Fallow", 123.123, 16);
    logger.Exception("Hello World [%2.1f] [%s] OK %08s %x", 123.421, "test Fallow", 123.123, 16);
}

TEST(Logging, log_with_file)
{
    return;
    auto fileLogger = std::make_shared<RollingFileLog>(".", "file_log");
    auto &mainLog = Logger::GetLogger();
    mainLog.BasicConfig(LogLevel::Info,
                        "T:[%(appname)-%(process)][%(asctime):%(levelname)][%(filename)-%(funcname)-%(lineno)] %(message)",
                        "filename", "%Y-%m-%d");
    mainLog.AddLogHandle(fileLogger.get());

    logger.Debug("Hello World [%2.1f] [%s] OK %08s %x", 123.421, "test Fallow", 123.123, 16);
    logger.Info("Hello World [%2.1f] [%s] OK %08s %x", 123.421, "test Fallow", 123.123, 16);
    logger.Warning("Hello World [%2.1f] [%s] OK %08s %x", 123.421, "test Fallow", 123.123, 16);
    logger.Error("Hello World [%2.1f] [%s] OK %08s %x", 123.421, "test Fallow", 123.123, 16);
    logger.Alert("Hello World [%2.1f] [%s] OK %08s %x", 123.421, "test Fallow", 123.123, 16);
    logger.Fatal("Hello World [%2.1f] [%s] OK %08s %x", 123.421, "test Fallow", 123.123, 16);
    logger.Emergency("Hello World [%2.1f] [%s] OK %08s %x", 123.421, "test Fallow", 123.123, 16);
    logger.Exception("Hello World [%2.1f] [%s] OK %08s %x", 123.421, "test Fallow", 123.123, 16);


    auto fileName = "file_log.log." + TimeStamp::Now().ToFmtString("%Y-%m-%d");
    EXPECT_TRUE(fs::exists(fileName));
}
