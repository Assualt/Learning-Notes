//
// Created by 侯鑫 on 2024/1/3.
//

#ifndef SSP_TOOLS_PROTOCOL_MSG_H
#define SSP_TOOLS_PROTOCOL_MSG_H

#include "common_msg.h"

namespace ssp::net::protocol {

class ProtocolMessage : public CommMessageInput, CommMessageOutput {
protected:
    uint32_t AppendToOutput(const void *buf, uint32_t size) override
    {
        return 0;
    }

    uint32_t ParseInput(const void *buf, uint32_t size) override
    {
        return 0;
    }
};

}

#endif //SSP_TOOLS_PROTOCOL_MSG_H
