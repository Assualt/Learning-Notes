
#pragma once
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <mutex>
#include <queue>
#include <set>
#include <sstream>
#include <stdio.h>
#include <sys/time.h>
#include <thread>
#include <unistd.h>
#include <vector>
using namespace std;

namespace tlog {
struct detail {
    enum Level { DEBUG, INFO, WARNING, ERROR, ALERT, EMERGENCY };
    static std::vector<std::string> levelVec;

    struct CommandLineInfo {
        size_t      lineno;
        const char *funcname;
        const char *file;
    };
    static std::map<std::string, int> levelMap;
    static std::string                endl;
    static std::string                asctime;
    static std::string                ctime;
    static std::set<char>             timeOpt;
    static std::vector<std::string>   WeekDayEngList;
    static std::vector<std::string>   MonthEngList;
    static std::set<std::string>      MessageOperation;
    static std::set<char>             basicMsgFmt;

    static void FormatTimeString(struct tm *t, struct timeval *tv, const std::string &FilledString, const char op, std::stringstream &ss) {
        char FilledChar;
        int  FilledLength = 0;
        bool Find_start   = false;
        for (int i = 0; i < FilledString.size(); i++) {
            if (FilledString[ i ] >= '0' && FilledString[ i ] <= '9' && Find_start) {
                FilledLength = FilledLength * 10 + FilledString[ i ] - '0';
            } else if (FilledString[ i ] > '0' && FilledString[ i ] <= '9') {
                FilledLength = FilledLength * 10 + FilledString[ i ] - '0';
                Find_start   = true;
            } else if (!Find_start) {
                FilledChar = FilledString[ i ];
            }
        }
        // std::cout << "Filed With:" << FilledChar << " length:" << FilledLength << std::endl;
        if (op == 'T') {
            // ISO String
            ss << detail::WeekDayEngList[ t->tm_wday ] << " ";
            ss << detail::MonthEngList[ t->tm_mon ] << " ";
            ss << setw(2) << setfill('0') << (t->tm_mday) << " ";
            ss << setw(2) << setfill('0') << (t->tm_hour) << ":";
            ss << setw(2) << setfill('0') << (t->tm_min) << ":";
            ss << setw(2) << setfill('0') << (t->tm_sec) << " ";
            ss << "CST " << setw(4) << (t->tm_year + 1900);
            return;
        }
        if (FilledLength != 0)
            ss << setw(FilledLength);
        if (FilledChar)
            ss << setfill(FilledChar);
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

    static std::string getFormatTimeString(const char *timeFmt) {
        if (nullptr == timeFmt)
            return "";
        struct timeval tv;
        gettimeofday(&tv, NULL);
        time_t            ct = time(NULL);
        struct tm *       t  = localtime(&ct);
        std::stringstream ss;

        const char *p = timeFmt;
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
                        if (!detail::timeOpt.count(*q))
                            FilledString.push_back(*q);
                        else if (detail::timeOpt.count(*q)) { // Found Time Opt
                            FormatTimeString(t, &tv, FilledString, *q, ss);
                            p       = q++;
                            FindOpt = true;
                            break;
                        }
                    } while (++q);
                    if (FindOpt == false)
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
        char FilledChar        = ' ';
        int  FilledLength      = 0;
        bool Find_start        = false;
        bool Find_Pointer      = false;
        int  FilledWithPointer = 0;
        int  FilledWithZS      = 0;
        for (int i = 0; i < FilledString.size(); i++) {
            if (FilledString[ i ] >= '0' && FilledString[ i ] <= '9' && Find_start && !Find_Pointer) {
                FilledLength = FilledLength * 10 + FilledString[ i ] - '0';
            } else if (FilledString[ i ] > '0' && FilledString[ i ] <= '9' && !Find_start && !Find_Pointer) {
                FilledLength = FilledLength * 10 + FilledString[ i ] - '0';
                Find_start   = true;
            } else if (FilledString[ i ] == '.') {
                Find_Pointer = true;
                Find_start   = false;
            } else if (FilledString[ i ] >= '0' && FilledString[ i ] <= '9' && Find_Pointer)
                FilledWithPointer = FilledWithPointer * 10 + FilledString[ i ] - '0';
            else if (!Find_start)

                FilledChar = FilledString[ i ];
        }
        FilledWithZS = FilledLength;
        // std::cout << FilledWithPointer << ":" << FilledLength << std::endl;
        FilledLength += Find_Pointer ? 1 + FilledWithPointer : 0;

        if (FilledLength != 0 && op != 'f')
            ss << setw(FilledLength);
        if (FilledChar && op != 'f')
            ss << setfill(FilledChar);
        // std::cout << "Filed With:" << FilledChar << " length:" << FilledLength << " FilledString:" <<
        // FilledString << std::endl;
        if (op == 's' || op == 'd') {
            ss << item;
        } else if (op == 'f') {
            if (!FilledLength) {
                ss << item;
            } else {
                if (item.find(".") != std::string::npos) {
                    std::string zs = item.substr(0, item.find("."));
                    if (!FilledWithZS)
                        ss << zs;
                    else if (zs.size() > FilledWithZS) {
                        ss << zs.substr(zs.length() - FilledWithZS);
                    } else if (zs.size() == FilledWithZS) {
                        ss << zs;
                    } else {
                        ss << setfill('0') << setw(FilledWithZS) << zs;
                    }
                    ss << ".";
                    std::string pointer = item.substr(item.find(".") + 1);
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
                    else
                        ss << item.substr(item.size() - FilledWithZS);
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
                    if (FindOpt == false)
                        p += 1;
                }
            } else {
                ss << *p;
            }
        }
        return ss.str();
    }

