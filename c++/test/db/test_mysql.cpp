//
// Created by xhou on 2022/8/7.
//

#include "db/mysql_client.h"
#include "base/Logging.h"
using namespace muduo::base;
using namespace db;

int main(int argc, char **argv) {
    auto &log = Logger::getLogger();
    auto stdHandle = std::make_shared<StdOutLogHandle>();
    log.BasicConfig(LogLevel::Debug, "T:%(tid)(%(asctime))[%(appname):%(levelname)][%(filename):%(funcname):%(lineno)] %(message)", "", "");
    log.setAppName("app");
    log.addLogHandle(stdHandle.get());

    MysqlClient client("test", "localhost", 3306, "root,", "123456","utf8");

    bool ret = client.connect();
    logger.info("connect result %b errmsg:%s", ret, client.getErrMsg());
    if (ret != MYSQL_SUCCESS) {
        logger.info("connect Ret is => %b", ret);
        return 0;
    }

    bool pingRet = client.Ping();
    logger.info("ping Ret is => %b", pingRet == MYSQL_SUCCESS);

    auto execRet = client.Execute("insert test_mysql(username,password,time) values('123', '123', now())");
    logger.info("exec result:%d, affect:%d errmsg:%s", execRet.first, execRet.second, client.getErrMsg()) ;

    auto result = client.Query("select * from test_mysql");
    logger.info("result:%d, errmsg:%s", result.first, client.getErrMsg());

    if (result.first == SqlError::MYSQL_SUCCESS) {
        for (auto &item : result.second.array_items()) {
            logger.info("str: %s", item.dump());
        }
    }

    execRet = client.Execute("delete from test_mysql where username = '123'");
    logger.info("exec result:%d, affect:%d errmsg:%s", execRet.first, execRet.second, client.getErrMsg()) ;

    result = client.Query("select * from test_mysql");
    logger.info("result:%d, errmsg:%s", result.first, client.getErrMsg());

    if (result.first == SqlError::MYSQL_SUCCESS) {
        for (auto &item : result.second.array_items()) {
            logger.info("str: %s", item.dump());
        }
    }


    return 0;
}