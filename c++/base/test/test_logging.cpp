#include "Logging.h"
#include <memory>
#include "LogHandle.h"
using namespace muduo;
using namespace muduo::base;

int main(int argc, char const *argv[])
{

    auto &Log = Logger::getLogger(__FILE__);
    Log.BasicConfig(Logger::Debug, "T:%(process)[%(asctime):%(levelname)][%(filename)-%(funcname)-%(lineno)] %(message)", "filename", "%Y-%m-%d");


    std::shared_ptr<LogHandle> _au(new StdOutLogHandle);
    std::shared_ptr<LogHandle> _FileLogger(new RollingFileLogHandle("./", "server.log"));
    std::shared_ptr<LogHandle> _FileSizeLogger(new RollingFileSizeLogHandle("./","server.log", 300));
    std::shared_ptr<LogHandle> _File2Logger(new RollingFile2LogHandle("./","serverlog_", "da_"));
    Log.addLogHandle(_au.get());
    // Log.addLogHandle(_File2Logger.get());
    // Log.addLogHandle(_FileSizeLogger.get());
    // Log.addLogHandle(_FileLogger.get());

    // ax.debug("Hello World [%2.1f] [%s] OK %08s %x", 123.421, "test Fallow", 123.123, 16);

    auto &mainLog = Logger::getLogger("MAIN");
    mainLog.BasicConfig(Logger::Debug, "T:%(process)[%(asctime):%(levelname)][%(filename)-%(funcname)-%(lineno)] %(message)", "filename", "%Y-%m-%d");
    mainLog.addLogHandle(_au.get());

    logger(__FILE__).debug("Hello World [%2.1f] [%s] OK %08s %x", 123.421, "test Fallow", 123.123, 16);
    logger(__FILE__).info("Hello World [%2.1f] [%s] OK %08s %x", 123.421, "test Fallow", 123.123, 16);
    logger(__FILE__).warning("Hello World [%2.1f] [%s] OK %08s %x", 123.421, "test Fallow", 123.123, 16);
    logger(__FILE__).alert("Hello World [%2.1f] [%s] OK %08s %x", 123.421, "test Fallow", 123.123, 16);
    logger(__FILE__).fatal("Hello World [%2.1f] [%s] OK %08s %x", 123.421, "test Fallow", 123.123, 16);
    logger(__FILE__).emergency("Hello World [%2.1f] [%s] OK %08s %x", 123.421, "test Fallow", 123.123, 16);


    // LOG_INFO << "Hello wolrd" << std::flush;
    int a = 123;
    LOG_INFO << "Hello Wolrd" << a << "Test OK\n";

    

    return 0;
}