    static void FormatLogMessage(const std::string &key, char type, std::stringstream &ss, const std::string &message, const std::string &levelName, const detail::CommandLineInfo &info) {
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
            ss << getFormatTimeString(detail::asctime.c_str());
        else if (key == "(ctime)")
            ss << getFormatTimeString(detail::ctime.c_str());
        else if (key == "(lineno)")
            ss << std::dec << info.lineno;
        else if (key == "(filename)")
            ss << info.file;
        else if (key == "(funcName)")
            ss << info.funcname;
    }

    static std::string getLogMessageFormat(const char *fmt, const char *message, const char *levelName, const detail::CommandLineInfo &info) {
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
                    FormatLogMessage(key, *p, ss, message, levelName, info);
                }
            } else {
                ss << *p;
            }
        }
        return ss.str();
    }
};
class Logger : public std::ostream {
public:
    static Logger &getInstance() {
        return m_Logger;
    }

public:
    void initSetting(const char *messageFmt, const char *timeFmt, const std::string &fileName, const std::string &fileMode) {
        this->m_strMessageFmt = messageFmt;
        this->m_strTimeFmt    = timeFmt;
        this->m_strFileName   = fileName + "." + detail::getFormatTimeString("%04Y-%02m-%02d");
        this->m_strFileMode   = fileMode;
        if (!this->m_OutputStream.is_open() && !fileName.empty()) {
            m_lock.lock();
            fprintf(stderr, "try to open %s for output \n", this->m_strFileName.c_str());
            this->m_OutputStream.open(this->m_strFileName, std::ios::app);
            if (!m_OutputStream.is_open()) {
                fprintf(stderr, "try to open %s for output failed\n", this->m_strFileName.c_str());
            }
            m_lock.unlock();
        }
    }
    ~Logger() {
        if (this->m_OutputStream.is_open() && !this->m_strFileName.empty())
            this->m_OutputStream.close();
    }
    template <class T> friend Logger &operator<<(Logger &os, T &val) {
        if (os.m_OutputStream.is_open()) {
            os.m_lock.lock();
            os.m_OutputStream << val;
            os.m_lock.unlock();
        }
        std::cout << "val:" << val;
        return os;
    }

    void logMessage(const std::string &levelName, const std::string &messagefmt, std::queue<std::string> &argQueue, bool eofWithNext = true) {
        std::string MessageString = detail::getFormatMessageString(messagefmt.c_str(), argQueue);
        std::string LogMessage    = detail::getLogMessageFormat(m_strMessageFmt.c_str(), MessageString.c_str(), levelName.c_str(), info);
        if (m_OutputStream.is_open()) {
            m_lock.lock();
            m_OutputStream << LogMessage;
            if (eofWithNext)
                m_OutputStream << "\n";
            m_lock.unlock();
        }
        std::cout << LogMessage << std::endl;
    }

