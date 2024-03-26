//
// Created by 侯鑫 on 2024/2/4.
//

#ifndef SSP_TOOLS_JSON_TYPE_H
#define SSP_TOOLS_JSON_TYPE_H

#include "json-c/json.h"
#include <cstring>
#include <iostream>

namespace ssp::base {

class JsonType {
public:
  JsonType(int32_t idx) : isTypeInt(true), idx_(idx) {}

  JsonType(const std::string &key) : isTypeInt(false), key_(key) {}

  JsonType(const char *key) : isTypeInt(false), key_({key, strlen(key)}) {}

  bool IsTypeInt() const { return isTypeInt; }

  int32_t TypeIndex() const { return idx_; }

  std::string TypeString() const { return key_; }

private:
  bool isTypeInt{true};
  int32_t idx_;
  std::string key_;
};

class JsonValue {
public:
  JsonValue(struct json_object *value = nullptr) : val_(value) {}

  JsonValue(uint32_t val) {
    val_ = json_object_new_int(val);
    needFree_ = true;
  }

  // explicit JsonValue(double val) {
  //   val_ = json_object_new_double(val);
  //   needFree_ = true;
  // }

  JsonValue(const std::string &val) {
    val_ = json_object_new_string(val.c_str());
    needFree_ = true;
  }

  JsonValue(const char *val) {
    val_ = json_object_new_string(val);
    needFree_ = true;
  }

  ~JsonValue() {
    if (needFree_ && (val_ != nullptr)) {
      json_object_put(val_);
    }
  }

  friend std::ostream &operator<<(std::ostream &os, const JsonValue &obj) {
    if (obj.val_ == nullptr) {
      os << "null";
      return os;
    }

    os << json_object_to_json_string(obj.val_);
    return os;
  }

  friend bool operator>(const JsonValue &lhs, const JsonValue &rhs) {
    auto type = json_object_get_type(lhs.val_);
    auto typeOther = json_object_get_type(rhs.val_);
    if (type != typeOther) { // 类型不等无法比较
      return false;
    }

    switch (type) {
    case json_type_int:
      return json_object_get_int(lhs.val_) > json_object_get_int(rhs.val_);
    case json_type_double:
      return json_object_get_double(lhs.val_) >
             json_object_get_double(rhs.val_);
    default:
      return false;
    }
  }

  friend bool operator>=(const JsonValue &lhs, const JsonValue &rhs) {
    auto type = json_object_get_type(lhs.val_);
    auto typeOther = json_object_get_type(rhs.val_);
    if (type != typeOther) { // 类型不等无法比较
      return false;
    }

    switch (type) {
    case json_type_int:
      return json_object_get_int(lhs.val_) >= json_object_get_int(rhs.val_);
    case json_type_double:
      return json_object_get_double(lhs.val_) >=
             json_object_get_double(rhs.val_);
    default:
      return false;
    }
  }

  friend bool operator<=(const JsonValue &lhs, const JsonValue &rhs) {
    auto type = json_object_get_type(lhs.val_);
    auto typeOther = json_object_get_type(rhs.val_);
    if (type != typeOther) { // 类型不等无法比较
      return false;
    }

    switch (type) {
    case json_type_int:
      return json_object_get_int(lhs.val_) <= json_object_get_int(rhs.val_);
    case json_type_double:
      return json_object_get_double(lhs.val_) <=
             json_object_get_double(rhs.val_);
    default:
      return false;
    }
  }

  friend bool operator<(const JsonValue &lhs, const JsonValue &rhs) {
    auto type = json_object_get_type(lhs.val_);
    auto typeOther = json_object_get_type(rhs.val_);
    if (type != typeOther) { // 类型不等无法比较
      return false;
    }

    switch (type) {
    case json_type_int:
      return json_object_get_int(lhs.val_) < json_object_get_int(rhs.val_);
    case json_type_double:
      return json_object_get_double(lhs.val_) <
             json_object_get_double(rhs.val_);
    default:
      return false;
    }
  }

  friend bool operator==(const JsonValue &lhs, const JsonValue &rhs) {
    auto type = json_object_get_type(lhs.val_);
    auto typeOther = json_object_get_type(rhs.val_);
    if (type != typeOther) { // 类型不等无法比较
      std::cout << "type:" << type << " " << typeOther << std::endl;
      return false;
    }

    switch (type) {
    case json_type_null:
      return true;
    case json_type_boolean:
      return json_object_get_boolean(lhs.val_) ==
             json_object_get_boolean(rhs.val_);
    case json_type_int:
      return json_object_get_int(lhs.val_) == json_object_get_int(rhs.val_);
    case json_type_double:
      return json_object_get_double(lhs.val_) ==
             json_object_get_double(rhs.val_);
    case json_type_object:
    case json_type_array:
      return json_object_to_json_string(lhs.val_) ==
             json_object_to_json_string(rhs.val_);
    case json_type_string:
      return strcmp(json_object_get_string(lhs.val_), json_object_get_string(rhs.val_)) == 0;
    default:
      return false;
    }
  }

private:
  struct json_object *val_{nullptr};
  bool needFree_{false};
};

} // namespace ssp::base

#endif
