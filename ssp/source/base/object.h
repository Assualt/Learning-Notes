//
// Created by 侯鑫 on 2023/12/25.
//

#ifndef SSP_TOOLS_OBJECT_H
#define SSP_TOOLS_OBJECT_H

#include <cstdint>
#include <iostream>

namespace ssp::base {

enum class ConstructStage {
    INIT_SELF,
    INIT_OTHER,
    INIT_FINISH,
};

class Object {
public:
    Object()          = default;
    virtual ~Object() = default;

public:
    bool AutoInit(ConstructStage stage);

    virtual bool InitSelf();

    virtual bool InitOther();

    virtual bool InitFinish();

    virtual void Dump(std::ostream &os);

    [[nodiscard]] virtual long Hashcode() const;

    [[nodiscard]] virtual const char *GetObjectName() const;

public:
    [[nodiscard]] virtual const char *ToString() const;

    virtual bool Equals(const Object &rhs);
};

}
#endif //SSP_TOOLS_OBJECT_H
