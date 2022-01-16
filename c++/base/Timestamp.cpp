#include "Timestamp.h"
#include <sstream>
#include <sys/time.h>

namespace muduo {

namespace base {
int64_t Timestamp::kMicroSecondsPerSecond = 1000 * 1000;
Timestamp::Timestamp()
    : microSecondsPerSeconds_(0) {
}

Timestamp::Timestamp(int64_t val)
    : microSecondsPerSeconds_(val) {
}

Timestamp Timestamp::fromUnixTime(time_t t) {
    return Timestamp::fromUnixTime(t, 0);
}

Timestamp Timestamp::fromUnixTime(time_t t, int microSeconds) {
    return Timestamp(static_cast<int64_t>(t) * kMicroSecondsPerSecond + microSeconds);
}

Timestamp Timestamp::now() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    int64_t seconds = tv.tv_sec;
    return Timestamp(seconds * kMicroSecondsPerSecond + tv.tv_usec);
}

Timestamp Timestamp::invalid() {
    return Timestamp();
}
void Timestamp::swap(Timestamp &other) {
    microSecondsPerSeconds_ = other.microSeconds();
}

std::string Timestamp::toString() {
    std::stringstream ss;
    ss << seconds() << "." << static_cast<int>(microSecondsPerSeconds_ % kMicroSecondsPerSecond);
    return ss.str();
}

std::string Timestamp::toFormattedString(const char *fmt) {
    time_t     ts                = seconds();
    struct tm *t                 = localtime(&ts);
    char       timeBuffer[ 128 ] = {0};
    strftime(timeBuffer, 128, fmt, t);
    return timeBuffer;
}

bool Timestamp::valid() const {
    return microSecondsPerSeconds_ > 0;
}

int64_t Timestamp::microSeconds() const {
    return microSecondsPerSeconds_;
}

time_t Timestamp::seconds() const {
    return static_cast<time_t>(microSecondsPerSeconds_ / kMicroSecondsPerSecond);
}
} // namespace base
} // namespace muduo