//
// Created by xhou on 2022/8/7.
//

#include "base/Logging.h"
#include "db/mysql_cli.h"
using namespace muduo::base;
using namespace db;

int main(int, char **) {
    auto &log       = Logger::getLogger();
    auto  stdHandle = std::make_shared<StdOutLogHandle>();
    log.BasicConfig(LogLevel::Debug, "T:%(tid)(%(asctime))[%(appname):%(levelname)][%(filename):%(funcname):%(lineno)] %(message)", "", "");
    log.setAppName("app");
    log.addLogHandle(stdHandle.get());

    MysqlClient client("test", "localhost", 3306, "root,", "123456", "utf8");
    logger.info("-----test connection--------");
    auto ret = client.connect();
    logger.info("connect result %d errmsg:%s", ret, client.getErrMsg());
    if (ret != MYSQL_SUCCESS) {
        logger.info("connect Ret is => %d", ret);
        if (ret == db::MYSQL_DB_NOT_FOUND) {
            logger.info("-----test create database--------");
            ret = client.NewDB("test");
            if (ret != MYSQL_SUCCESS) {
                logger.info("create db for test failed ret:%d", ret);
                return 0;
            }
            logger.info("create db for %s success", "test");

            logger.info("-----test create database table--------");
            ret = client.CreateTable("create table test_mysql ("
                                     "userid int primary key not null AUTO_INCREMENT,"
                                     "username varchar(20) default '',"
                                     "password varchar(20) default '',"
                                     "modifytime datetime,"
                                     "price float default 0.0"
                                     ")");
            logger.info("create table for %s ret:%d", "test", ret);
        }
    }

    logger.info("-----test database ping--------");
    bool pingRet = client.Ping();
    logger.info("ping Ret is => %b", pingRet == MYSQL_SUCCESS);

    logger.info("-----test database insert--------");
    for (auto idx : {0, 1, 2, 3, 4}) {
        std::string name    = "idx-" + std::to_string(idx);
        auto        execRet = client.Execute("insert test_mysql(username,password,modifytime) values('" + name + "', '123', now())");
        logger.info("exec result:%d, affect:%d errmsg:%s", execRet.first, execRet.second, client.getErrMsg());
        sleep(1);
    }

    logger.info("-----test database query--------");
    auto result = client.Query("select * from test_mysql");
    logger.info("result:%d, errmsg:%s", result.first, client.getErrMsg());

    if (result.first == SqlError::MYSQL_SUCCESS) {
        for (auto &item : result.second.array_items()) {
            logger.info("str: %s", item.dump());
        }
    }

    logger.info("-----test database delete--------");
    {
        auto execRet = client.Execute("delete from test_mysql where username = 'idx-1'");
        logger.info("exec result:%d, affect:%d errmsg:%s", execRet.first, execRet.second, client.getErrMsg());

        result = client.Query("select * from test_mysql");
        logger.info("result:%d, errmsg:%s", result.first, client.getErrMsg());

        if (result.first == SqlError::MYSQL_SUCCESS) {
            for (auto &item : result.second.array_items()) {
                logger.info("str: %s", item.dump());
            }
        }
    }
    logger.info("-----test database update--------");
    {

        auto execRet = client.Execute("update test_mysql set price=256.6 where username = 'idx-0'");
        logger.info("exec result:%d, affect:%d errmsg:%s", execRet.first, execRet.second, client.getErrMsg());

        result = client.Query("select * from test_mysql");
        logger.info("result:%d, errmsg:%s", result.first, client.getErrMsg());

        if (result.first == SqlError::MYSQL_SUCCESS) {
            for (auto &item : result.second.array_items()) {
                logger.info("str: %s", item.dump());
            }
        }
    }
    {
        logger.info("-----test show database--------");
        auto [ errCode, tables ] = client.ShowTables();
        if (errCode == MYSQL_SUCCESS) {
            logger.info("table Info: %s", tables.dump());
        }
    }

    {
        logger.info("-----test show database table desc--------");
        auto [ errCode, desc ] = client.ShowTable("test_mysql");
        if (errCode == MYSQL_SUCCESS) {
            for (auto &item : desc.array_items()) {
                logger.info("table Info: %s", item.dump());
            }
        }
    }
    {
        logger.info("-----test database switch db--------");
        auto retCode = client.SwitchBD("mysql");
        logger.info("switch database to mysql ret:%d", retCode);
        logger.info("-----test drop database--------");
        auto [ err, affectNum ] = client.Execute("drop database test");
        logger.info("drop database ret:%d affectNum:%d", err, affectNum);
    }
    return 0;
}