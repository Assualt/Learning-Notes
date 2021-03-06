//
// Created by Sunshine on 2020/8/30.
//
#pragma once
#include <cstdio>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <mutex>
#include <pwd.h>
#include <queue>
#include <set>
#include <sstream>
#include <sys/time.h> // NOLINT
#include <sys/types.h>
#include <thread>
#include <unistd.h>
#include <vector>

#define STDOUT_PREFIX "main.stdout"

namespace tlog {
enum Level { DEBUG, INFO, WARNING, ERRORS, ALERT, EMERGENCY };
struct CommandLineInfo {
    size_t      lineno;
    std::string funcName;
    std::string fileName;
};

class detail {
public:
    static std::vector<std::string>   levelStringVector;
    static std::map<std::string, int> levelMap;
    static std::string                timeFmt;
    static std::string                ctime;
    static std::set<char>             timeOperator;
    static std::vector<std::string>   WeekDayEngList;
    static std::vector<std::string>   MonthEngList;
    static std::set<char>             basicMsgFmt;

public:
    static void FormatTimeString(struct tm *t, struct timeval *tv, const std::string &FilledString, const char op, std::stringstream &ss) {
        char FilledChar;
        int  FilledLength = 0;
        bool Find_start   = false;
        for (char i : FilledString) {
            if (i >= '0' && i <= '9' && Find_start) {
                FilledLength = FilledLength * 10 + i - '0';
            } else if (i > '0' && i <= '9') {
                FilledLength = FilledLength * 10 + i - '0';
                Find_start   = true;
            } else if (!Find_start) {
                FilledChar = i;
            }
        }
        if (op == 'T') {
            // ISO String
            ss << detail::WeekDayEngList[ t->tm_wday ] << " ";
            ss << detail::MonthEngList[ t->tm_mon ] << " ";
            ss << std::setw(2) << std::setfill('0') << (t->tm_mday) << " ";
            ss << std::setw(2) << std::setfill('0') << (t->tm_hour) << ":";
            ss << std::setw(2) << std::setfill('0') << (t->tm_min) << ":";
            ss << std::setw(2) << std::setfill('0') << (t->tm_sec) << " ";
            ss << "CST " << std::setw(4) << (t->tm_year + 1900);
            return;
        }
        if (FilledLength != 0)
            ss << std::setw(FilledLength);
        if (FilledChar)
            ss << std::setfill(FilledChar);
        if (op == 'Y')
            ss << (t->tm_year + 1900);
        else if (op == 'm')
            ss << (t->tm_mon + 1);
        else if (op == 'd')
            ss << (t->tm_mday);
        else if (op == 'H')
            ss << (t->tm_hour);
        else if (op == 'M')
            ss << (t->tm_min);
        else if (op == 'S')
            ss << (t->tm_sec);
        else if (op == 'w')
            ss << detail::WeekDayEngList[ t->tm_wday ];
        else if (op == 'n')
            ss << (tv->tv_usec / 1000);
        else if (op == 'u')
            ss << (tv->tv_usec);
        else if (op == 'Z')
            ss << '2';
        else if (op == 'a')
            ss << (t->tm_hour / 12 >= 1 ? "PM." : "AM.") << (t->tm_hour % 12);
    }
    static std::string getFormatTimeString(const char *strFmt) {
        if (nullptr == strFmt)
            return "";
        struct timeval tv {};
        gettimeofday(&tv, nullptr);
        time_t            ct = time(nullptr);
        struct tm *       t  = localtime(&ct);
        std::stringstream ss;

        const char *p = strFmt;
        for (; (*p) != '\0'; p++) {
            if (('%' == *p) && (p + 1 != nullptr)) // find %
            {
                if (*(p + 1) == '%') {
                    ss << "%";
                    p = p + 1;
                } else {
                    const char *q       = p + 1;
                    bool        FindOpt = false;
                    std::string FilledString;
                    do {
                        if (!detail::timeOperator.count(*q))
                            FilledString.push_back(*q);
                        else if (detail::timeOperator.count(*q)) { // Found Time Opt
                            FormatTimeString(t, &tv, FilledString, *q, ss);
                            p       = q++;
                            FindOpt = true;
                            break;
                        }
                    } while (++q);
                    if (!FindOpt)
                        p += 1;
                }
            } else
                ss << *p;
        }
        return ss.str();
    }

