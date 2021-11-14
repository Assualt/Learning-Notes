#include "base/Logging.h"
#include <memory>
using namespace muduo;
using namespace muduo::base;

int main(int argc, char const *argv[])
{

    std::shared_ptr<LogHandle> _au(new StdOutLogHandle);
    std::shared_ptr<LogHandle> _FileLogger(new RollingFileLogHandle("./", "server.log"));
    std::shared_ptr<LogHandle> _FileSizeLogger(new RollingFileSizeLogHandle("./","server.log", 300));
    std::shared_ptr<LogHandle> _File2Logger(new RollingFile2LogHandle("./","serverlog_", "da_"));
    // Log.addLogHandle(_File2Logger.get());
    // Log.addLogHandle(_FileSizeLogger.get());
    // Log.addLogHandle(_FileLogger.get());

    // ax.debug("Hello World [%2.1f] [%s] OK %08s %x", 123.421, "test Fallow", 123.123, 16);

    auto &mainLog = Logger::getLogger();
    mainLog.BasicConfig(Logger::Debug, "T:%(process)[%(asctime):%(levelname)][%(filename)-%(funcname)-%(lineno)] %(message)", "filename", "%Y-%m-%d");
    mainLog.addLogHandle(_au.get());
    mainLog.addLogHandle(_FileLogger.get());

    logger.debug("Hello World [%2.1f] [%s] OK %08s %x", 123.421, "test Fallow", 123.123, 16);
    logger.info("Hello World [%2.1f] [%s] OK %08s %x", 123.421, "test Fallow", 123.123, 16);
    logger.warning("Hello World [%2.1f] [%s] OK %08s %x", 123.421, "test Fallow", 123.123, 16);
    logger.alert("Hello World [%2.1f] [%s] OK %08s %x", 123.421, "test Fallow", 123.123, 16);
    logger.fatal("Hello World [%2.1f] [%s] OK %08s %x", 123.421, "test Fallow", 123.123, 16);
    logger.emergency("Hello World [%2.1f] [%s] OK %08s %x", 123.421, "test Fallow", 123.123, 16);



    return 0;
}
