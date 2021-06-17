#include "cmdline.hpp"
#include "log.h"
#include "mailenv.h"
#include "mailserver.h"
#include <signal.h>
void handle(int sig) {
    if(sig == SIGINT) {
        logger.info("recved signal sigint. begin to run shutdown httpserver now.");
    }
    exit(0);
}
int main(int argc, char const *argv[]) {

    // logger.BasicConfig("T:%(thread)s(%(ctime)s)[%(user)s-%(appname)s-%(levelname)s] [%(filename)s-%(lineno)s-%(funcname)s] %(message)s", "mailserver.log");
    logger.BasicConfig("T:%(thread)s(%(ctime)s)[%(user)s-%(appname)s-%(levelname)s]%(message)s", "mailserver.log");

    cmdline::parser CommandParse;
    CommandParse.add("version", 'v', "show this Simple Mailer Version and exit");
    CommandParse.add("help", 'h', "show this Simple Mailer's Usage and exit");
    CommandParse.add("level",'l', "set logger's level", false, 0);
    CommandParse.add<int>("threads_count", 'n', "The Mail's threads count", false, 3, cmdline::range<int>(1, 10));
    CommandParse.add<std::string>("config_path", 'c', "The Mail's config path", true);
    
    bool ok = CommandParse.parse(argc, argv);

    if (!ok) {
        std::cout << CommandParse.error() << std::endl;
        std::cout << CommandParse.usage() << std::endl;
        return 0;
    }
    if (CommandParse.exist("help")) {
        std::cout << CommandParse.usage() << std::endl;
        return 0;
    }
    if (CommandParse.exist("version")) {
        std::cout << "v1.0.0" << std::endl;
        return 0;
    }
    if(CommandParse.exist("level")) {
        int nLevel = CommandParse.get<int>("level");
        logger.setLevel(static_cast<tlog::Level>(nLevel));
    }

    std::string configPath = CommandParse.get<std::string>("config_path");
    signal(SIGINT, handle);
    mail::MailServer mailServer;
    mailServer.initEx(configPath);
    mailServer.startServer();
    return 0;
}
