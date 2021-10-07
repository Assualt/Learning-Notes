#include <exception>
#include <iostream>
#define DECLARE_EXCEPTION(exp, expParent)               \
    class exp : public expParent {                      \
    public:                                             \
        explicit exp(const char *lpszMsg) throw()       \
            : expParent(lpszMsg) {                      \
        }                                               \
        explicit exp(const std::string &strMsg) throw() \
            : expParent(strMsg.c_str()) {               \
        }                                               \
        exp(const exp &re) throw()                      \
            : expParent(re) {                           \
        }                                               \
    };
namespace muduo {
namespace base {
class Exception : public std::exception {
public:
    Exception(const char *what) throw()
        : m_strErrorMsg(what) {
    }
    const char *what() const throw() {
        return m_strErrorMsg.c_str();
    }
    friend std::ostream &operator<<(std::ostream &out, Exception &e) {
        out << e.what();
        return out;
    }

private:
    std::string m_strErrorMsg;
};
} // namespace base
}