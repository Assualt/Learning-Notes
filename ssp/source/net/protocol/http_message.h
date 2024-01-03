//
// Created by 侯鑫 on 2024/1/3.
//

#ifndef SSP_TOOLS_HTTP_MESSAGE_H
#define SSP_TOOLS_HTTP_MESSAGE_H

#include "protocol_msg.h"

namespace ssp::net {
class HttpRequestMessage : protected protocol::ProtocolMessage {
protected:
    uint32_t AppendToOutput(const void *buf, uint32_t size) override;

    uint32_t ParseInput(const void *buf, uint32_t size) override;

    int32_t Append(const void *buf, int32_t size);
};

}

#endif //SSP_TOOLS_HTTP_MESSAGE_H
