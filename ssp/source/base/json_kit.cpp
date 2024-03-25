//
// Created by 侯鑫 on 2024/2/4.
//

#include "json_kit.h"

using namespace ssp::base;

JsonKit::JsonKit(const std::string &path)
    : obj_(json_object_from_file(path.c_str())), error_(obj_ == nullptr)
{
}

JsonKit::~JsonKit()
{
    if (obj_ != nullptr) {
    }
}

JsonKit::JsonKit()
{
    obj_ = nullptr;
}

JsonKit::JsonKit(const json_object *object) 
{
    obj_ = const_cast<json_object*>(object);
    error_ = obj_ == nullptr;
}

void JsonKit::SetError(JsonCode err, bool required)
{
    if ((err != NO_ERR) && required) {
        errorCode_ = err;
        error_ = true;
    }
}

JsonCode JsonKit::GetItem(const json_object *object, int32_t & objVal)
{
    if (json_object_get_type(object) != json_type_int) {
        return JsonCode::TYPE_MISMATCH;
    }

    objVal = json_object_get_int(object);
    return JsonCode::NO_ERR;
}

JsonCode JsonKit::GetItem(const json_object *object, bool &objVal)
{
    if (json_object_get_type(object) != json_type_boolean) {
        return JsonCode::TYPE_MISMATCH;
    }

    objVal = json_object_get_boolean(object);
    return JsonCode::NO_ERR;
}

JsonCode JsonKit::GetItem(const json_object *object, double &objVal)
{
    if (json_object_get_type(object) != json_type_double) {
        return JsonCode::TYPE_MISMATCH;
    }

    objVal = json_object_get_double(object);
    return JsonCode::NO_ERR;
}

JsonCode JsonKit::GetItem(const json_object *object, std::string &objVal)
{
    if (json_object_get_type(object) != json_type_string) {
        return JsonCode::TYPE_MISMATCH;
    }

    size_t len = json_object_get_string_len(object);
    if (len > 0) {
        objVal = { json_object_get_string((struct json_object*)object), len };
    }

    return JsonCode::NO_ERR;
}

JsonCode JsonKit::GetItem(const json_object *object, JsonKit &kit)
{
    if (json_object_get_type(object) != json_type_object) { // 对象
        return JsonCode::TYPE_MISMATCH;
    }

    kit = JsonKit(object);
    return JsonCode::NO_ERR;
}

std::vector<std::string> JsonKit::Keys() const
{
    if (IsError() || (json_object_get_type(obj_) != json_type_object)) {
        return {};
    }

    std::vector<std::string> keyList;
    json_object_object_foreach(obj_, key, val) {
        if (key != nullptr) {
            keyList.push_back(key);
        }   
    }

    return keyList;
}