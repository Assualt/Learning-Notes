#pragma once

#include "Format.h"
#include "LogHandle.h"
#include "Range.h"
#include <iomanip>
#include <iostream>
#include <map>
#include <pwd.h>
#include <sys/time.h>
#include <unistd.h>
#include <vector>
namespace muduo {
namespace base {

class Logger {
public:
    enum LogLevel { Debug, Info, Warn, Error, Fatal, Alert, Emergency };
    class LogImpl {
    public:
        LogImpl(Logger *pLogger, int flushSize = 50) {
            m_nflushSize = flushSize;
            m_pLogger    = pLogger;
        }
        void setLevelAndLogger(LogLevel nLevel) {
            m_nCurrentLevel = nLevel;
        }

        template <class valType> LogImpl &operator<<(const valType &val) {
            m_sout << val;

            std::string message = m_pLogger->MessageFormat(m_sout.str(), m_nCurrentLevel);
            if (message.size() > m_nflushSize) {
                m_pLogger->logMessage(m_nCurrentLevel, message.c_str());
                m_sout.str("");
            }
            return *this;
        }
        ~LogImpl() {
            std::string message = m_pLogger->MessageFormat(m_sout.str(), m_nCurrentLevel);
            std::cout << message << " " << message.size()<< std::endl;
        }

    protected:
        int                m_nflushSize;
        std::ostringstream m_sout;
        LogLevel           m_nCurrentLevel;
        Logger *           m_pLogger;
    };

    struct FileAttribute {
        int         lineno;
        std::string funcname;
        std::string filename;
    };

    Logger &BasicConfig(LogLevel defaultLevel, const char *messageFormat, const char *filePrefix, const char *fileFormat, const char *fileMode = "a+");

    template <class... Args> void debug(const char *messagefmt, Args &&... arg) {
        this->logMessage(Debug, messagefmt, arg...);
    }

    template <class... Args> void info(const char *messagefmt, Args &&... arg) {
        this->logMessage(Info, messagefmt, arg...);
    }

    template <class... Args> void warning(const char *messagefmt, Args &&... arg) {
        this->logMessage(Warn, messagefmt, arg...);
    }

    template <class... Args> void error(const char *messagefmt, Args &&... arg) {
        this->logMessage(Error, messagefmt, arg...);
    }

    template <class... Args> void fatal(const char *messagefmt, Args &&... arg) {
        this->logMessage(Fatal, messagefmt, arg...);
    }

    template <class... Args> void alert(const char *messagefmt, Args &&... arg) {
        this->logMessage(Alert, messagefmt, arg...);
    }

    template <class... Args> void emergency(const char *messagefmt, Args &&... arg) {
        this->logMessage(Emergency, messagefmt, arg...);
    }

    std::string getLevelName(LogLevel nLevel);

    Logger &setFileAttr(const std::string &filename, const std::string &funcname, int lineno);
    Logger &setAppName(const std::string &appname);

    void addLogHandle(LogHandle *au) {
        m_vHandleList.push_back(au);
    }

    LogImpl &log(LogLevel nLevel) {
        if (m_Logger == nullptr)
            m_Logger = new LogImpl(this);
        m_Logger->setLevelAndLogger(nLevel);
        return *m_Logger;
    }

    ~Logger() {
        if (m_Logger)
            delete m_Logger;
        m_Logger = nullptr;
    }

protected:
    std::string getCurrentHourTime(bool showMicroSeconds);
    void        getKeyString(const std::string &key, std::stringstream &ss, const std::string &message, LogLevel nLevel);
    std::string MessageFormat(const std::string &FormattedLogmessage, LogLevel nLevel);

    template <class... Args> void logMessage(LogLevel nLevel, const char *messagefmt, Args &&... arg) {
        if (nLevel < m_nLevel)
            return;
        std::string LogMessage = messagefmt;
        formatString(LogMessage, arg...);
        std::string message = MessageFormat(LogMessage, nLevel);
        message.append("\n");
        for (auto &handle : m_vHandleList) {
            handle->writeData(message.c_str(), message.size());
        }
    }

    template <class T, class... Args> void formatString(std::string &result, T &val, Args &&... arg) {
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
                    if (result[ i ] == 's' || result[ i ] == 'd' || result[ i ] == 'f' || result[ i ] == 'c' || result[ i ] == 'x' || result[ i ] == 'o') {
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
            ss << val;
            ss << result.substr(index + 1);
            result = ss.str();
            formatString(result, arg...);
        }
    }
    void formatString(std::string &result) {
    }

public:
    static Logger &getLogger(const std::string &LoggerName) {
        if (!_MapLogger.count(LoggerName)) {
            _MapLogger[ LoggerName ] = Logger();
        }
        return _MapLogger.at(LoggerName);
    }

private:
    static std::map<std::string, Logger> _MapLogger;

protected:
    LogLevel                 m_nLevel;
    std::string              m_strMessageFormat;
    std::string              m_strAppName;
    FileAttribute            m_FileAttribute;
    std::vector<LogHandle *> m_vHandleList;
    LogImpl *                m_Logger;
};

} // namespace base
} // namespace muduo

// #define Log(Name, App) muduo::base::Logger::getLogger(Name).setFileAttr(__FILE__, __func__, __LINE__).setAppName(App)

// #define logger(Name) Log(Name, __FILE__)

// #define LOG_INFO muduo::base::Logger::getLogger("MAIN").setFileAttr(__FILE__, __func__, __LINE__).setAppName("System").log(muduo::base::Logger::Info)
