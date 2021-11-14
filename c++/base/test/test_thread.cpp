#include "base/Thread.h"
#include <iostream>
#include <stdlib.h>
#include <sys/time.h>
using namespace std;
using namespace muduo;
using namespace muduo::base;

void myFunc() {
    int n = 0;
    while (n--) {
        std::cout << "ThreadFunc; " << std::endl;
    }
}

int main(int argc, char const *argv[]) {

    Thread c(myFunc, "OK");
    c.start();
    c.join();
    return 0;
}
