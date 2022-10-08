//
// Created by xhou on 2022/10/5.
//

#include "base/Logging.h"
#include "db/redis_cli.h"
using namespace muduo::base;

int main(int, char **) {
    auto &log       = Logger::getLogger();
    auto  stdHandle = std::make_shared<StdOutLogHandle>();
    log.BasicConfig(LogLevel::Debug,
                    "T:%(tid)(%(asctime))[%(appname):%(levelname)][%(filename):%(funcname):%(lineno)] %(message)", "",
                    "");
    log.setAppName("app");
    log.addLogHandle(stdHandle.get());

    RedisClient client("127.0.0.1", 6379);

    logger.info("begin to connect 127.0.0.1:6379 connected:%b", client.isConnected());

    logger.info("-------get-key---------");
    auto val = client.get("key");
    logger.info("get key:\"key\" ==> val:\"%s\"", val);

    logger.info("--------keys---------");
    auto keys = client.keys();
    std::for_each(keys.begin(), keys.end(), [](auto &key) { logger.info("keys ==>%s", key); });

    logger.info("-------set-key-test_OK---------");
    client.set("key", "test_OK");
    logger.info("get key:\"key\" ==> val:\"%s\"", client.get("key"));

    logger.info("-------expire-key_1---------");
    client.set("key_1", "123456");
    client.expire("key_1", 5);
    logger.info("get key:\"key_1\" ==> val:\"%s\"", client.get("key_1"));

    logger.info("-------sleep-key_1---------");
    for (auto i = 0; i < 7; i++) {
        sleep(1);
        logger.info("get key:\"key_1\" ==> val:\"%s\"", client.get("key_1"));
    }

    logger.info("-------del-key---------");
    client.del("key");
    logger.info("get key:\"key\" ==> val:\"%s\"", client.get("key"));

    logger.info("-------incr-key---------");
    client.set("key_2", "3");
    logger.info("get key:\"key_2\" ==> val:\"%s\"", client.get("key_2"));
    client.incr("key_2");
    logger.info("get key:\"key_2\" ==> val:\"%s\"", client.get("key_2"));
    client.del("key_2");

    logger.info("-------exists-key---------");
    client.set("key_3", "5");
    logger.info("exists key \"key_3\":%b", client.exists("key_3"));
    client.del("key_3");
    logger.info("after del exists key \"key_3\":%b", client.exists("key_3"));

    logger.info("-------type-key---------");
    client.set("key_4", "5");
    logger.info("type key \"key_4\":%s", client.type("key_4"));
    logger.info("type for not exists key \"key_5\":%s", client.type("key_5"));

    logger.info("-------lpush-key---------");
    client.lpush("key_5", 123);
    client.lpush("key_5", 124);
    client.lpush("key_5", 125);
    auto result = client.lrange("key_5");
    std::for_each(result.begin(), result.end(), [](auto item) { logger.info("item => %s", item); });
    logger.info("-------lpop-key---------");
    client.lpop("key_5");
    result = client.lrange("key_5");
    std::for_each(result.begin(), result.end(), [](auto item) { logger.info("item => %s", item); });
    logger.info("-------lpop-key---------");
    client.lpop("key_5");
    result = client.lrange("key_5");
    std::for_each(result.begin(), result.end(), [](auto item) { logger.info("item => %s", item); });
    logger.info("-------lpop-key---------");
    client.lpop("key_5");
    result = client.lrange("key_5");
    std::for_each(result.begin(), result.end(), [](auto item) { logger.info("item => %s", item); });

    logger.info("-------rpush-key---------");
    client.rpush("key_5", 123);
    client.rpush("key_5", 124);
    client.rpush("key_5", 125);
    logger.info("length of list is: %d", client.llen("key_5"));
    result = client.lrange("key_5");
    std::for_each(result.begin(), result.end(), [](auto item) { logger.info("item => %s", item); });
    logger.info("-------rpop-key---------");
    client.rpop("key_5");
    result = client.lrange("key_5");
    std::for_each(result.begin(), result.end(), [](auto item) { logger.info("item => %s", item); });
    logger.info("-------rpop-key---------");
    client.rpop("key_5");
    result = client.lrange("key_5");
    std::for_each(result.begin(), result.end(), [](auto item) { logger.info("item => %s", item); });
    logger.info("-------rpop-key---------");
    client.rpop("key_5");
    result = client.lrange("key_5");
    std::for_each(result.begin(), result.end(), [](auto item) { logger.info("item => %s", item); });


    return 0;
}