#pragma once

#include "net/Buffer.h"
#include <map>
using namespace muduo::net;

enum ParaType {       // 请求参数type
    Type_Char,        // char
    Type_Ref_Char,    // char&
    Type_Bool,        // bool
    Type_Ref_Bool,    // bool &
    Type_Int,         // int
    Type_Ref_Int,     // int&
    Type_Ponitor_Int, // int*
    Type_Long,        // long
    Type_Const_Chars, // const char *
    Type_Chars,       // char *
    Type_String,      // string
    Type_Ref_String,  // string&
    Type_Ref_String,  // const string&
    Type_Binary
};

struct RequestInfo {
    std::string                                   reqId;   // 请求id
    std::string                                   reqFunc; // 请求函数名
    std::map<int, std::pair<ParaType, uint8_t *>> params;
};

class RpcRequest {
public:
    void parse(const Buffer &inbuf);

private:
    RequestInfo info_; // 请求的body解析
};