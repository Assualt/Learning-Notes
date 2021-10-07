//
// Created by Sunshine on 2021/4/17.
//
#include <iostream>
#include "EventLoop.h"
using namespace std;
using namespace muduo;
using namespace muduo::net;
using namespace muduo::base;
int main(int argc, char **argv) {

    EventLoop loop;

    

    loop.loop();

   
    return 0;
}