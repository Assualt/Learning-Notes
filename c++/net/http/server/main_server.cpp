#include "HttpServer.h"
#include "base/ObjPool.h"
#include "base/argparse/cmdline.h"
#include <signal.h>
using std::string;
using namespace muduo;

void RegisterSignalHandle(HttpServer &server) {
    auto handle = [](int sig, uintptr_t param) {
        if ((sig == SIGSEGV) || (sig == SIGABRT) || (sig == SIGINT) || (sig == SIGHUP)) {
            auto callstack = GetBackCallStack();
            logger.info("callstack is:\n%s", callstack);
            auto server = reinterpret_cast<HttpServer *>(param);
            server->Exit();
            _exit(0);
        }
    };
    server.RegSignalCallback(SIGINT, reinterpret_cast<uintptr_t>(&server), handle);
    server.RegSignalCallback(SIGHUP, reinterpret_cast<uintptr_t>(&server), handle);
    server.RegSignalCallback(SIGSEGV, reinterpret_cast<uintptr_t>(&server), handle);
    server.RegSignalCallback(SIGABRT, reinterpret_cast<uintptr_t>(&server), handle);
    server.RegSignalCallback(SIGPIPE, reinterpret_cast<uintptr_t>(&server), handle);
}

void InitObjPool() {
    ObjPool::Instance().PreInit();
    ObjPool::Instance().PostInit();
}

int main(int argc, char const *argv[]) {
    cmdline::parser cmd;
    cmd.add("version", 'v', "show this HttpServer Version and exit");
    cmd.add<int>("threads_count", 'n', "The http server's threads count", false, 10, cmdline::range<int>(1, 100));
    cmd.add<std::string>("config_path", 'c', "The http server's config path.", true);
    cmd.add<int>("logLevel", 'l', "The http server's logs level.", false, 1, cmdline::range<int>(0, 7));
    cmd.add<bool>("daemon", 'd', "The http server's run in daemon.", false, false);
    cmd.add<std::string>("libpath", 'p', "this controller lib path", true);
    bool ok = cmd.parse(argc, argv);
    if (!ok) {
        std::cout << cmd.error() << std::endl;
        std::cout << cmd.usage() << std::endl;
        return 0;
    }
    bool        runInDaemon   = cmd.get<bool>("daemon");
    auto        nLevel        = cmd.get<int>("logLevel");
    auto        threadNum     = cmd.get<int>("threads_count");
    auto        libPaths      = cmd.get<std::string>("libpath");
    std::string strConfigPath = cmd.get<std::string>("config_path");

    auto &log = Logger::getLogger();
    log.BasicConfig(static_cast<LogLevel>(nLevel),
                    "T:%(tid)(%(asctime))[%(appname):%(levelname)][%(filename):%(lineno)] %(message)", "", "");
    log.setAppName("app");
    auto stdHandle  = std::make_shared<StdOutLogHandle>();
    auto fileHandle = std::make_shared<RollingFileLogHandle>(".", "http_server.log");
    log.addLogHandle(stdHandle.get());
    log.addLogHandle(fileHandle.get());
    InitObjPool();

    EventLoop  loop;
    HttpServer server(&loop, InetAddress(8100), true);
    RegisterSignalHandle(server);
    server.SetThreadNum(threadNum);
    server.Start();
    server.startScannerTask(libPaths);
    loop.loop();

    return 0;
}