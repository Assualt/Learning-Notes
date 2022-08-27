#pragma once
#include "nonecopyable.h"
namespace muduo {
namespace base {

enum ConstructorStage {
    INIT_SELF,
    INIT_OTHER,
    INIT_FINISH,
};

class Object : nonecopyable {
public:
    Object()          = default;
    virtual ~Object() = default;

public:
    bool AutoInit(ConstructorStage stage) {
        if (stage == INIT_SELF) {
            return InitSelf();
        } else if (stage == INIT_OTHER) {
            return InitOther();
        }
        return InitFinish();
    }

    virtual bool InitSelf()  = 0;
    virtual bool InitOther() = 0;
    virtual bool InitFinish() { return true; }

public:
    virtual const char *toString() { return "Object"; }
    virtual bool        equals(const Object &rhs) { return true; };
};

} // namespace base
} // namespace muduo