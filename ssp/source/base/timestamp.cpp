//
// Created by 侯鑫 on 2023/12/26.
//

#include "timestamp.h"
#include <string>
#include <sys/time.h>
#include <functional>
#include <sstream>
#include <map>
#include "format.h"

using namespace ssp::base;

using TransFunc = std::function<std::string(const tm &t)>;

namespace {
uint64_t kMicroSecondsPerSecond = 1000 * 1000;

std::vector<std::string> monVec = {"January", "February", "March",     "April",   "May",      "June",
                                   "July",    "August",   "September", "October", "November", "December"};

std::vector<std::string> dayVec = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

std::string GetWeekDayStr(int idx, bool brief) {
    if (idx < 0 || idx > 6) {
        throw TimeStampException("fmt day is invalid");
    }
    if (brief) {
        return dayVec[ idx ].substr(0, 3);
    }
    return dayVec[ idx ];
}

std::string GetMonStr(int idx, bool brief) {
    if (idx < 0 || idx > 11) {
        throw TimeStampException("mon day is invalid");
    }

    if (brief) {
        return monVec[ idx ].substr(0, 3);
    }
    return monVec[ idx ];
}

std::string GetKeyFmtStr(const std::string &str, const struct tm &t);

std::string GetKeyFmt(char ch, const struct tm &t, uint64_t ms = 0) {
    std::map<char, TransFunc> transMap = {
            {'a', [](const tm &t) { return GetWeekDayStr(t.tm_wday, true); }},
            {'A', [](const tm &t) { return GetWeekDayStr(t.tm_wday, false); }},
            {'b', [](const tm &t) { return GetMonStr(t.tm_mon, true); }},
            {'B', [](const tm &t) { return GetMonStr(t.tm_mon, false); }},
            {'c', [](const tm &t) { return GetKeyFmtStr("a b d H:M:S", t); }},
            {'d', [](const tm &t) { return ToFixedString(t.tm_mday, 2, false); }},
            {'D', [](const tm &t) { return GetKeyFmtStr("m/d/Y", t); }}, // 月/日/年
            {'e', [](const tm &t) { return std::to_string(t.tm_mday); }},
            {'F', [](const tm &t) { return GetKeyFmtStr("Y/m/d", t); }}, // 年-月-日
            {'g', [](const tm &t) { return std::to_string(t.tm_year + 1900).substr(2); }},
            {'G', [](const tm &t) { return std::to_string(t.tm_year + 1900); }},
            {'h', [](const tm &t) { return GetMonStr(t.tm_mon, true); }},
            {'H', [](const tm &t) { return ToFixedString(t.tm_hour, 2, false); }},
            {'l', [](const tm &t) { return std::to_string(t.tm_hour % 12); }},
            {'F', [](const tm &t) { return GetKeyFmtStr("Y-m-d", t); }},
            {'j', [](const tm &t) { return std::to_string(t.tm_yday + 1); }},
            {'k', [ms](const tm &t) { return ToFixedString(ms, 5, true, '0'); }},
            {'K', [ms](const tm &t) { return ToFixedString(ms, 5, true, '0'); }},
            {'m', [](const tm &t) { return ToFixedString(t.tm_mon + 1, 2, false); }},
            {'M', [](const tm &t) { return ToFixedString(t.tm_min, 2, false); }},
            {'n', [](const tm &t) { return "\n"; }},
            {'p', [](const tm &t) { return t.tm_hour > 12 ? "PM" : "AM"; }},
            {'r', [](const tm &t) { return std::to_string(t.tm_hour % 12); }},
            {'R', [](const tm &t) { return GetKeyFmtStr("H:M", t); }},
            {'S', [](const tm &t) { return ToFixedString(t.tm_sec, 2, false); }},
            {'t', [](const tm &t) { return "\r"; }},
            {'T', [](const tm &t) { return GetKeyFmtStr("H:M:S", t); }},
            {'u', [](const tm &t) { return t.tm_wday == 0 ? "7" : std::to_string(t.tm_wday); }},
            // 把星期日作为第一天（值从0到53）
            {'U', [](const tm &t) { return std::to_string(static_cast<int>(ceil(t.tm_yday / 7.0))); }},
            // 每年的第几周，使用基于周的年
            {'V', [](const tm &t) { return std::to_string(static_cast<int>(floor(t.tm_yday / 7.0))); }},
            {'w', [](const tm &t) { return std::to_string(t.tm_wday); }},
            {'W', [](const tm &t) { return FmtString("%-%-%").str(); }},
            {'x', [](const tm &t) { return GetKeyFmtStr("m/d/Y", t); }},
            {'X', [](const tm &t) { return GetKeyFmtStr("H:M:S", t); }},
            {'y', [](const tm &t) { return std::to_string(t.tm_year + 1900).substr(2); }},
            {'Y', [](const tm &t) { return std::to_string(t.tm_year + 1900); }},
            {'z', [](const tm &t) { return "+" + ToFixedString(t.tm_gmtoff / 36, 4, false, '0'); }},
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

std::string GetKeyFmtStr(const std::string &str, const struct tm &t) {
    std::stringstream ss;
    std::for_each(str.begin(), str.end(), [&t, &ss](auto ch) { ss << GetKeyFmt(ch, t, 0); });
    return ss.str();
}
}

TimeStamp TimeStamp::Now()
{
    struct timeval tv {};
    gettimeofday(&tv, nullptr);
    int64_t seconds = tv.tv_sec;
    return TimeStamp(seconds * kMicroSecondsPerSecond + tv.tv_usec);
}

TimeStamp::TimeStamp()
    :msSeconds_(0)
{
}

TimeStamp::TimeStamp(uint64_t val)
    : msSeconds_(val)
{
}

std::string TimeStamp::ToFmtString(const char *fmt) const
{
    time_t     ts = Seconds();
    struct tm *t  = localtime(&ts);

    std::stringstream ss;
    for (size_t idx = 0; idx < strlen(fmt) - 1; ++idx) {
        if (fmt[ idx ] == '%') {
            if (fmt[ idx + 1 ] == '%') {
                ss << '%';
            }
            ss << GetKeyFmt(fmt[ idx + 1 ], *t, MSeconds());
            idx++;
        } else {
            ss << fmt[ idx ];
        }
    }

    return ss.str();
}

[[maybe_unused]] uint64_t TimeStamp::MicroSeconds() const
{
    return msSeconds_;
}

time_t TimeStamp::Seconds() const
{
    return static_cast<time_t>(msSeconds_ / kMicroSecondsPerSecond);
}

uint64_t TimeStamp::MSeconds() const
{
    return msSeconds_ % kMicroSecondsPerSecond;
}