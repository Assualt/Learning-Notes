#include "base/Logging.h"
#include "sqlite_cli.h"

using namespace muduo::base;
using namespace db;

bool SqliteClient::openDb(const std::string &dbpath, int option) {
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
        logger.warning("sql is not valid. input:%s ret:%d", stmt.getFullExecuteSql(), ret);
        sqlite3_finalize(st);
        st = nullptr;
        return false;
    }
    ret = sqlite3_step(st);
    sqlite3_finalize(st);
    st = nullptr;
    return ret == 1;
}

SqliteClient::~SqliteClient() { close(); }

void SqliteClient::close() {
    if (m_sqlHandle == nullptr) {
        return;
    }
    sqlite3_close_v2(m_sqlHandle);
    m_sqlHandle = nullptr;
}
