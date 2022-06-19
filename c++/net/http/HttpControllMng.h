#include <algorithm>
#include <any>
#include <functional>
#include <map>
#include <memory>
#include "controller/Controller_if.h"

#define REG_CONTROLLER(para1, para2, ...) ControllManager::Instance().RegObj<para1, para2>(__VA_ARGS__);
class ControllManager final {
public:
    ControllManager &Instance() {
        static ControllManager self;
        return self;
    }

    void Init() {
        if (inited_) {
            return;
        }

        std::for_each(objMapper_.begin(), objMapper_.end(), [](auto &item) {
            auto iter = reinterpret_cast<IController *>(item.second);
            iter->onInit();
        });

        inited_ = true;
    }

    template <class parent, class child, class... Args> void RegObj(const char *childClassName, Args... arg) {
        auto obj = std::make_unique<child>(std::forward(arg...));
        if (obj != nullptr) {
            objMapper_[ childClassName ] = reinterpret_cast<uintptr_t>(obj);
        }
    }

    template <class obj> obj *QueryObj(const std::string &className) {
        if (objMapper_.count(className) == 0) {
            return nullptr;
        }

        return reinterpret_cast<obj *>(objMapper_[ className ]);
    }

private:
    std::map<std::string, uintptr_t> objMapper_;
    ControllManager() = default;
    bool inited_{false};
};