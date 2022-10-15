#include "base/Logging.h"
#include "gtest/gtest.h"
#include <memory>
using namespace muduo;
using namespace muduo::base;

TEST(Logging, TestSuit1) {
    std::shared_ptr<LogHandle> _au(new StdOutLogHandle);
    std::shared_ptr<LogHandle> fileLogger(new RollingFileLogHandle("./", "server.log"));
    std::shared_ptr<LogHandle> fileSizeLogger(new RollingFileSizeLogHandle("./", "server.log", 300));
    std::shared_ptr<LogHandle> file2Logger(new RollingFile2LogHandle("./", "", "log_"));

    auto &mainLog = Logger::getLogger();
    mainLog.BasicConfig(LogLevel::Debug,
                        "T:%(process)[%(asctime):%(levelname)][%(filename)-%(funcname)-%(lineno)] %(message)",
                        "filename", "%Y-%m-%d");
    mainLog.addLogHandle(_au.get());
    mainLog.addLogHandle(fileLogger.get());
    mainLog.addLogHandle(file2Logger.get());

    logger.debug("Hello World [%2.1f] [%s] OK %08s %x", 123.421, "test Fallow", 123.123, 16);
    logger.info("Hello World [%2.1f] [%s] OK %08s %x", 123.421, "test Fallow", 123.123, 16);
    logger.warning("Hello World [%2.1f] [%s] OK %08s %x", 123.421, "test Fallow", 123.123, 16);
    logger.alert("Hello World [%2.1f] [%s] OK %08s %x", 123.421, "test Fallow", 123.123, 16);
    logger.fatal("Hello World [%2.1f] [%s] OK %08s %x", 123.421, "test Fallow", 123.123, 16);
    logger.emergency("Hello World [%2.1f] [%s] OK %08s %x", 123.421, "test Fallow", 123.123, 16);
}

int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}