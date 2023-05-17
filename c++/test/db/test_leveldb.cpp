#include "base/Logging.h"
#include "db/leveldb_cli.h"
#include <memory>

using namespace muduo::base;
int main(int argc, char const *argv[]) {
    auto &log = Logger::getLogger();
    log.BasicConfig(LogLevel::Debug,
                    "T:%(tid)(%(asctime))[%(appname):%(levelname)][%(filename):%(funcname):%(lineno)] %(message)", "",
                    "");
    log.setAppName("app");
    log.addLogHandle(std::make_shared<StdOutLogHandle>().get());
    db::LevelDBClient client;

    leveldb::Options options;
    options.create_if_missing = true;

    bool ok = client.open("test", options);

    if (!ok)
        return 0;

    client.Put("Hello", "World");
    client.Put("test", "OK");

    logger.info("client Get Hello %s", client.Get("Hello"));

    client.Iterator([](const std::string &key, const std::string &val) { logger.info("Key:%s Val:%s", key, val); });

    logger.info("dbStatus:\n%s", client.getDBStatus());
    client.close();

    /* code */
    return 0;
}
