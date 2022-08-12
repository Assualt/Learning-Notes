#pragma once
#include "base/json/json.h"
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
    MYSQL_SWITCH_DB_FAIL,
    MYSQL_COMMIT_FAIL,
    MYSQL_ROLLBACK_FAIL,
};

class MysqlClient {
public:
    explicit MysqlClient(const std::string &dbName, const std::string &host = "127.0.0.1", size_t port = 3306, const std::string &user = "root", const std::string &pass = "",
                         const std::string &charset = "utf8");
    ~MysqlClient();

public:
    SqlError    Ping();
    SqlError    connect();
    void        close();
    std::string getErrMsg() const {
        return m_strErrMsg;
    }
    std::pair<SqlError, json::Json> Query(const std::string &strSql);
    std::pair<SqlError, int>        Execute(const std::string &mysql);
    void                            ShowTables();
    SqlError                        SwitchBD(const std::string &dbName);
    SqlError                        CommitTransaction();
    SqlError                        Rollback();

private:
    std::string m_strHost{"127.0.0.1"};
    size_t      m_nPort{3306};
    std::string m_strUser{"root"};
    std::string m_strPassWord;
    std::string m_strDefaultCharSet{"UTF-8"};
    std::string m_strDatabaseName;

    MYSQL      *driver_{nullptr};
    std::string m_strErrMsg;
};

} // namespace db