    template <typename F> static void ExpandItem(std::queue<std::string> &argQueue, F &&val) {
        std::stringstream ss;
        ss << val;
        argQueue.push(ss.str());
    }

    template <typename... Args> static void ExpandArgs(std::queue<std::string> &argQueue, Args &&... args) {
        std::initializer_list<int>{(ExpandItem(argQueue, args), 0)...};
    }

    static void FormatBasicMsg(const std::string &FilledString, const char op, std::stringstream &ss, const std::string &item) {
        char   FilledChar        = ' ';
        size_t FilledLength      = 0;
        bool   Find_start        = false;
        bool   Find_Pointer      = false;
        size_t FilledWithPointer = 0;
        size_t FilledWithZS;
        for (char i : FilledString) {
            if (i >= '0' && i <= '9' && Find_start && !Find_Pointer) {
                FilledLength = FilledLength * 10 + i - '0';
            } else if (i > '0' && i <= '9' && !Find_start && !Find_Pointer) {
                FilledLength = FilledLength * 10 + i - '0';
                Find_start   = true;
            } else if (i == '.') {
                Find_Pointer = true;
                Find_start   = false;
            } else if (i >= '0' && i <= '9' && Find_Pointer)
                FilledWithPointer = FilledWithPointer * 10 + i - '0';
            else if (!Find_start)

                FilledChar = i;
        }
        FilledWithZS = FilledLength;
        FilledLength += Find_Pointer ? 1 + FilledWithPointer : 0;
        if (FilledLength != 0 && op != 'f')
            ss << std::setw(FilledWithZS);
        if (FilledChar && op != 'f')
            ss << std::setfill(FilledChar);
        if (op == 's' || op == 'd') {
            ss << item;
        } else if (op == 'f') {
            if (!FilledLength) {
                ss << item;
            } else {
                if (item.find('.') != std::string::npos) {
                    std::string zs = item.substr(0, item.find('.'));
                    if (!FilledWithZS)
                        ss << zs; // NOLINT
                    else if (zs.size() > FilledWithZS) {
                        ss << zs.substr(zs.length() - FilledWithZS);
                    } else if (zs.size() == FilledWithZS) {
                        ss << zs;
                    } else {
                        ss << std::setfill('0') << std::setw(FilledWithZS) << zs;
                    }
                    ss << ".";
                    std::string pointer = item.substr(item.find('.') + 1);
                    if (!FilledWithPointer)
                        ss << pointer;
                    if (pointer.size() > FilledWithPointer) {
                        ss << pointer.substr(0, FilledWithPointer);
                    } else if (pointer.size() == FilledWithPointer) {
                        ss << pointer;
                    } else {
                        ss << pointer << std::string(FilledWithPointer - pointer.size(), '0');
                    }
                } else {
                    if (!FilledWithZS)
                        ss << item;
                    else if (item.size() > FilledWithZS)
                        ss << item.substr(item.size() - FilledWithZS);
                    else
                        ss << std::string(FilledWithZS - item.size(), '0') << item;
                    ss << ".";
                    if (FilledWithPointer)
                        ss << std::string(FilledWithPointer, '0');
                }
            }
        }
    }

    static std::string getFormatMessageString(const char *messageFmt, std::queue<std::string> &argsQueue) {
        std::stringstream ss;
        const char *      p = messageFmt;
        for (; *p != '\0'; p++) {
            if (*p == '%' && p + 1 != nullptr) {
                if (*(p + 1) == '%') {
                    ss << "%";
                    p = p + 1;
                } else {
                    const char *q       = p + 1;
                    bool        FindOpt = false;
                    std::string FilledString;
                    do {
                        if (!detail::basicMsgFmt.count(*q))
                            FilledString.push_back(*q);
                        else if (detail::basicMsgFmt.count(*q)) {
                            if (!argsQueue.empty()) {
                                auto item = argsQueue.front();
                                argsQueue.pop();
                                FormatBasicMsg(FilledString, *q, ss, item);
                            }
                            p       = q++;
                            FindOpt = true;
                            break;
                        }
                    } while (++q);
                    if (!FindOpt)
                        p += 1;
                }
            } else {
                ss << *p;
            }
        }
        return ss.str();
    }

