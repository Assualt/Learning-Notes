#include "logging.h"
#include <algorithm>
#include <iostream>
#include <map>
#include <sqlite3.h>
#include <sstream>

namespace db {

#define DefaultMode (SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_NOMUTEX | SQLITE_OPEN_SHAREDCACHE)

class SqlPrepareStatement {
public:
    SqlPrepareStatement(const std::string &sql)
        : m_strRawSql(sql)
        , m_nPos(0)
        , m_nCurrentParamCnt(0) {
        m_nParamsCnt = std::count_if(sql.begin(), sql.end(), [](char ch) { return ch == '%'; });
    }

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

    std::string getFullExecuteSql() const {
        if (m_nParamsCnt == 0)
            return m_strRawSql;
        return m_sstream.str();
    }

    ~SqlPrepareStatement() {
        reset();
    }

    void reset() {
        m_nPos = 0;
        m_strRawSql.clear();
        m_nCurrentParamCnt = 0;
        m_nParamsCnt       = 0;
        m_sstream.clear();
    }

protected:
    std::stringstream m_sstream;
    std::string       m_strRawSql;
    size_t            m_nParamsCnt;
    size_t            m_nPos;
    size_t            m_nCurrentParamCnt;
};

class SqliteClient {
public:
    bool opendb(const std::string &dbpath, int option = DefaultMode) {
        m_strDBFilePath = dbpath;
        int result      = sqlite3_open_v2(dbpath.c_str(), &m_sqlHandle, option, nullptr);
        if (result == SQLITE_OK)
            return true;
        logger.warning("open db %s failed", dbpath);
        return false;
    }
    template <int N> std::vector<std::vector<std::string>> query(SqlPrepareStatement &stmt) {
        sqlite3_stmt *                        st = nullptr;
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

    bool execute(SqlPrepareStatement &stmt) {
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

    ~SqliteClient() {
        close();
    }

    void close() {
        if (m_sqlHandle) {
            sqlite3_close_v2(m_sqlHandle);
            m_sqlHandle = nullptr;
        }
    }

protected:
    std::string m_strDBFilePath;
    sqlite3 *   m_sqlHandle{nullptr};
};

} // namespace db

int main(int argc, char const *argv[]) {
    db::SqlPrepareStatement st("insert into test(id,name,desc) values(%,'%','%')");

    st.assignValue(12).assignValue("xhou").assignValue("description ok...");
    std::cout << st.getFullExecuteSql() << std::endl;

    db::SqliteClient client;

    client.opendb("./test.db");

    // std::cout << "Insert Row :" << client.exec(st) << std::endl;

    db::SqlPrepareStatement st1("select * from test");

    auto result = client.query<3>(st1);

    for (auto item : result) {
        for (auto val : item) {
            std::cout << val << " ";
        }
        std::cout << "\n";
    }
    return 0;
}
