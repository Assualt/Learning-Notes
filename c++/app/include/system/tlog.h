//
// Created by Sunshine on 2020/8/30.
//
#pragma once
#include "xmtdef.h"
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <mutex>
#include <queue>
#include <set>
#include <sstream>
#include <cstdio>
#include <ctime>
#include <thread>
#include <unistd.h>
#include <vector>
#include <sys/time.h>  // NOLINT

using namespace std;

NAMESPACE_BEGIN
namespace detail {
enum Level { DEBUG, INFO, WARNING, ERRORS, ALERT, EMERGENCY };
const std::vector<std::string> levelStringVector = {  // NOINT
        "Debug",
        "Info",
        "Warning",
        "Errors",
        "Alert",       // NOLINT
        "Emergency"};  // NOLINT

typedef struct {
    size_t lineno;
    const char *funcName;
    const char *fileName;
} commandLineInfo;

constexpr const char *timeFmt = "%04Y-%02m-%02d %02H:%02M:%02S.%03n"; /* NO INT */
constexpr const char *ctime = "%02H:%02M:%02S.%06u";
const std::set<char> timeOperator = {
        // NOLINT
        'Y', /*	Year (1900- now)*/
        'm', /*	Month (0-11) 	*/
        'd', /*	Day of Month (1-31) */
        'H', /* Hour of Day (0-23) */
        'M', /* Minutes of Hour (0-59) */
        'S', /* Seconds of Minute (0-59) */
        'n', /* Micro Seconds */
        'w', /* the day of Week */
        'Z', /* the Time Zone */
        'T', /* the ISO time string */
        'a', /* the 12 Am. Pm*/
        'u', /* the useconds */
};
const std::vector<std::string> WeekDayEngList = {"Sun", "Mon", "Tues", "Wed", "Thur", "Fri", "Sat"};  // NOLINT
const std::vector<std::string> MonthEngList = {
        "Jan",
        "Feb",
        "Mar",
        "Apr",
        "May",
        "Jun",
        "Jul",
        "Aug",  // NOLINT
        "Sept",
        "Oct",
        "Nov",
        "Dec"};  // NOLINT

const std::set<char> basicMsgFmt = {'f', 's', 'd', 'l'};  // NOLINT

void FormatTimeString(
        struct tm *t,
        struct timeval *tv,
        const std::string &FilledString,
        const char op,
        std::stringstream &ss);
enum RotatingMode { DayRotatingMode, SizeLimitRotatingMode, LineLmitRotatingMode };

std::string getFormatTimeString(const char *strFmt);

template <typename F>
void ExpandItem(std::queue<std::string> &argQueue, F &&val) {
    std::stringstream ss;
    ss << val;
    argQueue.push(ss.str());
}

template <typename... Args>
void ExpandArgs(std::queue<std::string> &argQueue, Args &&... args) {
    std::initializer_list<int>{(ExpandItem(argQueue, args), 0)...};

    Level getLevelFromString(const std::string &strLevel);
}

void FormatBasicMsg(const std::string &FilledString, const char op, std::stringstream &ss, const std::string &item);

std::string getFormatMessageString(const char *messageFmt, std::queue<std::string> &argsQueue);

void FormatLogMessage(
        const std::string &key,
        std::stringstream &ss,
        const std::string &message,
        const std::string &levelName,
        const detail::commandLineInfo &info,
        const std::string &loggerName);

std::string getLogMessageFormat(
        const char *fmt,
        const char *message,
        const char *levelName,
        const detail::commandLineInfo &info,
        const std::string &loggerName);

std::ios::openmode getFileMode(const std::string &strMode);

Level getLevelFromString(const string &basicString);
}  // namespace detail

