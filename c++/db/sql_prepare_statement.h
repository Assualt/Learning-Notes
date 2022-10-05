//
// Created by xhou on 2022/10/5.
//

#ifndef MUDUO_BASE_TOOLS_DB_CLIENT_IF_H
#define MUDUO_BASE_TOOLS_DB_CLIENT_IF_H

#include <algorithm>
#include <sstream>
#include <string>

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
        m_stream << m_strRawSql.substr(m_nPos, nPos - m_nPos);
        m_stream << val;
        m_nPos = nPos + 1;
        if (m_nCurrentParamCnt == m_nParamsCnt)
            m_stream << m_strRawSql.substr(nPos + 1);
        return *this;
    }

    std::string getFullExecuteSql() const {
        if (m_nParamsCnt == 0) {
            return m_strRawSql;
        }
        return m_nParamsCnt == 0 ? m_strRawSql : m_stream.str();
    }

    ~SqlPrepareStatement() { reset(); }

    void reset() {
        m_nPos = 0;
        m_strRawSql.clear();
        m_nCurrentParamCnt = 0;
        m_nParamsCnt       = 0;
        m_stream.clear();
    }

protected:
    std::stringstream m_stream;
    std::string       m_strRawSql;
    size_t            m_nParamsCnt;
    size_t            m_nPos;
    size_t            m_nCurrentParamCnt;
};

#endif // MUDUO_BASE_TOOLS_DB_CLIENT_IF_H
