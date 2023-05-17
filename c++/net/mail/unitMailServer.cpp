#include "base/Logging.h"
#include "base/argparse/cmdline.h"
#include "mailserver.h"
#include "net/EventLoop.h"
using namespace muduo::base;
using namespace muduo::net;
using namespace mail;

static std::string defaultMsgFmt = "T:%(tid)(%(asctime))[%(appname):%(levelname)][%(filename):%(lineno)] %(message)";

int main(int argc, char const *argv[]) {
    auto &          log = Logger::getLogger();
    cmdline::parser cmd;
    cmd.add("version", 'v', "show this Simple Mailer Version and exit");
    cmd.add<int>("level", 'l', "set server mail logger level", false, Info);
    cmd.add<int>("threads_count", 'n', "The mail's threads count", false, 3, cmdline::range<int>(1, 10));
    cmd.add<std::string>("config_path", 'c', "The mail's config path", true);
    cmd.add("help", 'h', "The mail's usage help");
    bool ok = cmd.parse(argc, argv);

    auto level       = static_cast<LogLevel>(cmd.get<int>("level"));
    auto plainHandle = std::make_shared<StdOutLogHandle>();
    log.BasicConfig(level, defaultMsgFmt.c_str(), "", "");
    log.setAppName(argv[ 0 ]);
    log.addLogHandle(plainHandle.get());
    if (!ok || cmd.exist("help")) {
        std::cout << cmd.error() << std::endl;
        std::cout << cmd.usage() << std::endl;
        return 0;
    }

    std::string configPath = cmd.get<std::string>("config_path");
    auto        threadNum  = cmd.get<int>("threads_count");
    EventLoop   loop;
    MailServer  server(loop, configPath);
    server.startServer(threadNum);
    loop.loop();
    return 0;
}
