//
// Created by 侯鑫 on 2023/12/26.
//

#include "log.h"
#include <exception>
#include <map>
#include <string>
#include "system.h"
#include "timestamp.h"

using namespace ssp::base;
using KeyFormatFunc = std::function<void(std::stringstream &, const std::string &, LogLevel)>;

namespace {
thread_local LogFileAttr g_fileAttr;
std::map<std::string, Logger> g_loggerMap;
std::string g_strDefaultAppName = "APP";
}

Logger & Logger::BasicConfig(LogLevel level, const char *msgFmt, const char *prefix, const char *fmt,
                             const char *)
{
    if (msgFmt == nullptr) {
        throw LogException("msg format is nullptr");
    }
    defaultLevel_ = level;
    msgFmt_ = msgFmt;
    if ((prefix == nullptr) || (fmt == nullptr)) {
        return *this;
    }
    return *this;
}

std::string Logger::GetLevelName(LogLevel level)
{
    static std::map<LogLevel, std::string> levelMap = {
        {LogLevel::Debug, "Debug"}, {LogLevel::Info, "Info"}, {LogLevel::Warn, "Warning"},
        {LogLevel::Error, "Error"}, {LogLevel::Alert, "Alert"}, {LogLevel::Fatal, "Fatal"},
        {LogLevel::Emergency, "Emergency"}, {LogLevel::Except, "Exception"}};

    if (levelMap.count(level)) {
        return levelMap[ level ];
    }
    return levelMap[ LogLevel::Debug ];
}

void Logger::GetKeyVal(const std::string &key, std::stringstream &ss, const std::string &msg, LogLevel level)
{
    static std::map<std::string, KeyFormatFunc> funcMap = {
            {"(message)", [](auto &ss, auto &msg, auto) { ss << msg; }},
            {"(thread)", [](auto &ss, auto &msg, auto) { ss << std::hex << pthread_self(); }},
            {"(tid)", [](auto &ss, auto &msg, auto) { ss << System::Tid(); }},
            {"(process)", [](auto &ss, auto &msg, auto) { ss << System::Pid(); }},
            {"(levelname)", [](auto &ss, auto &, auto level) { ss << GetLevelName(level); }},
            {"(asctime)", [](auto &ss, auto &msg, auto) { ss << TimeStamp::Now().ToFmtString("%H:%M:%S.%k"); }},
            {"(ctime)", [](auto &ss, auto &msg, auto) { ss << TimeStamp::Now().ToString(); }},
            {"(lineno)", [](auto &ss, auto &msg, auto) { ss << std::dec << g_fileAttr.lineNo; }},
            {"(filename)", [](auto &ss, auto &msg, auto) { ss << g_fileAttr.fileName; }},
            {"(funcname)", [](auto &ss, auto &msg, auto) { ss << g_fileAttr.funcName; }},
            {"(threadName)", [](auto &ss, auto &msg, auto) { ss << System::GetCurrentThreadName(); }},
            {"(appname)",
                    [this](auto &ss, auto &msg, auto) {
                        ss << (appName_.empty() ? g_strDefaultAppName : appName_);
                        appName_.clear();
                    }},
            {"(uname)", [](auto &ss, auto &msg, auto) { ss << System::GetUname(); }},
    };

    auto itr = funcMap.find(key);
    if (itr == funcMap.end()) {
        return;
    }
    std::invoke(itr->second, ss, msg, level);
}

std::string Logger::MessageFormat(const std::string &fmtLogMsg, ssp::base::LogLevel level)
{
    std::stringstream ss;
    std::string       strKey;
    bool              findKey = false;
    for (size_t i = 0; i < msgFmt_.size(); ++i) {
        if (findKey) {
            if (msgFmt_[ i ] != ')') {
                strKey.push_back(msgFmt_[i]);
                continue;
            }

            strKey.push_back(')');
            findKey = false;
            GetKeyVal(strKey, ss, fmtLogMsg, level);
            strKey.clear();
            continue;
        } else if (msgFmt_[ i ] == '%' && i + 1 < msgFmt_.size() && msgFmt_[ i + 1 ] == '(') {
            strKey.push_back('(');
            findKey = true;
            i++;
            continue;
        }

        ss << msgFmt_[ i ];
    }
    if (!strKey.empty()) {
        GetKeyVal(strKey, ss, fmtLogMsg, level);
    }
    return ss.str();
}

void Logger::SetFileAttr(const std::string &fileName, const std::string &funcName, uint32_t lineno)
{
    g_fileAttr.fileName = fileName;
    size_t pos = g_fileAttr.fileName.rfind('/');
    if (pos != std::string::npos) {
        g_fileAttr.fileName = g_fileAttr.fileName.substr(pos + 1);
    }
    g_fileAttr.funcName = funcName;
    g_fileAttr.lineNo = lineno;
}

Logger &Logger::GetLogger(const std::string &name)
{
    std::string prefix = name;
    if (name.empty()) {
        prefix = "APP";
    }
    if (g_loggerMap.find(prefix) == g_loggerMap.end()) {
        g_loggerMap[ prefix ] = Logger();
    }
    return g_loggerMap.at(prefix);
}

Logger &Logger::SetAppName(const std::string &appName)
{
    appName_ = appName;
    return *this;
}

void Logger::AddLogHandle(LogImpl *au)
{
    logImplList_.push_back(au);
}