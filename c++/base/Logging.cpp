#include "Logging.h"
#include "System.h"
#include "Timestamp.h"
#include <functional>
#include <memory>

using namespace muduo;
using namespace muduo::base;

namespace {
static const std::string g_defaultAppName = "main";

std::string getCurrentHourTime() {
    Timestamp now = Timestamp::now();

    return now.toFmtString("%H:%M:%S.%k");
}

} // namespace
std::map<std::string, Logger> Logger::mapLogger_;

std::string stripFileName(const std::string &filename) {
    if (filename.rfind('/') == std::string::npos) {
        return filename;
    }
    return filename.substr(filename.rfind('/') + 1);
}

Logger &Logger::BasicConfig(LogLevel defaultLevel, const char *messageFormat, const char *filePrefix,
                            const char *fileFormat, const char *) {
    if (messageFormat == nullptr) {
        throw LogException("invalid message format");
    }
    m_nLevel    = defaultLevel;
    m_strMsgFmt = messageFormat;
    if ((filePrefix == nullptr) || (fileFormat == nullptr)) {
        return *this;
    }
    return *this;
}

std::string Logger::MessageFormat(const std::string &fmtLogMsg, LogLevel nLevel) {
    std::stringstream ss;
    std::string       strKey;
    bool              bFindKey = false;
    for (size_t i = 0; i < m_strMsgFmt.size(); ++i) {
        if (bFindKey) {
            if (m_strMsgFmt[ i ] != ')')
                strKey.push_back(m_strMsgFmt[ i ]);
            else {
                strKey.push_back(')');
                bFindKey = false;
                getKeyVal(strKey, ss, fmtLogMsg, nLevel);
                strKey.clear();
            }
        } else if (m_strMsgFmt[ i ] == '%' && i + 1 < m_strMsgFmt.size() && m_strMsgFmt[ i + 1 ] == '(') {
            strKey.push_back('(');
            bFindKey = true;
            i++;
        } else {
            ss << m_strMsgFmt[ i ];
        }
    }
    if (!strKey.empty()) {
        getKeyVal(strKey, ss, fmtLogMsg, nLevel);
    }
    return ss.str();
}

std::string Logger::getLevelName(LogLevel nLevel) {
    static std::map<LogLevel, std::string> levelMap = {{Debug, "Debug"},        {Info, "Info"},   {Warn, "Warning"},
                                                       {Error, "Error"},        {Alert, "Alert"}, {Fatal, "Fatal"},
                                                       {Emergency, "Emergency"}};
    if (levelMap.count(nLevel)) {
        return levelMap[ nLevel ];
    }
    return levelMap[ Debug ];
}

using KeyFormatFunc = std::function<void(std::stringstream &, const std::string &, LogLevel)>;

void Logger::getKeyVal(const std::string &key, std::stringstream &ss, const std::string &message, LogLevel nLevel) {
    static std::map<std::string, KeyFormatFunc> keyMap = {
        {"(message)", [](auto &ss, auto &msg, auto) { ss << msg; }},
        {"(thread)", [](auto &ss, auto &msg, auto) { ss << std::hex << pthread_self(); }},
        {"(tid)", [](auto &ss, auto &msg, auto) { ss << System::Tid(); }},
        {"(process)", [](auto &ss, auto &msg, auto) { ss << System::Pid(); }},
        {"(levelname)", [](auto &ss, auto &, auto level) { ss << getLevelName(level); }},
        {"(asctime)", [](auto &ss, auto &msg, auto) { ss << getCurrentHourTime(); }},
        {"(ctime)", [](auto &ss, auto &msg, auto) { ss << Timestamp::now().toFmtString(); }},
        {"(lineno)", [ this ](auto &ss, auto &msg, auto) { ss << std::dec << m_FileAttribute.lineNo; }},
        {"(filename)", [ this ](auto &ss, auto &msg, auto) { ss << stripFileName(m_FileAttribute.fileName); }},
        {"(funcname)", [ this ](auto &ss, auto &msg, auto) { ss << m_FileAttribute.funcName; }},
        {"(threadName)", [](auto &ss, auto &msg, auto) { ss << System::GetCurrentThreadName(); }},
        {"(appname)",
         [ this ](auto &ss, auto &msg, auto) {
             ss << (m_strAppName.empty() ? g_defaultAppName : m_strAppName);
             m_strAppName.clear();
         }},
        {"(threadName)", [](auto &ss, auto &msg, auto) { ss << getpwuid(getuid())->pw_name; }},
    };

    auto itr = keyMap.find(key);
    if (itr != keyMap.end()) {
        itr->second(ss, message, nLevel);
    }
}

Logger &Logger::setFileAttr(const std::string &fileName, const std::string &funcName, int lineNo) {
    m_FileAttribute.fileName = fileName;
    m_FileAttribute.funcName = funcName;
    m_FileAttribute.lineNo   = lineNo;
    return *this;
}

Logger &Logger::setAppName(const std::string &appName) {
    m_strAppName = appName;
    return *this;
}

Logger &Logger::getLogger(const std::string &name) {
    string prefix = name;
    if (name.empty()) {
        prefix = APP;
    }
    if (mapLogger_.find(prefix) == mapLogger_.end()) {
        mapLogger_[ prefix ] = Logger();
    }
    return mapLogger_.at(prefix);
}