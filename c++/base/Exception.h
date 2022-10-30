#pragma once
#include <exception>
#include <iostream>
#define DECLARE_EXCEPTION(exp, expParent)                                                                              \
    class exp : public expParent {                                                                                     \
    public:                                                                                                            \
        explicit exp(const char *lpszMsg) noexcept                                                                     \
            : expParent(lpszMsg) {}                                                                                    \
        explicit exp(const std::string &strMsg) noexcept                                                               \
            : expParent(strMsg.c_str()) {}                                                                             \
        exp(const exp &re) noexcept                                                                                    \
            : expParent(re) {}                                                                                         \
    };
namespace muduo::base {
class Exception : public std::exception {
public:
    explicit Exception(const char *what) noexcept
        : m_strErrorMsg(what) {}

    [[nodiscard]] const char *what() const noexcept override { return m_strErrorMsg.c_str(); }
    friend std::ostream      &operator<<(std::ostream &out, Exception &e) {
             out << e.what();
             return out;
    }

private:
    std::string m_strErrorMsg;
};
} // namespace muduo::base