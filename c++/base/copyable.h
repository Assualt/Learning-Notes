#pragma once
namespace muduo {
namespace base {
class copyable {
protected:
    copyable()  = default;
    ~copyable() = default;
};
} // namespace base
} // namespace muduo