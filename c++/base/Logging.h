#pragma once

#include "Backtrace.h"
#include "Exception.h"
#include "Format.h"
#include "LogHandle.h"
#include "Mutex.h"
#include "Range.h"
#include <iomanip>
#include <iostream>
#include <map>
#include <pwd.h>
#include <sstream>
#include <sys/time.h>
#include <unistd.h>
#include <vector>
#ifndef APP
#define APP "app"
#endif

namespace muduo {
namespace base {

DECLARE_EXCEPTION(LogException, Exception);

enum LogLevel { Debug, Info, Except, Warn, Error, Fatal, Alert, Emergency };
class Logger final {
public:
    ~Logger() {
        m_vHandleList.clear();
    }

public:
    struct FileAttribute {
        int         lineno;
        std::string funcname;
        std::string filename;
    };

    void SetAppendLF(bool val) {
        m_MessageAppendCRLF = val;
    }

    Logger &BasicConfig(LogLevel defaultLevel, const char *messageFormat, const char *filePrefix, const char *fileFormat, const char *fileMode = "a+");

    template <class... Args> void debug(const char *messagefmt, Args &&...arg) {
        this->LogMessage(Debug, messagefmt, arg...);
    }

    template <class... Args> void info(const char *messagefmt, Args &&...arg) {
        this->LogMessage(Info, messagefmt, arg...);
    }

    template <class... Args> void warning(const char *messagefmt, Args &&...arg) {
        this->LogMessage(Warn, messagefmt, arg...);
    }

    template <class... Args> void error(const char *messagefmt, Args &&...arg) {
        this->LogMessage(Error, messagefmt, arg...);
    }

    template <class... Args> void fatal(const char *messagefmt, Args &&...arg) {
        this->LogMessage(Fatal, messagefmt, arg...);
    }

    template <class... Args> void alert(const char *messagefmt, Args &&...arg) {
        this->LogMessage(Alert, messagefmt, arg...);
    }

    template <class... Args> void emergency(const char *messagefmt, Args &&...arg) {
        this->LogMessage(LogLevel::Emergency, messagefmt, arg...);
    }

    template <class... Args> void exception(const char *messagefmt, Args &&...arg) {
        this->LogMessage(Except, messagefmt, arg...);
        auto callstack = GetBackCallStack();
        this->LogMessage(Except, callstack.c_str());
    }

    std::string getLevelName(LogLevel nLevel);

    Logger &setFileAttr(const std::string &filename, const std::string &funcname, int lineno);
    Logger &setAppName(const std::string &appname);

    void addLogHandle(LogHandle *au) {
        m_mutexLock.Lock();
        m_vHandleList.push_back(au);
        m_mutexLock.UnLock();
    }

protected:
    std::string getCurrentHourTime(bool showMicroSeconds);
    void        getKeyString(const std::string &key, std::stringstream &ss, const std::string &message, LogLevel nLevel);
    std::string MessageFormat(const std::string &FormattedLogmessage, LogLevel nLevel);

    template <class... Args> void LogMessage(LogLevel nLevel, const char *messagefmt, Args &&...arg) {
        if (nLevel < m_nLevel)
            return;
        std::string LogMessage = messagefmt;
        formatString(LogMessage, arg...);
        std::string message = MessageFormat(LogMessage, nLevel);
        if (m_MessageAppendCRLF) {
            message.append("\n");
        }
        for (auto &handle : m_vHandleList) {
            m_mutexLock.Lock();
            handle->writeData(message.c_str(), message.size());
            m_mutexLock.UnLock();
        }
    }

    template <class T, class... Args> void formatString(std::string &result, T &val, Args &&...arg) {
        size_t      index    = 0;
        bool        finished = false;
        std::string keyPrefix;
        int         FilledInterger = 0;
        int         FilledPointer  = 0;
        int         nBeginIndex    = -1;
        char        FilledChar     = ' ';
        while (!finished) {
            index       = result.find("%", index);
            nBeginIndex = index;
            if (index == std::string::npos)
                break;
            if (result[ index + 1 ] == '%') {
                index += 2;
                continue;
            } else {
                bool bFindPointer = false;
                bool FirstInteger = false;
                for (size_t i = index + 1; i < result.size(); ++i) {
                    if (result[ i ] == 's' || result[ i ] == 'd' || result[ i ] == 'f' || result[ i ] == 'c' || result[ i ] == 'x' || result[ i ] == 'o' || result[ i ] == 'b') {
                        keyPrefix.push_back(result[ i ]);
                        finished = true;
                    } else if ((result[ i ] >= '0' && result[ i ] <= '9') || result[ i ] == '.') {
                        if (result[ i ] == '.')
                            bFindPointer = true;
                        else if (!bFindPointer)
                            FilledInterger = FilledInterger * 10 + result[ i ] - '0';
                        else if (bFindPointer)
                            FilledPointer = FilledPointer * 10 + result[ i ] - '0';
                        if (!FirstInteger && result[ i ] == '0') {
                            FilledChar   = '0';
                            FirstInteger = true;
                        }

                        keyPrefix.push_back(result[ i ]);
                    } else if (!FirstInteger) {
                        FilledChar = result[ i ];
                    }
                    index++;
                    if (finished)
                        break;
                }
            }
        }
        if (nBeginIndex != -1) {
            std::stringstream ss;
            ss << result.substr(0, nBeginIndex);
            if (FilledChar != ' ')
                ss << std::setfill(FilledChar);
            if (FilledInterger != 0 && keyPrefix.back() != 'f')
                ss << std::setw(FilledInterger);
            if (keyPrefix.back() == 'x')
                ss << std::hex;
            if (keyPrefix.back() == 'o')
                ss << std::oct;
            if (keyPrefix.back() == 'b')
                ss << std::boolalpha;
            ss << val;
            ss << result.substr(index + 1);
            result = ss.str();
            formatString(result, arg...);
        }
    }
    void formatString(std::string &result) {
    }

public:
    static Logger &getLogger(const std::string &LoggerName = "") {
        string strPrefix = LoggerName;
        if (LoggerName.empty())
            strPrefix = APP;
        if (!_MapLogger.count(strPrefix)) {
            _MapLogger[ strPrefix ] = Logger();
        }
        return _MapLogger.at(strPrefix);
    }

private:
    static std::map<std::string, Logger> _MapLogger;

protected:
    LogLevel                 m_nLevel;
    std::string              m_strMessageFormat;
    std::string              m_strAppName;
    FileAttribute            m_FileAttribute;
    std::vector<LogHandle *> m_vHandleList;
    MutexLock                m_mutexLock;
    bool                     m_MessageAppendCRLF{true};
};

} // namespace base
} // namespace muduo

#define LOG(appname) muduo::base::Logger::getLogger(appname).setFileAttr(__FILE__, __func__, __LINE__)

#define logger LOG(APP)
#define LOG_SYSTEM LOG(APP).setAppName("system")
#define LOG_IF(func) \
    if (func)        \
    LOG(APP)
