// JSON [de]serialization for protobuf + embedded HTTP server and client in C++.
// Copyright (C) 2014 http://protojson.com/
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along
// with this program; if not, write to the Free Software Foundation, Inc.,
// 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

#include "pjcore/make_json_value.h"

#include <limits>

namespace pjcore {

const JsonValue& JsonNull() { return JsonValue::default_instance(); }

JsonValue JsonNaN() {
  return MakeJsonValue(std::numeric_limits<double>::quiet_NaN());
}

JsonValue JsonInfinity() {
  return MakeJsonValue(std::numeric_limits<double>::infinity());
}

JsonValue JsonNegativeInfinity() {
  return MakeJsonValue(-std::numeric_limits<double>::infinity());
}

JsonValue MakeJsonValue(bool bool_value) {
  JsonValue json_value;
  json_value.set_type(JsonValue::TYPE_BOOL);
  json_value.set_bool_value(bool_value);
  return json_value;
}

JsonValue MakeJsonValue(int32_t signed_value) {
  return MakeJsonValue(static_cast<int64_t>(signed_value));
}

JsonValue MakeJsonValue(int64_t signed_value) {
  JsonValue json_value;
  json_value.set_type(JsonValue::TYPE_SIGNED);
  json_value.set_signed_value(signed_value);
  return json_value;
}

#ifdef PJCORE_DISTINCT_LONG_LONG
JsonValue MakeJsonValue(long long int  // NOLINT(runtime/int)
                        signed_value) {
  return MakeJsonValue(static_cast<int64_t>(signed_value));
}
#endif  // PJCORE_DISTINCT_LONG_LONG

JsonValue MakeJsonValue(uint32_t unsigned_value) {
  return MakeJsonValue(static_cast<uint64_t>(unsigned_value));
}

JsonValue MakeJsonValue(uint64_t unsigned_value) {
  JsonValue json_value;
  json_value.set_type(JsonValue::TYPE_UNSIGNED);
  json_value.set_unsigned_value(unsigned_value);
  return json_value;
}

#ifdef PJCORE_DISTINCT_LONG_LONG
JsonValue MakeJsonValue(long long unsigned int  // NOLINT(runtime/int)
                        unsigned_value) {
  return MakeJsonValue(static_cast<uint64_t>(unsigned_value));
}
#endif  // PJCORE_DISTINCT_LONG_LONG

JsonValue MakeJsonValue(float float_value) {
  JsonValue json_value;
  json_value.set_type(JsonValue::TYPE_DOUBLE);
  json_value.set_double_value(float_value);
  return json_value;
}

JsonValue MakeJsonValue(double double_value) {
  JsonValue json_value;
  json_value.set_type(JsonValue::TYPE_DOUBLE);
  json_value.set_double_value(double_value);
  return json_value;
}

JsonValue MakeJsonValue(const char* string_value) {
  return MakeJsonValue(StringPiece(string_value));
}

JsonValue MakeJsonValue(StringPiece string_value) {
  JsonValue json_value;
  json_value.set_type(JsonValue::TYPE_STRING);
  string_value.CopyToString(json_value.mutable_string_value());
  return json_value;
}

const JsonValue& MakeJsonValue(const JsonValue& json_value) {
  return json_value;
}

JsonValue MakeJsonObject() {
  JsonValue json_value;
  json_value.set_type(JsonValue::TYPE_OBJECT);
  return json_value;
}

JsonValue MakeJsonArray() {
  JsonValue json_value;
  json_value.set_type(JsonValue::TYPE_ARRAY);
  return json_value;
}

JsonValue MakeJsonObject(const google::protobuf::RepeatedPtrField<
    JsonValue::Property> object_properties) {
  JsonValue json_value = MakeJsonObject();
  json_value.mutable_object_properties()->CopyFrom(object_properties);
  return json_value;
}

JsonValue MakeJsonArray(
    const google::protobuf::RepeatedPtrField<JsonValue>& array_elements) {
  JsonValue json_value = MakeJsonArray();
  json_value.mutable_array_elements()->CopyFrom(array_elements);
  return json_value;
}

}  // namespace pjcore
