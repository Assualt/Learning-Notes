#pragma once

namespace muduo {
namespace base {
class nonecopyable{
public:
    nonecopyable(const nonecopyable&) = delete;
    void operator=(const nonecopyable&) = delete;
protected:    
    nonecopyable() = default;
    ~nonecopyable() = default;
};
}
}