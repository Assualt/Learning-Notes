//
// Created by 侯鑫 on 2024/1/3.
//

#ifndef SSP_TOOLS_HTTP_RESPONSE_H
#define SSP_TOOLS_HTTP_RESPONSE_H

#include "protocol_msg.h"
#include <string>
#include "base/timestamp.h"
#include "net/buffer.h"

#define CTRL "\r\n"

namespace ssp::net {
class HttpResponse : protected protocol::ProtocolMessage {
public:
    explicit HttpResponse(bool isClose = false);

    bool SetMethod(const std::string &val);

    void SetPath(const std::string &val);

    void SetQuery(const std::string &val);

    void SetReqVersion(const std::string &val);

    void SetReqStatusMsg(const std::string &val);

    void SetStatusCode(int32_t code);

    void AddHeader(const std::string &key, const std::string &val);

    void SetRecvTime(const ssp::base::TimeStamp &recvTime);

    std::string Get(const std::string &key);

    void AppendBody(const void *buffer, uint32_t size);

    [[nodiscard]] bool IsOk() const { return statusCode_ / 100 == 2; }

    bool IsTextHtml();

    bool IsJson();

    std::string Text();

    [[nodiscard]] std::pair<uint32_t, std::string> GetStatusCode() const { return {statusCode_, statusMsg_}; };

    std::string Error();

    [[nodiscard]] uint32_t GetBodySize() const;

    friend std::ostream & operator<<(std::ostream &os, const HttpResponse& response)
    {
        os << "* " << response.reqVersion_ << " assume close after body" << CTRL
           << "< " << response.reqVersion_ << " " << response.statusCode_ << " " << response.statusMsg_ << CTRL;
        for (auto &[key, val] : response.headers_) {
            os << "< " << key << ": " << val << CTRL;
        }
        os << "<" << CTRL;
        os << std::string {response.bodyBuffer_.peek(), response.bodyBuffer_.readableBytes()};
        os << "* Closing connection" << CTRL;
        return os;
    }

    Buffer GetOutBuffer() const { return bodyBuffer_; }

protected:
    uint32_t AppendToOutput(const void *buf, uint32_t size) override;

    uint32_t ParseInput(const void *buf, uint32_t size) override;

    [[maybe_unused]] int32_t Append(const void *buf, int32_t size);

private:
    std::string method_;
    std::string path_;
    std::string query_;
    std::string reqVersion_;
    std::string statusMsg_;
    int32_t statusCode_;
    std::vector<std::pair<std::string, std::string>> headers_;
    ssp::base::TimeStamp recvTime_;
    Buffer bodyBuffer_;
};
}

#endif //SSP_TOOLS_HTTP_RESPONSE_H
