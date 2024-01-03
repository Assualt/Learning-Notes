//
// Created by 侯鑫 on 2024/1/3.
//

#ifndef SSP_TOOLS_COMMON_MSG_H
#define SSP_TOOLS_COMMON_MSG_H

#include <cstdint>

namespace ssp::net {

struct _protocol_message {
    int32_t (*Append)(const void *, uint32_t, _protocol_message *);
    uint8_t data[0];
    uint32_t data_len;
};

class CommMessageInput {
public:
    virtual uint32_t ParseInput(const void *buf, uint32_t size) = 0;
};

class CommMessageOutput {
public:
    virtual uint32_t AppendToOutput(const void *buf, uint32_t size) = 0;
};

}

#endif //SSP_TOOLS_COMMON_MSG_H
