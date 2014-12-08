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

#ifndef PJCORE_MAKE_JSON_VALUE_H_
#define PJCORE_MAKE_JSON_VALUE_H_

#include <string>

#include "pjcore/json.pb.h"
#include "pjcore/third_party/chromium/string_piece.h"

namespace pjcore {

const JsonValue& JsonNull();

JsonValue JsonNaN();

JsonValue JsonInfinity();

JsonValue JsonNegativeInfinity();

JsonValue MakeJsonValue(bool bool_value);

JsonValue MakeJsonValue(int32_t signed_value);

JsonValue MakeJsonValue(int64_t signed_value);

JsonValue MakeJsonValue(long long int  // NOLINT(runtime/int)
                        signed_value);

JsonValue MakeJsonValue(uint32_t unsigned_value);

JsonValue MakeJsonValue(uint64_t unsigned_value);

JsonValue MakeJsonValue(long long unsigned int  // NOLINT(runtime/int)
                        unsigned_value);

JsonValue MakeJsonValue(float float_value);

JsonValue MakeJsonValue(double double_value);

JsonValue MakeJsonValue(const char* string_value);

JsonValue MakeJsonValue(StringPiece string_value);

const JsonValue& MakeJsonValue(const JsonValue& json_value);

JsonValue MakeJsonValue(const google::protobuf::Message& message);

template <typename Value>
JsonValue::Property MakeJsonProperty(StringPiece name, const Value& value) {
  JsonValue::Property property;
  name.CopyToString(property.mutable_name());
  *property.mutable_value() = MakeJsonValue(value);
  return property;
}

JsonValue MakeJsonObject();

JsonValue MakeJsonArray();

JsonValue MakeJsonObject(const google::protobuf::RepeatedPtrField<
    JsonValue::Property>& object_properties);

JsonValue MakeJsonArray(
    const google::protobuf::RepeatedPtrField<JsonValue>& array_elements);

}  // namespace pjcore

#include "pjcore/make_json_value_pump.h"

#endif  // PJCORE_MAKE_JSON_VALUE_H_
