//
// Created by 侯鑫 on 2024/1/3.
//

#ifndef SSP_TOOLS_COMM_SERVICE_H
#define SSP_TOOLS_COMM_SERVICE_H

#include "base/config.h"

using namespace ssp::base;

namespace ssp::net {
class CommService {
public:
    virtual bool Init(const ConfigureManager &configure) = 0;

    virtual bool Start() = 0;

    virtual bool UnInit() = 0;
};
}


#endif //SSP_TOOLS_COMM_SERVICE_H
