#include "RpcServer.h"
#include "base/Logging.h"
#include <signal.h>
using namespace rpc;
using namespace muduo::base;
using namespace muduo::net;

void handle_pipe(int sig) {}

void RegisterSignalAction() {
    struct sigaction sa;
    sa.sa_handler = handle_pipe;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGPIPE, &sa, nullptr);
}

int main(int, char const **) {
    auto &log = Logger::getLogger();
    log.BasicConfig(LogLevel::Info, "T:%(tid)(%(asctime))[%(appname):%(levelname)][%(filename):%(lineno)] %(message)",
                    "", "");
    log.setAppName("app");
    log.addLogHandle(new StdOutLogHandle);

    RegisterSignalAction();

    EventLoop loop;
    RpcServer server(&loop, InetAddress(22087), true);
    server.initEx(10);
    server.start();
    loop.loop();
    return 0;
}