    static void FormatLogMessage(const std::string &key, std::stringstream &ss, const std::string &message, const std::string &levelName, CommandLineInfo &info, const std::string &loggerName,
                                 const std::string &strAppName) {
        if (key == "(message)")
            ss << message;
        else if (key == "(thread)")
            ss << std::hex << pthread_self();
        // ss << std::this_thread::get_id();
        else if (key == "(process)")
            ss << getpid();
        else if (key == "(levelname)")
            ss << levelName;
        else if (key == "(asctime)")
            ss << getFormatTimeString(detail::timeFmt.c_str());
        else if (key == "(ctime)")
            ss << getFormatTimeString(detail::ctime.c_str());
        else if (key == "(lineno)")
            ss << std::dec << info.lineno;
        else if (key == "(filename)")
            ss << info.fileName;
        else if (key == "(funcname)")
            ss << info.funcName;
        else if (key == "(threadname)")
            ss << loggerName;
        else if (key == "(appname)")
            ss << strAppName;
        else if (key == "(user)") {
            uid_t          current_uid = getuid();
            struct passwd *pwd         = getpwuid(current_uid);
            ss << pwd->pw_name;
        }
    }

    static std::string getLogMessageFormat(const char *fmt, const char *message, const char *levelName, CommandLineInfo &info, const std::string &loggername, const std::string &appName) {
        std::stringstream ss;
        const char *      p = fmt;
        for (; *p != '\0'; p++) {
            if (*p == '%' && p + 1 != nullptr) { // search
                if (*(p + 1) == '%') {
                    ss << "%";
                    p = p + 1;
                } else if (*(p + 1) == '(') {
                    bool key_start = true;
                    p++;
                    std::string key;
                    do {
                        key.push_back(*p);
                    } while (*p++ != ')');
                    // std::cout << "type:" << *p << key << std::endl;
                    FormatLogMessage(key, ss, message, levelName, info, loggername, appName);
                }
            } else {
                ss << *p;
            }
        }
        return ss.str();
    }
    static std::ios::openmode getFileMode(const std::string &strMode) {
        std::ios::openmode mode = std::ios::app;
        for (char i : strMode) {
            if (i == 'a')
                mode |= std::ios::app;
            else if (i == 'r')
                mode |= std::ios::in;
            else if (i == 'w')
                mode |= std::ios::out;
            else if (i == '+')
                mode |= std::ios::binary;
        }
        return mode;
    }

    static Level getLevelFromString(const std::string &strLevel) {
        return static_cast<Level>(levelMap[ strLevel ]);
    }
};

class Logger {
public:
    Logger(const std::string &prefix = STDOUT_PREFIX)
        : m_strMessageFmt("%(asctime)s %(levelname)s %(message)s")
        , m_nLevel(Level::DEBUG)
        , m_strLoggerName(prefix) {
    }

    Logger(const std::string &prefix, const std::string &msgfmt, const std::string &filename = "", const std::string &filefmt = "%04Y-%02m-%02d", const std::string &openMode = "a")
        : m_strMessageFmt(msgfmt)
        , m_strLogFileName(filename)
        , m_strAppendFileFmt(filefmt)
        , m_strLoggerName(prefix)
        , m_strAppendMode(openMode) {
        initSetting();
    }

    void setLevel(Level nLevel) {
        m_nLevel = nLevel;
    }

    void BasicConfig(const std::string &msgfmt, const std::string &filename = "", const std::string &filefmt = "%04Y-%02m-%02d", const std::string &openMode = "a") {
        m_strMessageFmt    = msgfmt;
        m_strLogFileName   = filename;
        m_strAppendFileFmt = filefmt;
        m_strAppendMode    = openMode;
        initSetting();
    }

    Logger &setAppName(const std::string &appName = "") {
        m_strAppName = appName;
        return *this;
    }

    Logger &setCommandLineInfo(const std::string &filename, const std::string &funcname, int fileno) {
        m_CommandInfo.fileName = filename;
        m_CommandInfo.funcName = funcname;
        m_CommandInfo.lineno   = fileno;
        return *this;
    }

