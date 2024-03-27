//
// Created by 侯鑫 on 2024/2/4.
//

#include "json_kit.h"

using namespace ssp::base;

namespace {
constexpr uint32_t g_defaultArraySize = 16;
}

JsonValue JsonValue::operator[](JsonIndexType key)
{
    return JsonKit(val_)[ key ];
}

JsonKit::JsonKit(const std::string &path)
    : obj_(json_object_from_file(path.c_str()))
    , error_(obj_ == nullptr)
{
}

JsonKit::~JsonKit()
{
    if ((obj_ != nullptr) && needFree_) {
        json_object_put(obj_);
    }
}

JsonKit::JsonKit(struct json_object *obj)
{
    Reset();
    obj_      = obj;
    error_    = (obj_ == nullptr);
    needFree_ = false;
}

JsonKit::JsonKit(JsonKit &&rhs)
{
    obj_       = rhs.obj_;
    error_     = rhs.error_;
    errorCode_ = rhs.errorCode_;
}

JsonKit::JsonKit(JsonType type, JsonType arrayItemType)
{
    Reset();
    if ((type == JsonType::JsonObject) || (type == JsonType::JsonArray)) {
        obj_      = NewObjNode(type);
        needFree_ = true;
        arrayInnerType_ = arrayItemType;
    }
}

void JsonKit::SetError(JsonCode err, bool required)
{
    if ((err != NO_ERR) && required) {
        errorCode_ = err;
        error_     = true;
    }
}

json_object *JsonKit::Value() const
{
    return obj_;
}

JsonCode JsonKit::GetItem(const json_object *object, int32_t &objVal)
{
    if (!json_object_is_type(object, json_type_int)) {
        return JsonCode::TYPE_MISMATCH;
    }

    objVal = json_object_get_int(object);
    return JsonCode::NO_ERR;
}

JsonCode JsonKit::GetItem(const json_object *object, int64_t &objVal)
{
    if (!json_object_is_type(object, json_type_int)) {
        return JsonCode::TYPE_MISMATCH;
    }

    objVal = json_object_get_int64(object);
    return JsonCode::NO_ERR;
}

JsonCode JsonKit::GetItem(const json_object *object, uint32_t &objVal)
{
    if (!json_object_is_type(object, json_type_int)) {
        return JsonCode::TYPE_MISMATCH;
    }

    objVal = (UINT32_MAX & json_object_get_uint64(object));
    return JsonCode::NO_ERR;
}

JsonCode JsonKit::GetItem(const json_object *object, uint64_t &objVal)
{
    if (!json_object_is_type(object, json_type_int)) {
        return JsonCode::TYPE_MISMATCH;
    }

    objVal = json_object_get_uint64(object);
    return JsonCode::NO_ERR;
}

JsonCode JsonKit::GetItem(const json_object *object, bool &objVal)
{
    if (!json_object_is_type(object, json_type_boolean)) {
        return JsonCode::TYPE_MISMATCH;
    }

    objVal = json_object_get_boolean(object);
    return JsonCode::NO_ERR;
}

JsonCode JsonKit::GetItem(const json_object *object, double &objVal)
{
    if (!json_object_is_type(object, json_type_double)) {
        return JsonCode::TYPE_MISMATCH;
    }

    objVal = json_object_get_double(object);
    return JsonCode::NO_ERR;
}

JsonCode JsonKit::GetItem(const json_object *object, std::string &objVal)
{
    if (!json_object_is_type(object, json_type_string)) {
        return JsonCode::TYPE_MISMATCH;
    }

    size_t len = json_object_get_string_len(object);
    if (len > 0) {
        objVal = {json_object_get_string((struct json_object *)object), len};
    }

    return JsonCode::NO_ERR;
}

JsonCode JsonKit::GetItem(json_object *object, JsonKit &kit)
{
    if (!json_object_is_type(object, json_type_object) && !json_object_is_type(object, json_type_array)) {
        return JsonCode::TYPE_MISMATCH;
    }

    kit.SetObject(object);
    return JsonCode::NO_ERR;
}

json_object *JsonKit::NewNode(uint32_t val) const
{
    return json_object_new_int(val);
}

json_object *JsonKit::NewNode(int32_t val) const
{
    return json_object_new_int(val);
}

json_object *JsonKit::NewNode(int64_t val) const
{
    return json_object_new_int64(val);
}

json_object *JsonKit::NewNode(uint64_t val) const
{
    return json_object_new_uint64(val);
}

json_object *JsonKit::NewNode(bool val) const
{
    return json_object_new_boolean(val);
}

json_object *JsonKit::NewNode(double val) const
{
    return json_object_new_double(val);
}

json_object *JsonKit::NewNode(const std::string &val) const
{
    return json_object_new_string(val.c_str());
}

json_object *JsonKit::NewObjNode(JsonType type) const
{
    if (type == JsonObject) {
        return json_object_new_object();
    }

    return json_object_new_array_ext(g_defaultArraySize);
}

json_object *JsonKit::NewNode(const JsonKit &kit) const
{
    struct json_object *copyObject = nullptr;
    json_object_deep_copy(kit.Value(), &copyObject, nullptr);
    return copyObject;
}

std::vector<std::string> JsonKit::Keys() const
{
    if (HasError() || (json_object_get_type(obj_) != json_type_object)) {
        return {};
    }

    std::vector<std::string> keyList;
    json_object_object_foreach(obj_, key, val)
    {
        if (key != nullptr) {
            keyList.push_back(key);
        }
    }

    return keyList;
}

void JsonKit::SetObject(json_object *object)
{
    Reset();
    obj_ = object;
}

JsonValue JsonKit::GetArrayItem(uint32_t keyIndex)
{
    uint32_t size = json_object_array_length(obj_);
    if ((keyIndex > size) || (keyIndex < 0)) {
        throw std::out_of_range("out of range");
    }

    auto tmp = json_object_array_get_idx(obj_, keyIndex);
    if (tmp != nullptr) {
        return JsonValue(tmp);
    }

    return {};
}

JsonValue JsonKit::GetObjectItem(const std::string &key)
{
    struct json_object *objectVal = nullptr;
    if (!json_object_object_get_ex(obj_, key.c_str(), &objectVal)) {
        SetError(JsonCode::NO_SUCH_KEY, false);
        return {};
    }

    return JsonValue(objectVal);
}

JsonValue JsonKit::operator[](const JsonIndexType &key)
{
    if (HasError()) {
        return {};
    }

    auto type = json_object_get_type(obj_);
    if (type == json_type_array) {
        return GetArrayItem(key.TypeIndex());
    } else if (type == json_type_object) {
        return GetObjectItem(key.TypeString());
    }

    throw std::invalid_argument("invalid argument");
}

std::string JsonKit::ToString() const
{
    if (HasError()) {
        return "";
    }

    return json_object_to_json_string(obj_);
}

void JsonKit::Reset()
{
    errorCode_ = NO_ERR;
    error_     = false;
}

json_type JsonKit::Type() const
{
    return json_object_get_type(obj_);
}

void JsonKit::Delete(const std::string &key)
{
    if (HasError() || (Type() != json_type_object)) {
        return;
    }

    json_object_object_del(obj_, key.c_str());
}

int32_t JsonKit::DeleteAt(uint32_t from, uint32_t count)
{
    if (HasError() || (Type() != json_type_array)) {
        return 0;
    }

    auto length = json_object_array_length(obj_);
    if (from >= length - 1) {
        return 0;
    }

    return json_object_array_del_idx(obj_, from, count);
}