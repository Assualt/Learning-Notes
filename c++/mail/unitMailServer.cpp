#include "cmdline.hpp"
#include "logging.h"
#include "mailserver.h"
int main(int argc, char const *argv[]) {

    logger.BasicConfig("%(process)s %(levelname)s %(ctime)s [%(filename)s-%(lineno)s-%(funcName)s] %(message)s", "%Y-%m-%d %H:%M:%S,%s", "", "a");

    cmdline::parser CommandParse;
    CommandParse.add("version", 'v', "show this Simple Mailer Version and exit");
    CommandParse.add<int>("threads_count", 'n', "The Mail's threads count", false, 3, cmdline::range<int>(1, 10));
    CommandParse.add<std::string>("config_path", 'c', "The Mail's config path", true);
    bool ok = CommandParse.parse(argc, argv);

    if (!ok) {
        std::cout << CommandParse.error() << std::endl;
        std::cout << CommandParse.usage() << std::endl;
        return 0;
    }

    std::string configPath = CommandParse.get<std::string>("config_path");

    mail::MailServer mailServer;
    mailServer.initEx(configPath);
    mailServer.startServer();
    return 0;
}