    Logger &setFileStructInfo(const char *file, const char *funcName, int fileNo) {
        m_lock.lock();
        info.file     = file;
        info.lineno   = fileNo;
        info.funcname = funcName;
        m_lock.unlock();
        return *this;
    }

public:
    static std::mutex m_lock;
    static Logger     m_Logger;

private:
    std::string   m_strMessageFmt;
    std::string   m_strTimeFmt;
    std::string   m_strFileName;
    std::string   m_strFileMode;
    std::ofstream m_OutputStream;

    // FILE Strace
    detail::CommandLineInfo info;
};

class logImpl {
private:
    bool levleCompare(const std::string &levleName) {
        return this->nLevel <= detail::levelMap[ levleName ];
    }

public:
    template <typename... Args> void debug(const char *ptrMsg, Args &&... args) {
        if (levleCompare("Debug")) {
            std::queue<std::string> argsQueue;
            detail::ExpandArgs(argsQueue, args...);
            Logger::getInstance().logMessage("Debug", ptrMsg, argsQueue);
        }
    }
    template <typename... Args> void info(const char *ptrMsg, Args &&... args) {
        if (levleCompare("Info")) {
            std::queue<std::string> argsQueue;
            detail::ExpandArgs(argsQueue, args...);
            Logger::getInstance().logMessage("Info", ptrMsg, argsQueue);
        }
    }
    template <typename... Args> void warning(const char *ptrMsg, Args &&... args) {
        if (levleCompare("Warning")) {
            std::queue<std::string> argsQueue;
            detail::ExpandArgs(argsQueue, args...);
            Logger::getInstance().logMessage("Warning", ptrMsg, argsQueue);
        }
    }
    template <typename... Args> void error(const char *ptrMsg, Args &&... args) {
        if (levleCompare("Error")) {
            std::queue<std::string> argsQueue;
            detail::ExpandArgs(argsQueue, args...);
            Logger::getInstance().logMessage("Error", ptrMsg, argsQueue);
        }
    }
    template <typename... Args> void alert(const char *ptrMsg, Args &&... args) {
        if (levleCompare("Alert")) {
            std::queue<std::string> argsQueue;
            detail::ExpandArgs(argsQueue, args...);
            Logger::getInstance().logMessage("Alert", ptrMsg, argsQueue);
        }
    }

    template <typename... Args> void emergency(const char *ptrMsg, Args &&... args) {
        if (levleCompare("Emergency")) {
            std::queue<std::string> argsQueue;
            detail::ExpandArgs(argsQueue, args...);
            Logger::getInstance().logMessage("Emergency", ptrMsg, argsQueue);
        }
    }
    template <class T> friend logImpl &operator<<(logImpl &s, T &val) {
        std::queue<std::string> args;
        Logger::getInstance().logMessage(s.levelName, val, args, true);
        return s;
    }

public:
    void BasicConfig(const char *messageFmt, const char *timeFmt, const std::string &fileName = "", const std::string &fileMode = "a", detail::Level nLevel = detail::DEBUG) {
        if (nullptr == messageFmt || nullptr == timeFmt)
            throw std::invalid_argument("invalid message Format or time Format");
        Logger::getInstance().initSetting(messageFmt, timeFmt, fileName, fileMode);
        this->setLevel(nLevel);
    }
    logImpl() {
        Logger::getInstance().initSetting("%(levelname)s %(ctime)s %(message)s", "%Y-%m-%d %H:%M:%S,%s", "", "a");
        this->setLevel(detail::DEBUG);
    }
    void setLevel(detail::Level nLevel) {
        this->setLevelName(detail::levelVec[ static_cast<int>(nLevel) ]);
    }
    logImpl &setFileStructInfo(const std::string &file, const std::string &func, int fileno) {
        Logger::getInstance().setFileStructInfo(file.c_str(), func.c_str(), fileno);
        return *this;
    }

private:
    void setLevelName(const std::string &levelName) {
        this->levelName = levelName;
        this->nLevel    = detail::levelMap.at(this->levelName);
    }

private:
    std::string levelName;
    int         nLevel;

public:
    static logImpl LogObject;
};
} // namespace tlog

#define logger tlog::logImpl::LogObject.setFileStructInfo(__FILE__, __func__, __LINE__)
