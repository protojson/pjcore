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

#ifndef PJCORE_JSON_PROPERTIES_H_
#define PJCORE_JSON_PROPERTIES_H_

#include <string>

#include "pjcore/third_party/chromium/string_piece.h"

#include "pjcore/json.pb.h"
#include "pjcore/make_json_value.h"

namespace pjcore {

typedef google::protobuf::RepeatedPtrField<JsonValue::Property>
    JsonPropertyList;

void NormalizeJsonProperties(JsonPropertyList* mutable_properties);

void NormalizeJsonProperties(JsonValue* mutable_value);

bool AreJsonPropertiesNormalized(
    const JsonPropertyList& properties,
    std::string* optional_unnormalized_path = NULL);

bool AreJsonPropertiesNormalized(
    const JsonValue& value, std::string* optional_unnormalized_path = NULL);

bool HasJsonProperty(const JsonPropertyList& properties, StringPiece name);

bool HasJsonProperty(const JsonValue& object_value, StringPiece name);

template <typename WithJsonProperties>
bool HasJsonProperty(const WithJsonProperties& object_value, StringPiece name) {
  return HasJsonProperty(object_value.object_properties(), name);
}

const JsonValue& GetJsonProperty(const JsonPropertyList& properties,
                                 StringPiece name);

const JsonValue& GetJsonProperty(const JsonValue& object_value,
                                 StringPiece name);

template <typename WithJsonProperties>
const JsonValue& GetJsonProperty(const WithJsonProperties& object_value,
                                 StringPiece name) {
  return GetJsonProperty(object_value.object_properties(), name);
}

JsonValue* GetMutableJsonProperty(JsonPropertyList* mutable_properties,
                                  StringPiece name);

JsonValue* GetMutableJsonProperty(JsonValue* mutable_object_value,
                                  StringPiece name);

template <typename WithJsonProperties>
JsonValue* GetMutableJsonProperty(WithJsonProperties* mutable_object_value,
                                  StringPiece name) {
  return GetMutableJsonProperty(
      mutable_object_value->mutable_object_properties(), name);
}

template <typename Value>
void SetJsonProperty(JsonPropertyList* mutable_properties, StringPiece name,
                     const Value& value) {
  *GetMutableJsonProperty(mutable_properties, name) = MakeJsonValue(value);
}

template <typename Value>
void SetJsonProperty(JsonValue* mutable_object_value, StringPiece name,
                     const Value& value) {
  *GetMutableJsonProperty(mutable_object_value, name) = MakeJsonValue(value);
}

template <typename WithJsonProperties, typename Value>
void SetJsonProperty(WithJsonProperties* mutable_object_value, StringPiece name,
                     const Value& value) {
  *GetMutableJsonProperty(mutable_object_value, name) = MakeJsonValue(value);
}

void ClearJsonProperty(JsonPropertyList* mutable_properties, StringPiece name);

void ClearJsonProperty(JsonValue* mutable_object_value, StringPiece name);

template <typename WithJsonProperties>
void ClearJsonProperty(WithJsonProperties* mutable_object_value,
                       StringPiece name) {
  ClearJsonProperty(mutable_object_value->mutable_object_properties(), name);
}

}  // namespace pjcore

#endif  // PJCORE_JSON_PROPERTIES_H_
