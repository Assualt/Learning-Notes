#pragma once

#include "Exception.h"
#include "copyable.h"
#include <iostream>
#include <string>

#define INLINE_OPERATOR(ops, type, val)                                                                                \
    inline bool operator ops(type lhs, type rhs) { return lhs.val ops rhs.val; }

namespace muduo::base {

DECLARE_EXCEPTION(TimeStampException, Exception)

class Timestamp {
public:
    Timestamp();

    explicit Timestamp(int64_t val);

public:
    void swap(Timestamp &other);

    [[nodiscard]] std::string toString() const;

    std::string toFmtString(const char *fmt = "%Y-%m-%d %H:%M:%S.000.%Z%z") const;

    [[nodiscard]] bool valid() const;

    [[nodiscard]] int64_t microSeconds() const;

    [[nodiscard]] time_t seconds() const;

    [[nodiscard]] int64_t mseconds() const;

public:
    static Timestamp now();
    static Timestamp invalid();
    static Timestamp fromUnixTime(time_t t);
    static Timestamp fromUnixTime(time_t t, int microSecond);
    static Timestamp fromTimeStr(const std::string &str, const std::string &fmt = "%Y-%m-%d %H:%M:%S");

private:
    int64_t microSecondsPerSeconds_{0};
};

INLINE_OPERATOR(==, Timestamp, microSeconds())
INLINE_OPERATOR(<, Timestamp, microSeconds())
INLINE_OPERATOR(<=, Timestamp, microSeconds())
INLINE_OPERATOR(>, Timestamp, microSeconds())
INLINE_OPERATOR(>=, Timestamp, microSeconds())

} // namespace muduo::base
