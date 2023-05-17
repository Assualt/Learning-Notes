#pragma once
namespace muduo::base {
class copyable {
protected:
    copyable()  = default;
    ~copyable() = default;
};
} // namespace muduo::base