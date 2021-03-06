#include <iostream>
#include <sqlite3.h>
#include <sstream>
#include <vector>

namespace db {

#define DefaultMode (SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_NOMUTEX | SQLITE_OPEN_SHAREDCACHE)

class SqlPrepareStatement {
public:
    SqlPrepareStatement(const std::string &sqlTmpl);

    template <class T> SqlPrepareStatement &assignValue(const T &val) {
        size_t nPos = m_strRawSql.find_first_of('%', m_nPos);
        if (nPos == std::string::npos)
            return *this;
        if (m_nCurrentParamCnt > m_nParamsCnt)
            return *this;
        m_nCurrentParamCnt += 1;
        m_sstream << m_strRawSql.substr(m_nPos, nPos - m_nPos);
        m_sstream << val;
        m_nPos = nPos + 1;
        if (m_nCurrentParamCnt == m_nParamsCnt)
            m_sstream << m_strRawSql.substr(nPos + 1);
        return *this;
    }

    std::string getFullExecuteSql() const;

    ~SqlPrepareStatement();

    void reset();

protected:
    std::stringstream m_sstream;
    std::string       m_strRawSql;
    size_t            m_nParamsCnt;
    size_t            m_nPos;
    size_t            m_nCurrentParamCnt;
};

class SqliteClient {
public:
    bool opendb(const std::string &dbpath, int option = DefaultMode);

    template <int N> std::vector<std::vector<std::string>> query(SqlPrepareStatement &stmt) {
        sqlite3_stmt *                        st = nullptr;
        int                                   ret;
        std::vector<std::vector<std::string>> result;
        if ((ret = sqlite3_prepare_v2(m_sqlHandle, stmt.getFullExecuteSql().c_str(), -1, &st, nullptr)) != SQLITE_OK) {
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
    sqlite3 *   m_sqlHandle{nullptr};
};

} // namespace db
