#include "cmdline.hpp"
#include "configure.hpp"
int main(int argc, char const *argv[]) {

    cmdline::parser cmdparse;
    cmdparse.add<std::string>("conf_path", 'c', "config path", true);

    bool bOk = cmdparse.parse(argc, argv);
    if (!bOk) {
        std::cout << cmdparse.error() << std::endl;
        std::cout << cmdparse.usage() << std::endl;
    } else {
        std::string confstr = cmdparse.get<std::string>("conf_path");

        conf::ConfigureManager mgr(confstr, ".cf");

        mgr.init();

        logger.info("key:/test/datasource/database val: %s", mgr.getString("redis_1", "/test/datasource/database"));
        logger.info("key:/test/datasource/database val: %s", mgr.getString("redis", "/test/datasource/database1"));

        logger.info("key:/test/redis_server val: %s", mgr.getString("redis", "/test/redis_server"));
        logger.info("key:/test/datasource/testOK val: %s", mgr.getBool("redis", "/test/dataresource/testOK"));

        mgr.changeAccessPath("/test/datasource/");
        logger.info("key:/test/datasource/database %s", mgr.getString("default_1", "database"));
        logger.info("key:/test/datasource/port %s", mgr.getInt(123456, "port"));
    }
    return 0;
}
