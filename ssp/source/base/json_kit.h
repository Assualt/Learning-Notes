//
// Created by 侯鑫 on 2024/2/4.
//

#ifndef SSP_TOOLS_JSON_KIT_H
#define SSP_TOOLS_JSON_KIT_H

#include "json-c/json.h"
#include <cstdint>
#include <iostream>
#include <string>
#include <vector>
#include "json_type.h"

namespace ssp::base {

enum JsonCode {
  NO_ERR,
  NO_SUCH_KEY,
  TYPE_MISMATCH,
  FILE_OPEN_FAIL,
  JSON_COPY_FAIL
};

class JsonKit {
public:
  explicit JsonKit(const std::string &path);

  JsonKit();

  JsonKit(JsonKit &&rhs);

  ~JsonKit();

  std::string ToString() const {
    if (HasError()) {
      return "";
    }

    return json_object_to_json_string(obj_);
  }

  template <class T>
  JsonKit &Get(const std::string &key, T &val, const bool required = false) {
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

  template <class T>
  JsonKit &GetArray(const std::string &key, std::vector<T> &val,
                    const bool required = false) {
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
      T dstVal;
      if ((tmpVal != nullptr) &&
          (GetItem(tmpVal, dstVal) == JsonCode::NO_ERR)) {
        val.emplace_back(dstVal);
      }
    }

    return *this;
  }

  bool HasError() const { return error_; }

  bool operator!() const { return error_; }

  JsonCode Error() const { return errorCode_; }

  std::vector<std::string> Keys() const;

  void Reset() {
    errorCode_ = NO_ERR;
    error_ = false;
  }

  JsonValue operator[](const JsonType &type);

private:
  JsonCode GetItem(const json_object *object, uint32_t &objVal);
  JsonCode GetItem(const json_object *object, uint64_t &objVal);
  JsonCode GetItem(const json_object *object, int64_t &objVal);
  JsonCode GetItem(const json_object *object, int32_t &objVal);
  JsonCode GetItem(const json_object *object, bool &objVal);
  JsonCode GetItem(const json_object *object, double &objVal);
  JsonCode GetItem(const json_object *object, std::string &objVal);
  JsonCode GetItem(json_object *object, JsonKit &kit);

  void SetError(JsonCode err, bool required);
  void SetObject(json_object *object);
  JsonValue GetArrayItem(uint32_t keyIndex);
  JsonValue GetObjectItem(const std::string &key);

private:
  struct json_object *obj_{nullptr};
  bool error_{false};
  JsonCode errorCode_{NO_ERR};
};

} // namespace ssp::base

#endif // SSP_TOOLS_JSON_KIT_H
