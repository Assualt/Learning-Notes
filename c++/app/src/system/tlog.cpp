//
// Created by Sunshine on 2020/8/30.
//
#include "system/tlog.h"

NAMESPACE_BEGIN

std::map<std::string, Logger *> logImpl::g_LoggerMap;

namespace detail {
std::map<std::string, int> levelMap = {
        {"Debug", 0},  // NOLINT
        {"Info", 1},
        {"Warning", 2},
        {"Error", 3},
        {"Alert", 4},
        {"Emergency", 5}};
void FormatTimeString(
        struct tm *t,
        struct timeval *tv,
        const std::string &FilledString,
        const char op,
        std::stringstream &ss) {
    char FilledChar;
    int FilledLength = 0;
    bool Find_start = false;
    for (char i : FilledString) {
        if (i >= '0' && i <= '9' && Find_start) {
            FilledLength = FilledLength * 10 + i - '0';
        } else if (i > '0' && i <= '9') {
            FilledLength = FilledLength * 10 + i - '0';
            Find_start = true;
        } else if (!Find_start) {
            FilledChar = i;
        }
    }
    if (op == 'T') {
        // ISO String
        ss << detail::WeekDayEngList[t->tm_wday] << " ";
        ss << detail::MonthEngList[t->tm_mon] << " ";
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
        ss << detail::WeekDayEngList[t->tm_wday];
    else if (op == 'n')
        ss << (tv->tv_usec / 1000);
    else if (op == 'u')
        ss << (tv->tv_usec);
    else if (op == 'Z')
        ss << '2';
    else if (op == 'a')
        ss << (t->tm_hour / 12 >= 1 ? "PM." : "AM.") << (t->tm_hour % 12);
}

std::string getFormatTimeString(const char *strFmt) {
    if (nullptr == strFmt)
        return "";
    struct timeval tv {};
    gettimeofday(&tv, nullptr);
    time_t ct = time(nullptr);
    struct tm *t = localtime(&ct);
    std::stringstream ss;

    const char *p = strFmt;
    for (; (*p) != '\0'; p++) {
        if (('%' == *p) && (p + 1 != nullptr))  // find %
        {
            if (*(p + 1) == '%') {
                ss << "%";
                p = p + 1;
            } else {
                const char *q = p + 1;
                bool FindOpt = false;
                std::string FilledString;
                do {
                    if (!detail::timeOperator.count(*q))
                        FilledString.push_back(*q);
                    else if (detail::timeOperator.count(*q)) {  // Found Time Opt
                        FormatTimeString(t, &tv, FilledString, *q, ss);
                        p = q++;
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

void FormatBasicMsg(const std::string &FilledString, const char op, std::stringstream &ss, const std::string &item) {
    char FilledChar = ' ';
    size_t FilledLength = 0;
    bool Find_start = false;
    bool Find_Pointer = false;
    size_t FilledWithPointer = 0;
    size_t FilledWithZS;
    for (char i : FilledString) {
        if (i >= '0' && i <= '9' && Find_start && !Find_Pointer) {
            FilledLength = FilledLength * 10 + i - '0';
        } else if (i > '0' && i <= '9' && !Find_start && !Find_Pointer) {
            FilledLength = FilledLength * 10 + i - '0';
            Find_start = true;
        } else if (i == '.') {
            Find_Pointer = true;
            Find_start = false;
        } else if (i >= '0' && i <= '9' && Find_Pointer)
            FilledWithPointer = FilledWithPointer * 10 + i - '0';
        else if (!Find_start)

            FilledChar = i;
    }
    FilledWithZS = FilledLength;
    FilledLength += Find_Pointer ? 1 + FilledWithPointer : 0;
    if (FilledLength != 0 && op != 'f')
        ss << setw(FilledWithZS);
    if (FilledChar && op != 'f')
        ss << setfill(FilledChar);
    if (op == 's' || op == 'd') {
        ss << item;
    } else if (op == 'f') {
        if (!FilledLength) {
            ss << item;
        } else {
            if (item.find('.') != std::string::npos) {
                std::string zs = item.substr(0, item.find('.'));
                if (!FilledWithZS)
                    ss << zs;  // NOLINT
                else if (zs.size() > FilledWithZS) {
                    ss << zs.substr(zs.length() - FilledWithZS);
                } else if (zs.size() == FilledWithZS) {
                    ss << zs;
                } else {
                    ss << setfill('0') << setw(FilledWithZS) << zs;
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

std::string getFormatMessageString(const char *messageFmt, std::queue<std::string> &argsQueue) {
    std::stringstream ss;
    const char *p = messageFmt;
    for (; *p != '\0'; p++) {
        if (*p == '%' && p + 1 != nullptr) {
            if (*(p + 1) == '%') {
                ss << "%";
                p = p + 1;
            } else {
                const char *q = p + 1;
                bool FindOpt = false;
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
                        p = q++;
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

void FormatLogMessage(
        const std::string &key,
        std::stringstream &ss,
        const std::string &message,
        const std::string &levelName,
        const detail::commandLineInfo &info,
        const std::string &loggerName) {
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
        ss << getFormatTimeString(detail::timeFmt);
    else if (key == "(ctime)")
        ss << getFormatTimeString(detail::ctime);
    else if (key == "(lineno)")
        ss << std::dec << info.lineno;
    else if (key == "(filename)")
        ss << info.fileName;
    else if (key == "(funcname)")
        ss << info.funcName;
    else if (key == "(threadname)")
        ss << loggerName;
}

std::string getLogMessageFormat(
        const char *fmt,
        const char *message,
        const char *levelName,
        const detail::commandLineInfo &info,
        const std::string &loggerName) {
    if (nullptr == fmt || nullptr == message || nullptr == levelName)
        return "";

    std::stringstream ss;
    const char *p = fmt;
    for (; *p != '\0'; p++) {
        if (*p == '%' && p + 1 != nullptr) {  // search
            if (*(p + 1) == '%') {
                ss << "%";
                p = p + 1;
            } else if (*(p + 1) == '(') {
                p++;
                std::string key;
                do {
                    key.push_back(*p);
                } while (*p++ != ')');
                FormatLogMessage(key, ss, message, levelName, info, loggerName);
            }
        } else {
            ss << *p;
        }
    }
    return ss.str();
}

std::ios::openmode getFileMode(const std::string &strMode) {
    std::ios::openmode mode;
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
Level getLevelFromString(const std::string &strLevel) {
    return static_cast<Level>(levelMap[strLevel]);
}

}  // namespace detail

NAMESPACE_END