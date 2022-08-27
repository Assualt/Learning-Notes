#include "base/Logging.h"
#include "base/Timer.h"
#include "base/Timestamp.h"
using namespace muduo::base;
class T {
public:
    T(TimerCallback cb) { t = std::make_unique<Timer>(UINT32_MAX, 0.5, cb, reinterpret_cast<uintptr_t>(this), 20); }

    void Start() { t->Start(); }

    void Stop() {
        std::cout << "receive Stop Sig" << std::endl;
        t->Stop();
    }

    static void timeProcFunc(uintptr_t p1, uint32_t p2) {
        static int c = 0;
        logger.info("para1:%s, para2:%s", p1, p2);
        if (c++ > 10) {
            auto test = reinterpret_cast<T *>(p1);
            test->Stop();
        }
    }

private:
    std::unique_ptr<Timer> t{nullptr};
};

int main(int, char **) {
    auto &log = Logger::getLogger();
    log.BasicConfig(LogLevel::Info, "T:%(tid)(%(asctime))[%(appname):%(levelname)][%(filename):%(lineno)] %(message)",
                    "", "");
    log.setAppName("app");
    auto stdHandle = std::make_shared<StdOutLogHandle>();
    log.addLogHandle(stdHandle.get());

    T d(T::timeProcFunc);

    d.Start();

    return 0;
}