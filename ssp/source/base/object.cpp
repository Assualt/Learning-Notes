//
// Created by 侯鑫 on 2023/12/25.
//

#include "object.h"
#include <string>
#include <sstream>

using namespace ssp::base;

bool Object::AutoInit(ConstructStage stage)
{
    if (stage == ConstructStage::INIT_SELF) {
        return InitSelf();
    } else if (stage == ConstructStage::INIT_OTHER) {
        return InitOther();
    }

    return InitFinish();
}

bool Object::InitSelf()
{
    return true;
}

bool Object::InitOther()
{
    return true;
}

bool Object::InitFinish()
{
    return true;
}

const char *Object::GetObjectName() const
{
    return "Object";
}

const char *Object::ToString() const
{
    static std::string data;
    if (data.empty()) {
        std::stringstream ss;
        ss << GetObjectName() << "@" << std::hex << reinterpret_cast<uintptr_t>(this);
        data = ss.str();
    }
    return data.c_str();
}

long Object::Hashcode() const
{
    return 0L;
}

bool Object::Equals(const Object &rhs)
{
    return rhs.Hashcode() == Hashcode();
}

void Object::Dump(std::ostream &os)
{
    os << ToString() << std::endl;
}