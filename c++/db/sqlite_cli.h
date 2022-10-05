#include "base/Logging.h"
#include "sql_prepare_statement.h"
#include <algorithm>
#include <iostream>
#include <map>
#include <sqlite3.h>
#include <sstream>

namespace db {

#define DefaultMode (SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_NOMUTEX | SQLITE_OPEN_SHAREDCACHE)

class SqliteClient {
public:
    bool openDb(const std::string &dbpath, int option = DefaultMode);

    template <int N> std::vector<std::vector<std::string>> query(SqlPrepareStatement &stmt) {
        sqlite3_stmt                         *st = nullptr;
        int                                   ret;
        std::vector<std::vector<std::string>> result;
        if ((ret = sqlite3_prepare_v2(m_sqlHandle, stmt.getFullExecuteSql().c_str(), -1, &st, nullptr)) != SQLITE_OK) {
            logger.warning("sql is not valid. input:%s", stmt.getFullExecuteSql());
            sqlite3_finalize(st);
            st = nullptr;
            return result;
        }
        while ((ret = sqlite3_step(st)) == SQLITE_ROW) {
            std::vector<std::string> RowData;
            for (size_t i = 0; i < N; ++i)
                RowData.push_back((const char *)sqlite3_column_text(st, i));
            result.push_back(RowData);
        }
        return result;
    }

    bool execute(SqlPrepareStatement &stmt);

    ~SqliteClient();

    void close();

protected:
    std::string m_strDBFilePath;
    sqlite3    *m_sqlHandle{nullptr};
};

} // namespace db
