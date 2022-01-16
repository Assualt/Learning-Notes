#include "LogHandle.h"
#include "Logging.h"
#include "Os.h"
#include <memory>
#include <syscall.h>
namespace muduo {
namespace base {

static std::string g_detaultAppName = "main";

string strip_filename(const std::string &filename) {
    if (filename.rfind("/") != std::string::npos) {
        return filename.substr(filename.rfind("/") + 1);
    }
    return filename;
}

std::map<std::string, Logger> Logger::_MapLogger;

Logger &Logger::BasicConfig(LogLevel defaultLevel, const char *messageFormat, const char *filePrefix, const char *fileFormat, const char *fileMode) {
    m_strMessageFormat = messageFormat;
    m_nLevel           = defaultLevel;
    return *this;
}

std::string Logger::MessageFormat(const std::string &FormattedLogmessage, LogLevel nLevel) {
    std::stringstream ss;
    std::string       strKey;
    bool              bFindKey = false;
    for (size_t i = 0; i < m_strMessageFormat.size(); ++i) {
        if (bFindKey) {
            if (m_strMessageFormat[ i ] != ')')
                strKey.push_back(m_strMessageFormat[ i ]);
            else {
                strKey.push_back(')');
                bFindKey = false;
                getKeyString(strKey, ss, FormattedLogmessage, nLevel);
                strKey.clear();
            }
        } else if (!bFindKey && m_strMessageFormat[ i ] == '%' && i + 1 < m_strMessageFormat.size() && m_strMessageFormat[ i + 1 ] == '(') {
            strKey.push_back('(');
            bFindKey = true;
            i++;
        } else {
            ss << m_strMessageFormat[ i ];
        }
    }
    if (!strKey.empty()) {
        getKeyString(strKey, ss, FormattedLogmessage, nLevel);
    }
    return ss.str();
}

std::string Logger::getLevelName(LogLevel nLevel) {
    switch (nLevel) {
        case Debug:
            return "Debug";
        case Info:
            return "Info";
        case Warn:
            return "Warning";
        case Error:
            return "Error";
        case Alert:
            return "Alert";
        case Fatal:
            return "Fatal";
        case Emergency:
            return "Emergency";
    }
    return "Debug";
}

std::string Logger::getCurrentHourTime(bool showMicroSeconds) {
    time_t            tNow(time(nullptr));
    struct tm *       t = localtime(&tNow);
    std::stringstream ss;
    ss << std::setfill('0') << std::setw(2) << t->tm_hour << ":" << std::setfill('0') << std::setw(2) << t->tm_min << ":" << std::setfill('0') << std::setw(2) << t->tm_sec;
    if (showMicroSeconds) {
        struct timeval tv;
        gettimeofday(&tv, nullptr);
        ss << "." << std::setfill('0') << std::setw(5) << tv.tv_usec;
    }
    return ss.str();
}

void Logger::getKeyString(const std::string &key, std::stringstream &ss, const std::string &message, LogLevel nLevel) {
    if (key == "(message)") {
        ss << message;
    } else if (key == "(thread)")
        ss << std::hex << pthread_self();
    else if (key == "(tid)") {
        ss << syscall(SYS_gettid);
    } else if (key == "(process)") {
        ss << getpid();
    } else if (key == "(levelname)") {
        ss << getLevelName(nLevel);
    } else if (key == "(asctime)") {
        ss << getCurrentHourTime(true);
    } else if (key == "(ctime)") {
        time_t t(time(nullptr));
        char * timeBuffer = ctime(&t);
        ss << std::string(timeBuffer, strlen(timeBuffer) - 1);
    } else if (key == "(lineno)")
        ss << std::dec << m_FileAttribute.lineno;
    else if (key == "(filename)") {
        ss << strip_filename(m_FileAttribute.filename);
    } else if (key == "(funcname)") {
        ss << m_FileAttribute.funcname;
    } else if (key == "(threadname)") {
        ss << System::GetCurrentThreadName();
    } else if (key == "(appname)") {
        if (!m_strAppName.empty()) {
            ss << m_strAppName;
            m_strAppName.clear();
        } else {
            ss << g_detaultAppName;
        }
    } else if (key == "(user)") {
        uid_t          current_uid = getuid();
        struct passwd *pwd         = getpwuid(current_uid);
        ss << pwd->pw_name;
    }
}

Logger &Logger::setFileAttr(const std::string &filename, const std::string &funcname, int lineno) {
    m_FileAttribute.filename = filename;
    m_FileAttribute.funcname = funcname;
    m_FileAttribute.lineno   = lineno;
    return *this;
}

Logger &Logger::setAppName(const std::string &appname) {
    m_strAppName = appname;
    return *this;
}

} // namespace base
} // namespace muduo
