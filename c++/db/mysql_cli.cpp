//
// Created by xhou on 2022/8/7.
//

#include "base/Logging.h"
#include "base/Timestamp.h"
#include "mysql_cli.h"
#include <vector>

using namespace db;
using namespace muduo::base;

MysqlClient::MysqlClient(const std::string &dbName, const std::string &host, size_t port, const std::string &user,
                         const std::string &pass, const std::string &charset)
    : m_strHost(host)
    , m_nPort(port)
    , m_strUser(user)
    , m_strPassWord(pass)
    , m_strDefaultCharSet(charset)
    , m_strDatabaseName(dbName) {
    driver_ = new MYSQL;

    driver_ = mysql_init(driver_);
    if (driver_ == nullptr) {
        logger.warning("mysql_init failed");
    }
}

SqlError MysqlClient::Ping() {
    auto ret = mysql_ping(driver_);
    if (ret != MYSQL_SUCCESS) {
        m_strErrMsg = mysql_error(driver_);
    }
    return ret ? MYSQL_PING_FAIL : MYSQL_SUCCESS;
}

SqlError MysqlClient::connect() {
    if (!m_strDefaultCharSet.empty()) {
        mysql_options(driver_, MYSQL_SET_CHARSET_NAME, m_strDefaultCharSet.c_str());
    }

    if (mysql_real_connect(driver_, m_strHost.c_str(), m_strUser.c_str(), m_strPassWord.c_str(),
                           m_strDatabaseName.c_str(), m_nPort, nullptr, 0) == nullptr) {
        logger.info("connect with %s:%d use user:%s passwd:%s database:%s failed with err:%d", m_strHost, m_nPort,
                    m_strUser, m_strPassWord, m_strDatabaseName, mysql_errno(driver_));
        m_strErrMsg = mysql_error(driver_);
        if (mysql_errno(driver_) == 1049) {
            return MYSQL_DB_NOT_FOUND;
        }
        return MYSQL_CONNECT_FAIL;
    }

    m_strErrMsg    = "Success";
    m_bIsConnected = true;
    return MYSQL_SUCCESS;
}

SqlError MysqlClient::CreateTable(const std::string &strSql) {
    auto [ ret, affectNum ] = Execute(strSql);
    if (ret != MYSQL_SUCCESS) {
        m_strErrMsg = mysql_error(driver_);
        return MYSQL_CREATE_TABLE_FAILED;
    }

    return MYSQL_SUCCESS;
}

SqlError MysqlClient::NewDB(const std::string &dbName) {
    if (!m_bIsConnected) {
        if (mysql_real_connect(driver_, m_strHost.c_str(), m_strUser.c_str(), m_strPassWord.c_str(), nullptr, m_nPort,
                               nullptr, 0) == nullptr) {
            logger.info("reconnect to db failed with err:%d", mysql_errno(driver_));
            m_strErrMsg = mysql_error(driver_);
            return MYSQL_CONNECT_FAIL;
        }
    }

    auto [ err, affectRows ] = Execute("create database " + dbName);
    if (err != MYSQL_SUCCESS) {
        logger.info("create db for %s error.", dbName);
        m_strErrMsg = mysql_error(driver_);
        return MYSQL_CREATE_DB_FAILED;
    }

    err = SwitchBD(dbName);
    if (err != MYSQL_SUCCESS) {
        m_strErrMsg = mysql_error(driver_);
        return MYSQL_CREATE_DB_FAILED;
    }

    return MYSQL_SUCCESS;
}

void MysqlClient::close() {
    if (driver_ != nullptr) {
        mysql_close(driver_);
    }
}

MysqlClient::~MysqlClient() { close(); }

std::pair<SqlError, json::Json> MysqlClient::Query(const std::string &strSql) {
    auto ret = mysql_real_query(driver_, strSql.c_str(), strSql.size());
    if (ret != MYSQL_SUCCESS) {
        logger.warning("sql query failed. sql:%s ret:%d", strSql, ret);
        m_strErrMsg = mysql_error(driver_);
        return {MYSQL_QUERY_FAIL, {}};
    }

    auto res = mysql_store_result(driver_);
    if (res == nullptr) {
        logger.warning("query result is nullptr");
        m_strErrMsg = mysql_error(driver_);
        return {MYSQL_QUERY_FETCH_FAIL, {}};
    }

    MYSQL_FIELD *f{nullptr};
    FieldInfo    fieldInfo;
    while ((f = mysql_fetch_field(res)) != nullptr) {
        fieldInfo.push_back({f->org_name, f->type});
    }

    json::Json result = translateSqlResultToJson(*res, fieldInfo);
    return {MYSQL_SUCCESS, result};
}

