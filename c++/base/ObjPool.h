#pragma once
#include "Logging.h"
#include "Object.h"
#include <algorithm>
#include <functional>
#include <map>
#include <memory>
#include <vector>

namespace muduo {
namespace base {

using Creator = std::function<muduo::base::Object *()>;
#define REG_OBJECT(ClassName)                                                                                                                              \
    static void __attribute((constructor)) __Register##ClassName() {                                                                                       \
        ObjPool::Instance().Register(typeid(ClassName).hash_code(), typeid(ClassName).name(), []() { return reinterpret_cast<Object *>(new ClassName); }); \
    }
class ObjPool final {
public:
    static ObjPool &Instance() {
        static ObjPool pool;
        return pool;
    }

    void Register(size_t hash, const std::string &name, const Creator &func) {
        m_creators[ hash ]   = func;
        m_objNameMap[ hash ] = name;
    }

    void PreInit() {
        std::map<size_t, std::shared_ptr<Object>> pool;
        bool                                      failed = false;
        std::for_each(m_creators.begin(), m_creators.end(), [ &failed, &pool ](auto item) {
            if (failed) {
                return;
            }
            auto obj = item.second();
            if (obj == nullptr) {
                failed = true;
                return;
            }
            pool.emplace(item.first, obj);
        });

        if (failed) {
            pool.clear();
        } else {
            m_objMap.swap(pool);
        }
    }

    void PostInit() {
        std::vector<ConstructorStage> stages = {INIT_SELF, INIT_OTHER, INIT_FINISH};
        for (auto stage : stages) {
            auto result = std::all_of(m_objMap.begin(), m_objMap.end(), [ &stage ](auto &item) { return item.second->AutoInit(stage); });
            LOG_IF(result != true).info("auto init func failed in stage:%d result:%d", static_cast<int>(stage), result);
        }
    }

    template <class ClassName> auto Query() -> std::shared_ptr<Object> {
        auto hash = typeid(ClassName).hash_code();
        auto iter = m_objMap.find(hash);
        return (iter != m_objMap.end()) ? iter->second : nullptr;
    }

private:
    std::map<size_t, Creator>                 m_creators;
    std::map<size_t, std::string>             m_objNameMap;
    std::map<size_t, std::shared_ptr<Object>> m_objMap;
};

} // namespace base
} // namespace muduo
