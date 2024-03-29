#pragma once
#include "base/json/json.h"
#include <functional>
#include <iostream>
#include <mariadb/mysql.h>
#include <memory>

namespace db {

enum SqlError {
    MYSQL_SUCCESS,
    MYSQL_CONNECT_FAIL,
    MYSQL_CONNECT_AUTH_FAIL,
    MYSQL_CONNECT_INNER_ERROR,
    MYSQL_QUERY_NULL_TABLE,
    MYSQL_QUERY_FETCH_FAIL,
    MYSQL_QUERY_FAIL,
    MYSQL_QUERY_NULL_FIELD,
    MYSQL_PING_FAIL,
    MYSQL_DB_NOT_FOUND,
    MYSQL_CREATE_DB_FAILED,
    MYSQL_CREATE_TABLE_FAILED,
    MYSQL_SWITCH_DB_FAIL,
    MYSQL_COMMIT_FAIL,
    MYSQL_ROLLBACK_FAIL,
};

using QueryCallback = std::function<void(const json::Json &)>;
using FieldInfo     = std::vector<std::pair<std::string, enum_field_types>>;
class MysqlClient {
public:
    explicit MysqlClient(const std::string &dbName, const std::string &host = "127.0.0.1", size_t port = 3306,
                         const std::string &user = "root", const std::string &pass = "",
                         const std::string &charset = "utf8");
    ~MysqlClient();

public:
    SqlError Ping();

    SqlError connect();

    void close();

    std::string getErrMsg() const { return m_strErrMsg; }

    std::pair<SqlError, json::Json> Query(const std::string &strSql);

    void QueryCallback(const std::string &strSql, QueryCallback cb);

    std::pair<SqlError, long long> Execute(const std::string &strSql);

    std::pair<SqlError, json::Json> ShowTables();

    std::pair<SqlError, json::Json> ShowTable(const std::string &tableName);

    SqlError SwitchBD(const std::string &dbName);

    SqlError CommitTransaction();

    SqlError Rollback();

    SqlError CreateTable(const std::string &strSql);

    SqlError NewDB(const std::string &dbName);

private:
    json::Json translateSqlResultToJson(MYSQL_RES &res, const FieldInfo &fields);

private:
    std::string m_strHost{"127.0.0.1"};
    size_t      m_nPort{3306};
    std::string m_strUser{"root"};
    std::string m_strPassWord;
    std::string m_strDefaultCharSet{"UTF-8"};
    std::string m_strDatabaseName;

    MYSQL *     driver_{nullptr};
    std::string m_strErrMsg;
    bool        m_bIsConnected{false};
};

} // namespace db