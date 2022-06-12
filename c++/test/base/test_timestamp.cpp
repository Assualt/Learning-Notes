#include "base/Timestamp.h"
#include <iostream>
using namespace muduo::base;
int main(int agrc, char **argv) {

    Timestamp t = Timestamp::now();

    std::cout << t.toString() << std::endl;
    std::cout << t.toFormattedString() << std::endl;
    std::cout << t.toFormattedString("%Y-%m-%d %H:%M:%S") << std::endl;
    return 0;
}