class Logger {
public:
    Logger(const std::string &loggerName) :
            m_strMessageFmt("%(asctime)s :%(levelname)s: %(message)s"),
            m_nLevel(detail::Level::DEBUG),
            m_strLoggerName(loggerName) {}
    void setLevel(detail::Level level) {
        m_nLevel = level;
    }
    void BasicConfig(
            const char *messageFmt,
            const std::string &appendingFileName = "",
            const std::string &appendingFileFmt = "",
            const std::string &OpenMode = "a") {
        m_strMessageFmt = messageFmt;
        m_strAppendFileFmt = appendingFileFmt;
        if (!appendingFileName.empty()) {
            std::string FileName = appendingFileName + "." + detail::getFormatTimeString(appendingFileFmt.c_str());
            if (m_OutStream.is_open()) {
                throw std::runtime_error("appending file is opened.");
            }
            m_OutStream.open(FileName, detail::getFileMode(OpenMode));
        }
    }
    virtual ~Logger() {
        if (m_OutStream.is_open())
            m_OutStream.close();
    }

public:
    template <typename... Args>
    void debug(const char *ptrMsg, Args &&... args) {
        if (levelCompare("Debug")) {
            std::queue<std::string> argsQueue;
            detail::ExpandArgs(argsQueue, args...);
            this->logMessage("Debug", ptrMsg, argsQueue);
        }
    }
    template <typename... Args>
    void info(const char *ptrMsg, Args &&... args) {
        if (levelCompare("Info")) {
            std::queue<std::string> argsQueue;
            detail::ExpandArgs(argsQueue, args...);
            this->logMessage("INFO", ptrMsg, argsQueue);
        }
    }
    template <typename... Args>
    void warning(const char *ptrMsg, Args &&... args) {
        if (levelCompare("Warning")) {
            std::queue<std::string> argsQueue;
            detail::ExpandArgs(argsQueue, args...);
            this->logMessage("Warning", ptrMsg, argsQueue);
        }
    }
    template <typename... Args>
    void error(const char *ptrMsg, Args &&... args) {
        if (levelCompare("Error")) {
            std::queue<std::string> argsQueue;
            detail::ExpandArgs(argsQueue, args...);
            this->logMessage("Error", ptrMsg, argsQueue);
        }
    }
    template <typename... Args>
    void alert(const char *ptrMsg, Args &&... args) {
        if (levelCompare("Alert")) {
            std::queue<std::string> argsQueue;
            detail::ExpandArgs(argsQueue, args...);
            this->logMessage("Alert", ptrMsg, argsQueue);
        }
    }
    template <typename... Args>
    void emergency(const char *ptrMsg, Args &&... args) {
        if (levelCompare("Emergency")) {
            std::queue<std::string> argsQueue;
            detail::ExpandArgs(argsQueue, args...);
            this->logMessage("Emergency", ptrMsg, argsQueue);
        }
    }
    Logger &setFileCommandInfo(const std::string &filename, const std::string &funcname, int fileno) {
        m_CommandInfo.fileName = filename.c_str();
        m_CommandInfo.funcName = funcname.c_str();
        m_CommandInfo.lineno = fileno;
        return *this;
    }

private:
    bool levelCompare(const std::string &levelName) {
        return detail::getLevelFromString(levelName) >= this->m_nLevel;
    }

    void logMessage(const char *levelName, const char *msgfmt, std::queue<std::string> &args) {
        std::string messageString = detail::getFormatMessageString(msgfmt, args);
        std::string LogMessage = detail::getLogMessageFormat(
                m_strMessageFmt.c_str(), messageString.c_str(), levelName, m_CommandInfo, m_strLoggerName);
        m_Lock.lock();
        if (m_OutStream.is_open()) {
            m_OutStream << LogMessage << std::endl;
        } else {
            std::cout << LogMessage << std::endl;
        }
        m_Lock.unlock();
    }

private:
    std::string m_strMessageFmt;
    std::string m_strAppendFileFmt;
    detail::commandLineInfo m_CommandInfo;
    detail::Level m_nLevel;
    std::string m_strLoggerName;
    std::ofstream m_OutStream;
    mutex m_Lock;
    detail::RotatingMode m_RotatingMode;
};

class logImpl {
public:
    static std::map<std::string, Logger *> g_LoggerMap;

public:
    static Logger &getLogger(const std::string &prefix) {
        return *(g_LoggerMap.at(prefix));
    }
    static void AppendLogger(const std::string &prefix, Logger *x) {
        g_LoggerMap[prefix] = x;
    }
};

#define logger logImpl::getLogger("main").setFileCommandInfo(__FILE__, __func__, __LINE__)
#define LOG(level)

NAMESPACE_END