SqlError MysqlClient::SwitchBD(const std::string &dbName) {
    auto ret = mysql_select_db(driver_, dbName.c_str());
    LOG_IF(ret != 0).info("switch db to %s failed. ret:%d", dbName, ret);
    return (ret ? MYSQL_SWITCH_DB_FAIL : MYSQL_SUCCESS);
}

std::pair<SqlError, json::Json> MysqlClient::ShowTables() { return Query("show tables"); }

std::pair<SqlError, json::Json> MysqlClient::ShowTable(const std::string &tableName) {
    return Query("desc " + tableName);
}

SqlError MysqlClient::CommitTransaction() {
    if (!mysql_commit(driver_)) {
        logger.info("commit transaction failed.");
        m_strErrMsg = mysql_error(driver_);
        return MYSQL_COMMIT_FAIL;
    }
    return MYSQL_SUCCESS;
}

SqlError MysqlClient::Rollback() {
    if (!mysql_rollback(driver_)) {
        logger.info("rollback transaction failed.");
        m_strErrMsg = mysql_error(driver_);
        return MYSQL_ROLLBACK_FAIL;
    }
    return MYSQL_SUCCESS;
}

std::pair<SqlError, long long> MysqlClient::Execute(const std::string &strSql) {
    auto ret = mysql_real_query(driver_, strSql.c_str(), strSql.size());
    if (ret != 0) {
        logger.warning("sql query failed. sql:%s ret:%d", strSql, ret);
        m_strErrMsg = mysql_error(driver_);
        return {MYSQL_QUERY_FAIL, 0};
    }

    auto affectNum = mysql_affected_rows(driver_);
    m_strErrMsg    = "Success";
    return {MYSQL_SUCCESS, affectNum};
}

json::Json MysqlClient::translateSqlResultToJson(MYSQL_RES &res, const FieldInfo &info) {
    json::Json result(json::Json::ARRAY);
    MYSQL_ROW  row;
    uint32_t   rowCnt   = 0;
    auto       fieldNum = mysql_num_fields(&res);
    while ((row = mysql_fetch_row(&res))) {
        json::Json dict(json::Json::OBJECT);
        for (auto idx = 0; idx < fieldNum; ++idx) {
            auto field = info[ idx ];
            switch (field.second) {
                case MYSQL_TYPE_INT24:
                case MYSQL_TYPE_TINY:
                case MYSQL_TYPE_SHORT:
                    dict[ field.first ] = json::Json(atoi(row[ idx ]));
                    break;
                case MYSQL_TYPE_DOUBLE:
                case MYSQL_TYPE_FLOAT:
                    dict[ field.first ] = json::Json(atof(row[ idx ]));
                    break;
                case MYSQL_TYPE_STRING:
                case MYSQL_TYPE_VARCHAR:
                case MYSQL_TYPE_VAR_STRING:
                    dict[ field.first ] = json::Json(row[ idx ]);
                    break;
                case MYSQL_TYPE_LONG:
                case MYSQL_TYPE_LONGLONG:
                    dict[ field.first ] = json::Json(atoi(row[ idx ]));
                    break;
                case MYSQL_TYPE_DATETIME:
                case MYSQL_TYPE_DATETIME2:
                    dict[ field.first ] = json::Json(Timestamp::fromTimeStr(row[ idx ]));
                    break;
                default:
                    if (row[ idx ] == nullptr) {
                        dict[ field.first ] = json::Json(nullptr);
                    } else {
                        dict[ field.first ] = json::Json(row[ idx ]);
                    }
                    break;
            }
        }
        rowCnt++;
        result.push_back(dict);
    }

    return result;
}

void MysqlClient::QueryCallback(const std::string &strSql, db::QueryCallback cb) {}
