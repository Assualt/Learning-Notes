#include "threadpool.h"
#include <iostream>

void fun1(int slp) {
    while (slp-- > 0) {
        printf("%d ======= fun1 sleep %d  =========  \n", std::this_thread::get_id(), slp);
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

struct gfun {
    int operator()(int n) {
        while (n--) {
            printf("%d gfunc1\n", std::this_thread::get_id());
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
        return 10;
    }
};

int main() try {
    std::threadpool executor{10};
    std::future<void> ff = executor.commit(fun1, 10);
    std::future<int> fg = executor.commit(gfun{}, 10);
    std::future<std::string> fh = executor.commit([]() -> std::string {
        while (1) {
            std::cout << std::this_thread::get_id() << "Run in Fun lambd" << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(3));
        }
        return "hello,fh ret !";
    });
    return 0;
} catch (std::exception& e) {
    std::cout << "some unhappy happened...  " << std::this_thread::get_id() << e.what() << std::endl;
}