    virtual ~Logger() {
        if (m_OutStream.is_open())
            m_OutStream.close();
    }

protected:
    void initSetting() {
        if (!m_strLogFileName.empty()) {
            std::string FileName = m_strLogFileName;
            if (!m_strAppendFileFmt.empty()) {
                FileName += "." + detail::getFormatTimeString(m_strAppendFileFmt.c_str());
            }
            if (m_OutStream.is_open())
                m_OutStream.close();
            m_OutStream.open(FileName, detail::getFileMode(m_strAppendMode));
        }
    }

    bool levelCompare(const std::string &levelName) {
        return detail::getLevelFromString(levelName) >= this->m_nLevel;
    }

    void logMessage(const std::string &levelName, const char *msgfmt, std::queue<std::string> &args) {
        std::string message = detail::getFormatMessageString(msgfmt, args);
        message             = detail::getLogMessageFormat(m_strMessageFmt.c_str(), message.c_str(), levelName.c_str(), m_CommandInfo, m_strLoggerName, m_strAppName);
        if (m_OutStream.is_open()) {
            std::lock_guard<std::mutex> guard(m_Lock);
            m_OutStream << message << std::endl;
        } else {
            std::cout << message << std::endl;
        }
    }

protected:
    std::string     m_strMessageFmt;
    std::string     m_strLogFileName;
    std::string     m_strAppendFileFmt;
    CommandLineInfo m_CommandInfo;
    Level           m_nLevel;
    std::string     m_strLoggerName;
    std::ofstream   m_OutStream;
    std::mutex      m_Lock;
    std::string     m_strAppName;
    std::string     m_strAppendMode;

public:
    static Logger stdoutLogger;

public:
    template <typename... Args> void debug(const char *ptrMsg, Args &&... args) {
        if (levelCompare("Debug")) {
            std::queue<std::string> argsQueue;
            detail::ExpandArgs(argsQueue, args...);
            this->logMessage("Debug", ptrMsg, argsQueue);
        }
    }
    template <typename... Args> void info(const char *ptrMsg, Args &&... args) {
        if (levelCompare("Info")) {
            std::queue<std::string> argsQueue;
            detail::ExpandArgs(argsQueue, args...);
            this->logMessage("INFO", ptrMsg, argsQueue);
        }
    }
    template <typename... Args> void warning(const char *ptrMsg, Args &&... args) {
        if (levelCompare("Warning")) {
            std::queue<std::string> argsQueue;
            detail::ExpandArgs(argsQueue, args...);
            this->logMessage("Warning", ptrMsg, argsQueue);
        }
    }
    template <typename... Args> void error(const char *ptrMsg, Args &&... args) {
        if (levelCompare("Error")) {
            std::queue<std::string> argsQueue;
            detail::ExpandArgs(argsQueue, args...);
            this->logMessage("Error", ptrMsg, argsQueue);
        }
    }
    template <typename... Args> void alert(const char *ptrMsg, Args &&... args) {
        if (levelCompare("Alert")) {
            std::queue<std::string> argsQueue;
            detail::ExpandArgs(argsQueue, args...);
            this->logMessage("Alert", ptrMsg, argsQueue);
        }
    }
    template <typename... Args> void emergency(const char *ptrMsg, Args &&... args) {
        if (levelCompare("Emergency")) {
            std::queue<std::string> argsQueue;
            detail::ExpandArgs(argsQueue, args...);
            this->logMessage("Emergency", ptrMsg, argsQueue);
        }
    }
};
} // namespace tlog
class LogImpl {
private:
    static std::map<std::string, tlog::Logger &&> g_LoggerMap;

public:
    static tlog::Logger &getLogger(const std::string &prefix) {
        if (prefix == STDOUT_PREFIX)
            return tlog::Logger::stdoutLogger;
        return g_LoggerMap.at(prefix);
    }
    static void AppendLogger(const std::string &prefix, tlog::Logger &&x) {
        g_LoggerMap.insert(std::pair<std::string, tlog::Logger &&>(prefix, std::forward<tlog::Logger>(x)));
    }
};
#define LOGGER(prefix, appname) LogImpl::getLogger(prefix).setAppName(appname).setCommandLineInfo(__FILE__, __func__, __LINE__)
// #define logger LogImpl::getLogger(STDOUT_PREFIX).setAppName("main").setCommandLineInfo(__FILE__, __func__, __LINE__)
#define AppLogger(name) LOGGER(STDOUT_PREFIX, name)
#define logger LOGGER(STDOUT_PREFIX, "main")
