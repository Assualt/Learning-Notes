#include "RpcServer.h"
#include "base/Logging.h"
using namespace rpc;
using namespace muduo::base;
using namespace muduo::net;
int main(int argc, char const *argv[]) {
    auto &log = Logger::getLogger();
    log.BasicConfig(LogLevel::Info, "T:%(tid)(%(asctime))[%(appname):%(levelname)][%(filename):%(lineno)] %(message)", "", "");
    log.setAppName("app");
    log.addLogHandle(new StdOutLogHandle);

    EventLoop loop;
    RpcServer server(&loop, InetAddress(22087), false);
    server.start();
    loop.loop();
    return 0;
}
