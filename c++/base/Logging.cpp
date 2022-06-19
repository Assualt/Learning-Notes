#include "Logging.h"
#include "LogHandle.h"
#include "System.h"
#include <functional>
#include <memory>

using namespace muduo;
using namespace muduo::base;

static std::string            g_detaultAppName = "main";
std::map<std::string, Logger> Logger::_MapLogger;

string strip_filename(const std::string &filename) {
    if (filename.rfind("/") == std::string::npos) {
        return filename;
    }
    return filename.substr(filename.rfind("/") + 1);
}

Logger &Logger::BasicConfig(LogLevel defaultLevel, const char *messageFormat, const char *filePrefix, const char *fileFormat, const char *fileMode) {
    if (messageFormat == nullptr) {
        throw LogException("invalid message format");
    }
    m_nLevel           = defaultLevel;
    m_strMessageFormat = messageFormat;
    if ((filePrefix == nullptr) || (fileFormat == nullptr)) {
        return *this;
    }
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
    static std::map<LogLevel, std::string> levelMap = {{Debug, "Debug"}, {Info, "Info"}, {Warn, "Warning"}, {Error, "Error"}, {Alert, "Alert"}, {Fatal, "Fatal"}, {Emergency, "Emergency"}};
    if (levelMap.count(nLevel)) {
        return levelMap[ nLevel ];
    }
    return levelMap[ Debug ];
}

std::string Logger::getCurrentHourTime(bool showMicroSeconds) {
    time_t            tNow(time(nullptr));
    struct tm        *t = localtime(&tNow);
    std::stringstream ss;
    ss << std::setfill('0') << std::setw(2) << t->tm_hour << ":" << std::setfill('0') << std::setw(2) << t->tm_min << ":" << std::setfill('0') << std::setw(2) << t->tm_sec;
    if (!showMicroSeconds) {
        return ss.str();
    }
    struct timeval tv;
    gettimeofday(&tv, nullptr);
    ss << "." << std::setw(5) << std::setfill('0') << tv.tv_usec;
    return ss.str();
}

using KeyFormatFunc = std::function<void(std::stringstream &, const std::string &, LogLevel)>;

void Logger::getKeyString(const std::string &key, std::stringstream &ss, const std::string &message, LogLevel nLevel) {
    static std::map<std::string, KeyFormatFunc> keyMap = {
        {"(message)", [](std::stringstream &ss, const std::string &msg, LogLevel) { ss << msg; }},
        {"(thread)", [](std::stringstream &ss, const std::string &msg, LogLevel) { ss << std::hex << pthread_self(); }},
        {"(tid)", [](std::stringstream &ss, const std::string &msg, LogLevel) { ss << System::Tid(); }},
        {"(process)", [](std::stringstream &ss, const std::string &msg, LogLevel) { ss << System::Pid(); }},
        {"(levelname)", [ this ](std::stringstream &ss, const std::string &, LogLevel level) { ss << getLevelName(level); }},
        {"(asctime)", [ this ](std::stringstream &ss, const std::string &msg, LogLevel) { ss << getCurrentHourTime(true); }},
        {"(ctime)",
         [ this ](std::stringstream &ss, const std::string &msg, LogLevel) {
             Timestamp t = Timestamp::now();
             ss << t.toFormattedString();
         }},
        {"(lineno)", [ this ](std::stringstream &ss, const std::string &msg, LogLevel) { ss << std::dec << m_FileAttribute.lineno; }},
        {"(filename)", [ this ](std::stringstream &ss, const std::string &msg, LogLevel) { ss << strip_filename(m_FileAttribute.filename); }},
        {"(funcname)", [ this ](std::stringstream &ss, const std::string &msg, LogLevel) { ss << m_FileAttribute.funcname; }},
        {"(threadName)", [ this ](std::stringstream &ss, const std::string &msg, LogLevel) { ss << System::GetCurrentThreadName(); }},
        {"(appname)",
         [ this ](std::stringstream &ss, const std::string &msg, LogLevel) {
             if (!m_strAppName.empty()) {
                 ss << m_strAppName;
                 m_strAppName.clear();
             } else {
                 ss << g_detaultAppName;
             }
         }},
        {"(threadName)",
         [ this ](std::stringstream &ss, const std::string &msg, LogLevel) {
             uid_t          current_uid = getuid();
             struct passwd *pwd         = getpwuid(current_uid);
             ss << pwd->pw_name;
         }},
    };

    auto itr = keyMap.find(key);
    if (itr != keyMap.end()) {
        itr->second(ss, message, nLevel);
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
