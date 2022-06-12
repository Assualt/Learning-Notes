#include "RpcServer.h"
#include "base/Logging.h"
using namespace rpc;
using namespace muduo::base;
using namespace muduo::net;
int main(int argc, char const *argv[]) {
    auto &log = Logger::getLogger();
    log.BasicConfig(Logger::Info, "T:%(tid)(%(asctime))[%(appname):%(levelname)][%(filename):%(lineno)] %(message)", "", "");
    log.setAppName("app");
    log.addLogHandle(new StdOutLogHandle);

    EventLoop loop;
    RpcServer server(&loop, InetAddress(8000));
    server.Start();
    loop.loop();
    return 0;
}
