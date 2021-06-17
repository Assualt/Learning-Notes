#include <thread>
#include <vector>
#include "logging.h"
int main(int argc, char **argv) {

    std::vector<std::thread> threadArray;

    for (auto i = 0; i < 10; i++) {
        std::string threadName = "Thread " + to_string(i);
        threadArray.push_back(std::thread([](int index, const std::string &threadName) {
            while(index--){
                logger.info("threadName:%s hello world", threadName );
            }
        }, i, threadName));
    }
    for (auto i = 0; i < 10; i++) {
        threadArray.at(i).join();
    }

    return 0;
}