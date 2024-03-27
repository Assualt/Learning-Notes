//
// Created by 侯鑫 on 2024/2/4.
//

#ifndef SSP_TOOLS_JSON_KIT_H
#define SSP_TOOLS_JSON_KIT_H

#include "json-c/json.h"
#include "json_type.h"
#include <cstdint>
#include <cstring>
#include <iostream>
#include <string>
#include <vector>

namespace ssp::base {

enum JsonCode {
    NO_ERR,
    NO_SUCH_KEY,
    TYPE_MISMATCH,
    FILE_OPEN_FAIL,
    JSON_COPY_FAIL,
};

enum JsonType {
    JsonNull   = json_type_null,
    JsonBool   = json_type_boolean,
    JsonDouble = json_type_double,
    JsonInt    = json_type_int,
    JsonObject = json_type_object,
    JsonArray  = json_type_array,
    JsonString = json_type_string,
};

class JsonKit;
class JsonIndexType {
public:
    JsonIndexType(int32_t idx)
        : isTypeInt(true)
        , idx_(idx)
    {
    }

    JsonIndexType(const std::string &key)
        : isTypeInt(false)
        , key_(key)
    {
    }

    JsonIndexType(const char *key)
        : isTypeInt(false)
        , key_({key, strlen(key)})
    {
    }

    bool IsTypeInt() const
    {
        return isTypeInt;
    }

    int32_t TypeIndex() const
    {
        return idx_;
    }

    std::string TypeString() const
    {
        return key_;
    }

private:
    bool        isTypeInt{true};
    int32_t     idx_{0};
    std::string key_;
};

class JsonValue {
public:
    JsonValue(struct json_object *value = nullptr)
        : val_(value)
    {
    }

    JsonValue(int32_t val)
    {
        val_      = json_object_new_int(val);
        needFree_ = true;
    }

    JsonValue(double val)
    {
        val_      = json_object_new_double(val);
        needFree_ = true;
    }

    JsonValue(const std::string &val)
    {
        val_      = json_object_new_string(val.c_str());
        needFree_ = true;
    }

    JsonValue(const char *val)
    {
        val_      = json_object_new_string(val);
        needFree_ = true;
    }

    json_object *Value() const
    {
        return val_;
    }

    ~JsonValue()
    {
        if (needFree_ && (val_ != nullptr)) {
            json_object_put(val_);
        }
    }

    friend std::ostream &operator<<(std::ostream &os, const JsonValue &obj)
    {
        os << json_object_to_json_string(obj.val_);
        return os;
    }

    friend bool operator>(const JsonValue &lhs, const JsonValue &rhs)
    {
        return (lhs - rhs) > 0L;
    }

    friend bool operator>=(const JsonValue &lhs, const JsonValue &rhs)
    {
        return (lhs - rhs) >= 0L;
    }

    friend bool operator<=(const JsonValue &lhs, const JsonValue &rhs)
    {
        return (lhs - rhs) <= 0L;
    }

    friend bool operator<(const JsonValue &lhs, const JsonValue &rhs)
    {
        return (lhs - rhs) < 0L;
    }

    friend double operator-(const JsonValue &lhs, const JsonValue &rhs)
    {
        auto type      = json_object_get_type(lhs.val_);
        auto typeOther = json_object_get_type(rhs.val_);
        if (type != typeOther) { // 类型不等无法比较
            std::cout << "type:" << type << " " << typeOther << std::endl;
            return 0L;
        }

        switch (type) {
            case json_type_boolean:
                return (json_object_get_boolean(lhs.val_) > json_object_get_boolean(rhs.val_)) ? 1L : 0L;
            case json_type_int:
                return (json_object_get_int(lhs.val_) - json_object_get_int(rhs.val_)) * 1.0;
            case json_type_double:
                return json_object_get_double(lhs.val_) - json_object_get_double(rhs.val_);
            case json_type_object:
            case json_type_array:
                return strcmp(json_object_to_json_string(lhs.val_), json_object_to_json_string(rhs.val_)) * 1.0L;
            case json_type_string:
                return strcmp(json_object_get_string(lhs.val_), json_object_get_string(rhs.val_)) * 1.0L;
            default:
                return 0L;
        }
    }

    friend bool operator==(const JsonValue &lhs, const JsonValue &rhs)
    {
        return (lhs - rhs) == 0L;
    }

    JsonValue operator[](JsonIndexType key);

private:
    struct json_object *val_{nullptr};
    bool                needFree_{false};
};

class JsonKit {
public:
    explicit JsonKit(const std::string &path);

    JsonKit(struct json_object *obj);

    JsonKit(JsonType type = JsonNull, JsonType itemType = JsonNull);

    JsonKit(JsonKit &&rhs);

