#pragma once

namespace muduo {
namespace base {
class nonecopyable {
protected:
    nonecopyable()                      = default;
    ~nonecopyable()                     = default;
    nonecopyable(const nonecopyable &)  = delete;
    nonecopyable(const nonecopyable &&) = delete;
    nonecopyable &operator=(const nonecopyable &&) = delete;
    nonecopyable &operator=(const nonecopyable &) = delete;
};
} // namespace base
} // namespace muduo