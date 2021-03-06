#include "sqlitecli.h"
#include <map>
#include "logging.h"
#include <algorithm>
namespace db {

SqlPrepareStatement::SqlPrepareStatement(const std::string &sql)
    : m_strRawSql(sql)
    , m_nPos(0)
    , m_nCurrentParamCnt(0) {
    m_nParamsCnt = std::count_if(sql.begin(), sql.end(), [](char ch) { return ch == '%'; });
}

std::string SqlPrepareStatement::getFullExecuteSql() const {
    if (m_nParamsCnt == 0)
        return m_strRawSql;
    return m_sstream.str();
}

SqlPrepareStatement::~SqlPrepareStatement() {
    reset();
}

void SqlPrepareStatement::reset() {
    m_nPos = 0;
    m_strRawSql.clear();
    m_nCurrentParamCnt = 0;
    m_nParamsCnt       = 0;
    m_sstream.clear();
}

bool SqliteClient::opendb(const std::string &dbpath, int option) {
    m_strDBFilePath = dbpath;
    int result      = sqlite3_open_v2(dbpath.c_str(), &m_sqlHandle, option, nullptr);
    if (result == SQLITE_OK)
        return true;
    logger.warning("open db %s failed", dbpath);
    return false;
}

bool SqliteClient::execute(SqlPrepareStatement &stmt) {
    int           ret;
    sqlite3_stmt *st = nullptr;
    if ((ret = sqlite3_prepare_v2(m_sqlHandle, stmt.getFullExecuteSql().c_str(), -1, &st, nullptr)) != SQLITE_OK) {
        logger.warning("sql is not valid. input:%s", stmt.getFullExecuteSql());
        sqlite3_finalize(st);
        st = nullptr;
        return false;
    }
    ret = sqlite3_step(st);
    sqlite3_finalize(st);
    st = nullptr;
    return ret == 1;
}

SqliteClient::~SqliteClient() {
    close();
}

void SqliteClient::close() {
    if (m_sqlHandle) {
        sqlite3_close_v2(m_sqlHandle);
        m_sqlHandle = nullptr;
    }
}

} // namespace db
