//
// Created by 侯鑫 on 2023/12/26.
//

#ifndef SSP_TOOLS_EXCEPTION_H
#define SSP_TOOLS_EXCEPTION_H

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

namespace ssp::base {
    class Exception : public std::exception {
    public:
        explicit Exception(const char *what) noexcept
                : errMsg_(what) {}

        [[nodiscard]] const char *what() const noexcept override { return errMsg_.c_str(); }
        friend std::ostream &     operator<<(std::ostream &out, Exception &e) {
            out << e.what();
            return out;
        }

    private:
        std::string errMsg_;
    };
} // namespace ssp::base

#endif //SSP_TOOLS_EXCEPTION_H
