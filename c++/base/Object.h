#pragma once

#include "nonecopyable.h"
namespace muduo {
namespace base {

class Object : nonecopyable{
public:
    Object()          = default;
    virtual ~Object() = default;

public:
    virtual const char *toString();
    virtual bool        equals(const Object &rhs);
};

} // namespace base
} // namespace muduo