//
// Created by 侯鑫 on 2023/12/26.
//

#ifndef SSP_TOOLS_OBJECT_POOL_H
#define SSP_TOOLS_OBJECT_POOL_H

#include "object.h"
#include <cstdint>
#include <functional>
#include <map>
#include <memory>

namespace ssp::base {
using Creator = std::function<Object*()>;

class ObjectPool final {
private:
    ObjectPool() = default;
    ~ObjectPool() = default;

public:
    static ObjectPool& Instance();

    void Register(size_t hash, const std::string& name, const Creator& creator);

    void PreInit();

    void PostInit();

    void Dump(std::ostream &os);

    template<class T> void PushObj(T *val)
    {
        if (val == nullptr) {
            return;
        }

        static_assert(std::is_base_of<Object, T>::value, "object is not base of T");
        objMap_.emplace(typeid(T).hash_code(), reinterpret_cast<Object *>(val));
    }

    template<class T> auto Query() -> std::shared_ptr<Object>
    {
        static_assert(std::is_base_of<Object, T>::value, "object is not base of T");
        auto hash = typeid(T).hash_code();
        auto iter = objMap_.find(hash);
        return iter != objMap_.end() ? iter->second : nullptr;
    }

    [[nodiscard]] uint32_t Size() const;

    template<class Function, class ...Args> auto Invoke(Function func, Args... arg) -> std::result_of<Function(Args...)>
    {
        return std::invoke(func, arg...);
    }

private:
    std::map<size_t, Creator> creators_;
    std::map<size_t, std::string> objNameMap_;
    std::map<size_t, std::shared_ptr<Object>> objMap_;
};
}

#define REG_OBJECT(ClassName) \
    static void __attribute((constructor)) __Register##ClassName() {                                             \
        ssp::base::ObjectPool::Instance().Register(typeid(ClassName).hash_code(), typeid(ClassName).name(),      \
            []() { return reinterpret_cast<Object *>(new ClassName); });                                         \
    }

#endif //SSP_TOOLS_OBJECT_POOL_H
