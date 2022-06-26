#include "HttpServer.h"
#include "base/ObjPool.h"
#include "base/argparse/cmdline.h"
#include <signal.h>
using std::string;
using namespace muduo;

void RegisterSignalHandle(HttpServer &server) {
    auto handle = [](int sig, uintptr_t param) {
        if ((sig == SIGSEGV) || (sig == SIGABRT)) {
            auto callstack = GetBackCallStack();
            logger.info("callstack is:\n%s", callstack);
        }
        auto server = reinterpret_cast<HttpServer *>(param);
        server->Exit();
        _exit(0);
    };
    server.RegSignalCallback(SIGINT, reinterpret_cast<uintptr_t>(&server), handle);
    server.RegSignalCallback(SIGHUP, reinterpret_cast<uintptr_t>(&server), handle);
    server.RegSignalCallback(SIGSEGV, reinterpret_cast<uintptr_t>(&server), handle);
    server.RegSignalCallback(SIGABRT, reinterpret_cast<uintptr_t>(&server), handle);
}

void InitObjPool() {
    ObjPool::Instance().PreInit();
    ObjPool::Instance().PostInit();
}

int main(int argc, char const *argv[]) {
    cmdline::parser CommandParse;
    CommandParse.add("version", 'v', "show this HttpServer Version and exit");
    CommandParse.add<int>("threads_count", 'n', "The http server's threads count", false, 10, cmdline::range<int>(1, 100));
    CommandParse.add<std::string>("config_path", 'c', "The http server's config path.", true);
    CommandParse.add<int>("logLevel", 'l', "The http server's logs level.", false, 1, cmdline::range<int>(0, 7));
    CommandParse.add<bool>("daemon", 'd', "The http server's run in daemon.", false, false);
    bool ok = CommandParse.parse(argc, argv);
    if (!ok) {
        std::cout << CommandParse.error() << std::endl;
        std::cout << CommandParse.usage() << std::endl;
        return 0;
    }

    std::string strConfigPath = CommandParse.get<std::string>("config_path");
    bool        RunInDaemon   = CommandParse.get<bool>("daemon");
    auto        nLevel        = CommandParse.get<int>("logLevel");
    auto        threadNum     = CommandParse.get<int>("threads_count");

    auto &log = Logger::getLogger();
    log.BasicConfig(static_cast<LogLevel>(nLevel), "T:%(tid)(%(asctime))[%(appname):%(levelname)][%(filename):%(lineno)] %(message)", "", "");
    log.setAppName("app");
    auto stdHandle  = std::make_shared<StdOutLogHandle>();
    auto fileHandle = std::make_shared<RollingFileLogHandle>(".", "http_server.log");
    log.addLogHandle(stdHandle.get());
    log.addLogHandle(fileHandle.get());
    InitObjPool();

    EventLoop  loop;
    HttpServer server(&loop, InetAddress(8100));
    RegisterSignalHandle(server);
    server.SetThreadNum(threadNum);
    server.Start();
    loop.loop();

    return 0;
}