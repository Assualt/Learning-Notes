#ifndef _X_EXCEPTION_2018_12_14
#define _X_EXCEPTION_2018_12_14

#include "xmtdef.h"

#include <exception>
#include <iostream>

using namespace std;

NAMESPACE_BEGIN

// /* declare an exception
// 	Error code :
// 	from	1001 used by xml
// 	from	1101 used by Database
// */

#define DECLARE_EXCEPTION(exp, expParent)                             \
    class exp : public expParent {                                    \
    public:                                                           \
        explicit exp(const char *lpszMsg, XEP_CODE nCode) throw() :   \
                expParent(lpszMsg, nCode) {}                          \
        explicit exp(const tstring &strMsg, XEP_CODE nCode) throw() : \
                expParent(strMsg.c_str(), nCode) {}                   \
        explicit exp(const tstring &lpszMsg) throw() :                \
                expParent(lpszMsg.c_str()) {}                         \
        exp(const exp &re) throw() : expParent(re) {}                 \
    };

const char m_strErrorMsg[6][20] = {"NORMAL",
                                   "Normal Error",
                                   "Invalid Args",
                                   "Handler Error",
                                   "Internal Error",
                                   "Unknown Error"};

class XException : public std::exception {
public:
    enum XEP_CODE {
        XEP_NORMAIL,         //正常
        XEP_ERROR,           //一般错误
        XEP_INVALID_ARGS,    //错误参数
        XEP_HADNLER_ERROR,   //处理错误
        XEP_INTERNAL_ERROR,  //内部错误
        XEP_UNKNOWN_ERROR    //未知错误
    };
    XException(const char *what, XEP_CODE code = XEP_ERROR) throw() {
        m_nCode = code;
        if (NULL == what) {
            m_strMsg = new char[1];
            m_strMsg[0] = '\0';
        } else {
            m_strMsg = new char[strlen(what) + 1];
            strcpy(m_strMsg, what);
            m_strMsg[strlen(what)] = '\0';
        }
    }
    const char *what() const throw() {
        return m_strMsg;
    }
    XEP_CODE getErrorCode(void) const throw() {
        return m_nCode;
    }
    const char *getErrorCodeToString() const throw() {
        return m_strErrorMsg[m_nCode];
    }

    ~XException() throw() {
        delete[] m_strMsg;
    }
    friend ostream &operator<<(ostream &out, XException &e) {
        out << e.what();
        return out;
    }

private:
    char *m_strMsg;
    XEP_CODE m_nCode;
    static char m_strErrorMsg[6][20];
};
NAMESPACE_END
#endif  // !_X_EXCEPTION_2018_12_14
