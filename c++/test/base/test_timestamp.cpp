#include "base/Timestamp.h"
#include <iostream>
using namespace muduo::base;
int main(int, char **) {

    Timestamp t = Timestamp::now();

    std::cout << t.toString() << std::endl;
    std::cout << t.toFmtString() << std::endl;
    std::cout << t.toFmtString("%Y-%m-%d %H:%M:%S.%k") << std::endl;
    return 0;
}