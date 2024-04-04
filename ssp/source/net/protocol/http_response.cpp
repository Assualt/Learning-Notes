//
// Created by 侯鑫 on 2024/1/3.
//

#include "http_response.h"
#include "http_config.h"
#include "base/string_utils.h"
#include <set>
#include <algorithm>

using namespace ssp::net;
using namespace ssp::base;

HttpResponse::HttpResponse(bool isClose)
{}

bool HttpResponse::SetMethod(const std::string &val)
{
    auto tmp = util::trim(val);
    static std::set<std::string> validMethod = {
        "GET", "POST", "HEAD", "PUT", "DELETE"
    };

    for (auto &item : validMethod) {
        if (util::EqualsIgnoreCase(item, val)) {
            method_ = item;
            return true;
        }
    }

    return false;
}

void HttpResponse::SetPath(const std::string &val)
{
    path_ = util::trim(val);
}

void HttpResponse::SetQuery(const std::string &val)
{
    query_ = util::trim(val);
}

void HttpResponse::SetReqVersion(const std::string &val)
{
    reqVersion_ = util::trim(val);
}

void HttpResponse::SetReqStatusMsg(const std::string &val)
{
    statusMsg_ = util::trim(val);
}

void HttpResponse::SetStatusCode(int32_t code)
{
    statusCode_ = code;
}

void HttpResponse::AddHeader(const std::string &key, const std::string &val)
{
    headers_.emplace_back(util::trim(key), util::trim(val));
}

void HttpResponse::SetRecvTime(const ssp::base::TimeStamp &recvTime)
{
    recvTime_ = recvTime;
}

std::string HttpResponse::Get(const std::string &key)
{
    for (auto [k, v] : headers_) {
        if (util::EqualsIgnoreCase(k, key)) {
            return v;
        }
    }
    return "";
}

uint32_t HttpResponse::GetBodySize() const
{
    return bodyBuffer_.readableBytes();
}

void HttpResponse::AppendBody(const void *buffer, uint32_t size)
{
    bodyBuffer_.append(buffer, size);
}

bool HttpResponse::IsTextHtml()
{
    auto item = Get(ContentType);
    return util::StartsWithIgnoreCase(item, "text/");
}

bool HttpResponse::IsJson()
{
    auto item = Get(ContentType);
    return util::EqualsIgnoreCase(item, "application/json");
}

std::string HttpResponse::Text()
{
    if (IsTextHtml()) {
        return {bodyBuffer_.peek(), bodyBuffer_.readableBytes()};
    }

    return "[NOT TEXT]";
}

std::string HttpResponse::Error()
{
    if (IsOk()) {
        return "";
    }

    return statusMsg_;
}

int32_t HttpResponse::Append(const void *buf, int32_t size)
{
    return 0;
}

uint32_t HttpResponse::AppendToOutput(const void *buf, uint32_t size)
{
    return 0;
}

uint32_t HttpResponse::ParseInput(const void *buf, uint32_t size)
{
    return 0;
}

