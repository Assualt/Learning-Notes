#pragma once

#include "copyable.h"
#include <chrono>
#include <iostream>

#define INLINE_OPERATOR(ops, type, val)            \
    inline bool operator ops(type lhs, type rhs) { \
        return lhs.val ops rhs.val;                \
    }
namespace muduo {
namespace base {
class Timestamp {

public:
    Timestamp();
    explicit Timestamp(int64_t val);

public:
    void swap(Timestamp &other);

    std::string toString();
    std::string toFormattedString(const char *fmt = "%Y-%m-%d %H:%M:%S.%Z%z");
    bool        valid() const;

    int64_t microSeconds() const;
    time_t  seconds() const;

public:
    static Timestamp now();
    static Timestamp invalid();
    static Timestamp fromUnixTime(time_t t);
    static Timestamp fromUnixTime(time_t t, int microSecond);
    static int64_t   kMicroSecondsPerSecond;

private:
    int64_t microSecondsPerSeconds_;
};

INLINE_OPERATOR(==, Timestamp, microSeconds());
INLINE_OPERATOR(<, Timestamp, microSeconds());
INLINE_OPERATOR(<=, Timestamp, microSeconds());
INLINE_OPERATOR(>, Timestamp, microSeconds());
INLINE_OPERATOR(>=, Timestamp, microSeconds());

} // namespace base
} // namespace muduo
