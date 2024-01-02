//
// Created by 侯鑫 on 2023/12/26.
//

#ifndef SSP_TOOLS_LOG_H
#define SSP_TOOLS_LOG_H

#include <string>
#include <iomanip>
#include <sstream>
#include <set>
#include <vector>
#include "exception.h"
#include "log_impl.h"
#include "mutex.h"

namespace ssp::base {
DECLARE_EXCEPTION(LogException, Exception)

enum class LogLevel {
    Debug, Info, Except, Warn, Error, Fatal, Alert, Emergency
};

struct LogFileAttr {
    uint32_t    lineNo;
    std::string funcName;
    std::string fileName;
};

class Logger {
public:
    template <class... Args> void Debug(const char *fmt, Args &&... arg) { LogMessage(LogLevel::Debug, fmt, arg...); }

    template <class... Args> void Info(const char *fmt, Args &&... arg) { LogMessage(LogLevel::Info, fmt, arg...); }

    template <class... Args> void Warning(const char *fmt, Args &&... arg) { LogMessage(LogLevel::Warn, fmt, arg...); }

    template <class... Args> void Error(const char *fmt, Args &&... arg) { LogMessage(LogLevel::Error, fmt, arg...); }

    template <class... Args> void Fatal(const char *fmt, Args &&... arg) { LogMessage(LogLevel::Fatal, fmt, arg...); }

    template <class... Args> void Alert(const char *fmt, Args &&... arg) { LogMessage(LogLevel::Alert, fmt, arg...); }

    template <class... Args> void Emergency(const char *fmt, Args &&... arg) { LogMessage(LogLevel::Emergency, fmt, arg...); }

    template <class... Args> void Exception(const char *fmt, Args &&... arg) {
        LogMessage(LogLevel::Except, fmt, arg...);
        LogMessage(LogLevel::Except, "%s%s", "\n", "");
    }

public:
    Logger & BasicConfig(LogLevel level, const char *msgFmt, const char *prefix, const char *fmt, const char *mode = "a+");

    void AddLogHandle(LogImpl *au);

    static void SetFileAttr(const std::string &fileName, const std::string &funcName, uint32_t lineno);

    static Logger &GetLogger(const std::string &name = "");

    Logger &SetAppName(const std::string &appName);

private:
    template <class... Args> void LogMessage(LogLevel level, const char *fmt, Args &&... arg);

    template <class T, class... Args> void FormatString(std::string &result, T &val, Args &&... arg);

    void FormatString(std::string &result) {}

    std::string MessageFormat(const std::string &fmtLogMsg, LogLevel level);

    void GetKeyVal(const std::string &key, std::stringstream &ss, const std::string &msg, LogLevel level);

    static std::string GetLevelName(LogLevel level);

private:
    LogLevel defaultLevel_ { LogLevel::Info };
    std::string msgFmt_;
    std::string appName_;
    std::vector<LogImpl *> logImplList_;
    MutexLock lock_;
};


template<class... Args> void Logger::LogMessage(LogLevel level, const char *fmt, Args &&...arg) {
    if (defaultLevel_ > level) {
        return;
    }

    if (fmt == nullptr) {
        return;
    }

    std::string result = fmt;
    FormatString(result, arg...);
    std::string message = MessageFormat(result, level);
    message.append("\r\n");
    for (auto &handle : logImplList_) {
        lock_.Lock();
        handle->WriteData(message.c_str(), message.size());
        lock_.UnLock();
    }
}

template <class T, class... Args> void Logger::FormatString(std::string &result, T &val, Args &&... arg)
{
    static std::set<char> fmtCharMap = {'s', 'd', 'f', 'c', 'x', 'o', 'b'};
    size_t      index    = 0;
    bool        finish = false;
    std::string keyPrefix;
    int         fInt = 0;
    int         filledPointer = 0;
    size_t      beginIndex;
    char        fChar = ' ';
    while (!finish) {
        index       = result.find('%', index);
        beginIndex = index;
        if (index == std::string::npos) {
            break;
        }

        if (result[ index + 1 ] == '%') {
            index += 2;
            continue;
        }

        bool findPointer = false;
        bool firstIsInt = false;
        for (size_t i = index + 1; i < result.size(); ++i) {
            if (fmtCharMap.count(result[i])) {
                keyPrefix.push_back(result[i]);
                finish = true;
            } else if ((result[i] >= '0' && result[i] <= '9') || result[i] == '.') {
                if (result[i] == '.')
                    findPointer = true;
                else if (!findPointer)
                    fInt = fInt * 10 + result[i] - '0';
                else
                    filledPointer = filledPointer * 10 + result[i] - '0';
                if (!firstIsInt && result[i] == '0') {
                    fChar = '0';
                    firstIsInt = true;
                }

                keyPrefix.push_back(result[i]);
            } else if (!firstIsInt) {
                fChar = result[i];
            }
            index++;
            if (finish) break;
        }
    }
    if (beginIndex == -1) {
        return;
    }

    std::stringstream ss;
    ss << result.substr(0, beginIndex);
    if (fChar != ' ')
        ss << std::setfill(fChar);
    if (fInt != 0 && keyPrefix.back() != 'f')
        ss << std::setw(fInt);
    if (keyPrefix.back() == 'x')
        ss << std::hex;
    if (keyPrefix.back() == 'o')
        ss << std::oct;
    if (keyPrefix.back() == 'b')
        ss << std::boolalpha;
    ss << val;
    ss << result.substr(index + 1);
    result = ss.str();
    FormatString(result, arg...);
}
}

#define LOG(appName) \
    ssp::base::Logger::SetFileAttr(__FILE__, __func__, __LINE__); \
    ssp::base::Logger::GetLogger(appName).SetAppName(appName)

#define logger LOG("APP")

#define log_sys LOG("System")

#endif //SSP_TOOLS_LOG_H
