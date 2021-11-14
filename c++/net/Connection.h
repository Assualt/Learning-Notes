#pragma once

#include "base/nonecopyable.h"

using muduo::base::nonecopyable;
namespace muduo {
namespace net {

class Connection : nonecopyable {
public:
    enum CONNECTION_TYPE {
        TYPE_TCP,
        TYPE_UDP
    };

public:
    
};

} // namespace net
} // namespace muduo