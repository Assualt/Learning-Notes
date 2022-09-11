#include "Backtrace.h"
#include <cxxabi.h>
#include <execinfo.h>
#include <memory.h>

namespace muduo {
int         maxTraceSize = 100;
std::string GetSymbolName(const std::string &symbol) {
    return abi::__cxa_demangle(symbol.c_str(), nullptr, nullptr, nullptr);
}

std::string TransLateDemangle(const std::string &str) {
    size_t pos = str.find('(');
    if (pos == std::string::npos) {
        return str;
    }

    size_t pos1 = str.find('+', pos + 1);
    if (pos1 == std::string::npos) {
        return str;
    }

    if (pos1 == pos + 1) {
        return str;
    }

    if (str.find("/libc.so") != std::string::npos) {
        return str;
    }

    std::string result = str.substr(0, pos);
    size_t      pos3   = str.find(')', pos1 + 1);
    std::string name   = GetSymbolName(str.substr(pos + 1, pos1 - pos - 1));
    result += " ";
    result += name;
    result += str.substr(pos1, pos3 - pos1);
    result += str.substr(pos3 + 1);
    return result;
}

std::string GetBackCallStack() {
    void *buffer[ 100 ] = {nullptr};
    int   size          = backtrace(buffer, maxTraceSize);

    auto trace = backtrace_symbols(buffer, size);
    if (trace == nullptr) {
        return "";
    }

    std::string res;
    for (auto i = 0; i < size; ++i) {
        res += "#";
        res += std::to_string(i);
        res += " ";
        try {
            res += TransLateDemangle(trace[ i ]);
        } catch (...) {
            res += trace[ i ];
            res += "\n";
            continue;
        }
        res += "\n";
    }

    return res;
}
} // namespace muduo