//
// Created by 侯鑫 on 2023/12/26.
//

#include "object_pool.h"
#include <vector>
#include <string>

using namespace ssp::base;

ObjectPool &ObjectPool::Instance()
{
    static ObjectPool pool;
    return pool;
}

void ObjectPool::Register(size_t hash, const std::string &name, const Creator &creator)
{
    creators_.emplace(hash, creator);
    objNameMap_.emplace(hash, name);
}

void ObjectPool::PreInit()
{
    decltype(objMap_) pool;
    bool failed = false;
    std::for_each(creators_.begin(), creators_.end(), [&failed, &pool](auto item) {
        if (failed) return ;
        auto obj = item.second();
        if (obj == nullptr) {
            failed = true;
            return ;
        }

        pool.emplace(item.first, obj);
    });

    if (failed) {
        pool.clear();
    } else {
        objMap_.swap(pool);
    }
}

void ObjectPool::PostInit()
{
    std::vector<ConstructStage> stages = {ConstructStage::INIT_SELF, ConstructStage::INIT_OTHER, ConstructStage::INIT_FINISH};
    for (auto stage : stages) {
        auto result = std::all_of(objMap_.begin(), objMap_.end(),
                                  [&stage](auto &item) { return item.second->AutoInit(stage); });
        if (!result) {
            return;
        }
    }
}

uint32_t ObjectPool::Size() const
{
    return objMap_.size();
}

void ObjectPool::Dump(std::ostream &os)
{
    os << "Dump objects: " << std::endl;
    uint32_t idx = 1;
    for (auto&[code, obj] : objMap_) {
        if (obj == nullptr) {
            continue;
        }

        os << "No." << idx++ << std::endl
           << objNameMap_[code] << "(0x" << std::hex << code << ")" << std::endl;
        obj->Dump(os);
    }
}
