#include "Timestamp.h"
#include "Format.h"
#include <algorithm>
#include <cmath>
#include <cstring>
#include <ctime>
#include <functional>
#include <iomanip>
#include <map>
#include <sstream>
#include <sys/time.h>

namespace muduo::base {

namespace {
int64_t kMicroSecondsPerSecond = 1000 * 1000;

using TransFunc = std::function<std::string(const tm &t)>;

std::vector<std::string> monVec = {"January", "February", "March",     "April",   "May",      "June",
                                   "July",    "August",   "September", "October", "November", "December"};

std::vector<std::string> dayVec = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

std::string getKeyFmt(const std::string &str, const struct tm &t);

std::string getWeekDayStr(int idx, bool brief) {
    if (idx < 0 || idx > 6) {
        throw TimeStampException("fmt day is invalid");
    }
    if (brief) {
        return dayVec[ idx ].substr(0, 3);
    }
    return dayVec[ idx ];
}

std::string getMonStr(int idx, bool brief) {
    if (idx < 0 || idx > 11) {
        throw TimeStampException("mon day is invalid");
    }

    if (brief) {
        return monVec[ idx ].substr(0, 3);
    }
    return monVec[ idx ];
}

std::string getKeyFmt(char ch, const struct tm &t, int64_t ms = 0) {
    std::map<char, TransFunc> transMap = {
        {'a', [](const tm &t) { return getWeekDayStr(t.tm_wday, true); }},
        {'A', [](const tm &t) { return getWeekDayStr(t.tm_wday, false); }},
        {'b', [](const tm &t) { return getMonStr(t.tm_mon, true); }},
        {'B', [](const tm &t) { return getMonStr(t.tm_mon, false); }},
        {'c', [](const tm &t) { return getKeyFmt("a b d H:M:S", t); }},
        {'d', [](const tm &t) { return toFixedString(t.tm_mday, 2, false); }},
        {'D', [](const tm &t) { return getKeyFmt("m/d/Y", t); }}, // 月/日/年
        {'e', [](const tm &t) { return std::to_string(t.tm_mday); }},
        {'F', [](const tm &t) { return getKeyFmt("Y/m/d", t); }}, // 年-月-日
        {'g', [](const tm &t) { return std::to_string(t.tm_year + 1900).substr(2); }},
        {'G', [](const tm &t) { return std::to_string(t.tm_year + 1900); }},
        {'h', [](const tm &t) { return getMonStr(t.tm_mon, true); }},
        {'H', [](const tm &t) { return toFixedString(t.tm_hour, 2, false); }},
        {'l', [](const tm &t) { return std::to_string(t.tm_hour % 12); }},
        {'F', [](const tm &t) { return getKeyFmt("Y-m-d", t); }},
        {'j', [](const tm &t) { return std::to_string(t.tm_yday + 1); }},
        {'k', [ ms ](const tm &t) { return toFixedString(ms, 5, true, '0'); }},
        {'K', [ ms ](const tm &t) { return toFixedString(ms, 5, true, '0'); }},
        {'m', [](const tm &t) { return toFixedString(t.tm_mon + 1, 2, false); }},
        {'M', [](const tm &t) { return toFixedString(t.tm_min, 2, false); }},
        {'n', [](const tm &t) { return "\n"; }},
        {'p', [](const tm &t) { return t.tm_hour > 12 ? "PM" : "AM"; }},
        {'r', [](const tm &t) { return std::to_string(t.tm_hour % 12); }},
        {'R', [](const tm &t) { return getKeyFmt("H:M", t); }},
        {'S', [](const tm &t) { return toFixedString(t.tm_sec, 2, false); }},
        {'t', [](const tm &t) { return "\r"; }},
        {'T', [](const tm &t) { return getKeyFmt("H:M:S", t); }},
        {'u', [](const tm &t) { return t.tm_wday == 0 ? "7" : std::to_string(t.tm_wday); }},
        // 把星期日作为第一天（值从0到53）
        {'U', [](const tm &t) { return std::to_string(static_cast<int>(ceil(t.tm_yday / 7.0))); }},
        // 每年的第几周，使用基于周的年
        {'V', [](const tm &t) { return std::to_string(static_cast<int>(floor(t.tm_yday / 7.0))); }},
        {'w', [](const tm &t) { return std::to_string(t.tm_wday); }},
        {'W', [](const tm &t) { return FmtString("%-%-%").str(); }},
        {'x', [](const tm &t) { return getKeyFmt("m/d/Y", t); }},
        {'X', [](const tm &t) { return getKeyFmt("H:M:S", t); }},
        {'y', [](const tm &t) { return std::to_string(t.tm_year + 1900).substr(2); }},
        {'Y', [](const tm &t) { return std::to_string(t.tm_year + 1900); }},
        {'z', [](const tm &t) { return "+" + toFixedString(t.tm_gmtoff / 36, 4, false, '0'); }},
        {'Z', [](const tm &t) { return t.tm_zone; }},
    };

    auto iter = transMap.find(ch);
    if (iter != transMap.end()) {
        return iter->second(t);
    }

    std::string str;
    str.push_back(ch);
    return str;
}

std::string getKeyFmt(const std::string &str, const struct tm &t) {
    std::stringstream ss;
    std::for_each(str.begin(), str.end(), [ &t, &ss ](auto ch) { ss << getKeyFmt(ch, t, 0); });
    return ss.str();
}

} // namespace
Timestamp::Timestamp()
    : microSecondsPerSeconds_(0) {}

Timestamp::Timestamp(int64_t val)
    : microSecondsPerSeconds_(val) {}

Timestamp Timestamp::fromUnixTime(time_t t) { return Timestamp::fromUnixTime(t, 0); }

Timestamp Timestamp::fromUnixTime(time_t t, int microSeconds) {
    return Timestamp(static_cast<int64_t>(t) * kMicroSecondsPerSecond + microSeconds);
}

Timestamp Timestamp::fromTimeStr(const std::string &str, const std::string &fmt) {
    struct tm         t {};
    std::stringstream ss(str);
    ss >> std::get_time(&t, fmt.c_str());
    if (ss.fail()) {
        return Timestamp(0);
    }
    return Timestamp::fromUnixTime(mktime(&t));
}

Timestamp Timestamp::now() {
    struct timeval tv {};
    gettimeofday(&tv, nullptr);
    int64_t seconds = tv.tv_sec;
    return Timestamp(seconds * kMicroSecondsPerSecond + tv.tv_usec);
}

Timestamp Timestamp::invalid() { return {}; }

void Timestamp::swap(Timestamp &other) { microSecondsPerSeconds_ = other.microSeconds(); }

std::string Timestamp::toString() const {
    std::stringstream ss;
    ss << seconds() << "." << static_cast<int>(microSecondsPerSeconds_ % kMicroSecondsPerSecond);
    return ss.str();
}

std::string Timestamp::toFmtString(const char *fmt) const {
    time_t     ts = seconds();
    struct tm *t  = localtime(&ts);

    std::stringstream ss;
    for (size_t idx = 0; idx < strlen(fmt) - 1; ++idx) {
        if (fmt[ idx ] == '%') {
            if (fmt[ idx + 1 ] == '%') {
                ss << '%';
            }
            ss << getKeyFmt(fmt[ idx + 1 ], *t, mseconds());
            idx++;
        } else {
            ss << fmt[ idx ];
        }
    }

    return ss.str();
}

bool Timestamp::valid() const { return microSecondsPerSeconds_ > 0; }

int64_t Timestamp::microSeconds() const { return microSecondsPerSeconds_; }

time_t Timestamp::seconds() const { return static_cast<time_t>(microSecondsPerSeconds_ / kMicroSecondsPerSecond); }

int64_t Timestamp::mseconds() const { return microSecondsPerSeconds_ % kMicroSecondsPerSecond; }

} // namespace muduo::base