    ~JsonKit();

    std::string ToString() const;

    template <class T> JsonKit &Get(const std::string &key, T &val, const bool required = false)
    {
        if (HasError() || key.empty()) {
            return *this;
        }

        struct json_object *objectVal = nullptr;
        if (!json_object_object_get_ex(obj_, key.c_str(), &objectVal)) {
            SetError(JsonCode::NO_SUCH_KEY, required);
            return *this;
        }

        auto code = GetItem(objectVal, val);
        SetError(code, required);
        return *this;
    }

    template <class T> JsonKit &GetArray(const std::string &key, std::vector<T> &val, const bool required = false)
    {
        if (HasError() || key.empty()) {
            return *this;
        }

        struct json_object *objectVal = nullptr;
        if (!json_object_object_get_ex(obj_, key.c_str(), &objectVal)) {
            SetError(JsonCode::NO_SUCH_KEY, required);
            return *this;
        } else if (!json_object_is_type(objectVal, json_type_array)) {
            SetError(JsonCode::TYPE_MISMATCH, required);
            return *this;
        }

        uint32_t size = json_object_array_length(objectVal);
        for (uint32_t idx = 0; idx < size; ++idx) {
            auto tmpVal = json_object_array_get_idx(objectVal, idx);
            T    dstVal;
            if ((tmpVal != nullptr) && (GetItem(tmpVal, dstVal) == JsonCode::NO_ERR)) {
                val.emplace_back(dstVal);
            }
        }

        return *this;
    }

    bool HasError() const
    {
        return error_;
    }

    bool operator!() const
    {
        return error_;
    }

    JsonCode Error() const
    {
        return errorCode_;
    }

    std::vector<std::string> Keys() const;

    void Reset();

    JsonValue operator[](const JsonIndexType &type);

    json_type Type() const;

    template <class T> JsonKit &Add(const std::string &key, const T &value)
    {
        if (HasError() || (Type() != json_type_object)) {
            return *this;
        }

        struct json_object *newNode = NewNode(value);
        if (newNode == nullptr) {
            return *this;
        }

        json_object_object_add(obj_, key.c_str(), newNode);
        return *this;
    }

    void Delete(const std::string &key);

    int32_t DeleteAt(uint32_t from, uint32_t count = 1);

    template <class T> JsonKit &Push(const T &val)
    {
        if (HasError() || (Type() != json_type_array)) {
            return *this;
        }

        auto node = NewNode(val);
        if (json_object_get_type(node) != static_cast<json_type>(arrayInnerType_)) { // wrong item type
            return *this;
        }

        json_object_array_add(obj_, node);
        return *this;
    }

    template <class T> JsonKit &Insert(const T &val, size_t pos)
    {
        if (HasError() || (Type() != json_type_array)) {
            return *this;
        }

        auto node = NewNode(val);
        if (json_object_get_type(node) != static_cast<json_type>(arrayInnerType_)) { // wrong item type
            return *this;
        }

        json_object_array_insert_idx(obj_, pos, node);
        return *this;
    }

    friend std::ostream &operator<<(std::ostream &os, const JsonKit &kit)
    {
        os << kit.ToString();
        return os;
    }

    json_object *Value() const;

private:
    JsonCode GetItem(const json_object *object, uint32_t &objVal);
    JsonCode GetItem(const json_object *object, uint64_t &objVal);
    JsonCode GetItem(const json_object *object, int64_t &objVal);
    JsonCode GetItem(const json_object *object, int32_t &objVal);
    JsonCode GetItem(const json_object *object, bool &objVal);
    JsonCode GetItem(const json_object *object, double &objVal);
    JsonCode GetItem(const json_object *object, std::string &objVal);
    JsonCode GetItem(json_object *object, JsonKit &kit);

    json_object *NewNode(uint32_t val) const;
    json_object *NewNode(int32_t val) const;
    json_object *NewNode(int64_t val) const;
    json_object *NewNode(uint64_t val) const;
    json_object *NewNode(bool val) const;
    json_object *NewNode(double val) const;
    json_object *NewNode(const std::string &val) const;
    json_object *NewNode(const JsonKit &kit) const;
    json_object *NewObjNode(JsonType type) const;

    void      SetError(JsonCode err, bool required);
    void      SetObject(json_object *object);
    JsonValue GetArrayItem(uint32_t keyIndex);
    JsonValue GetObjectItem(const std::string &key);

private:
    struct json_object *obj_{nullptr};
    bool                error_{false};
    JsonCode            errorCode_{NO_ERR};
    bool                needFree_{true};
    JsonType            arrayInnerType_;
};

using Json = JsonKit;

} // namespace ssp::base

#endif // SSP_TOOLS_JSON_KIT_H
