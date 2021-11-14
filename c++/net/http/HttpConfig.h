#include "base/nonecopyable.h"
#include <iostream>
using muduo::base::nonecopyable;
class HttpConfig : nonecopyable {

public:
    HttpConfig() = default;
    HttpConfig(const char *configPath);
};