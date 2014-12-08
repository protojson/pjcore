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

#include "pjcore/json_properties.h"

#include <utility>

#include "pjcore/logging.h"
#include "pjcore/json_util.h"
#include "pjcore/name_value_util.h"

namespace pjcore {

void NormalizeJsonProperties(JsonPropertyList* mutable_properties) {
  PJCORE_CHECK(mutable_properties);

  StableSortAndRemoveDuplicatesByName(mutable_properties);

  for (JsonPropertyList::iterator it = mutable_properties->begin();
       it != mutable_properties->end(); ++it) {
    NormalizeJsonProperties(it->mutable_value());
  }
}

void NormalizeJsonProperties(JsonValue* mutable_value) {
  switch (mutable_value->type()) {
    case JsonValue::TYPE_OBJECT:
      NormalizeJsonProperties(mutable_value->mutable_object_properties());
      break;

    case JsonValue::TYPE_ARRAY:
      for (int index = 0; index < mutable_value->array_elements_size();
           ++index) {
        NormalizeJsonProperties(mutable_value->mutable_array_elements(index));
      }
      break;

    default:
      break;
  }
}

static bool AreJsonPropertiesNormalizedRecursive(
    const JsonValue& value, std::string* optional_unnormalized_path);

static bool AreJsonPropertiesNormalizedRecursive(
    const JsonPropertyList& properties,
    std::string* optional_unnormalized_path) {
  JsonPropertyList::const_iterator it = properties.begin();
  if (it == properties.end()) {
    return true;
  }
  JsonPropertyList::const_iterator next = it;
  ++next;
  while (next != properties.end()) {
    if (!(it->name() < next->name())) {
      if (optional_unnormalized_path) {
        AppendJsonPathProperty(next->name(), optional_unnormalized_path);
      }
      return false;
    }

    it = next;
    ++next;
  }
  for (it = properties.begin(); it != properties.end(); ++it) {
    size_t previous_path_length = 0;
    if (optional_unnormalized_path) {
      previous_path_length = optional_unnormalized_path->length();
      AppendJsonPathProperty(it->name(), optional_unnormalized_path);
    }
    if (!AreJsonPropertiesNormalizedRecursive(it->value(),
                                              optional_unnormalized_path)) {
      return false;
    }
    if (optional_unnormalized_path) {
      optional_unnormalized_path->resize(previous_path_length);
    }
  }
  return true;
}

static bool AreJsonPropertiesNormalizedRecursive(
    const JsonValue& value, std::string* optional_unnormalized_path) {
  switch (value.type()) {
    case JsonValue::TYPE_OBJECT:
      return AreJsonPropertiesNormalizedRecursive(value.object_properties(),
                                                  optional_unnormalized_path);

    case JsonValue::TYPE_ARRAY:
      for (int index = 0; index < value.array_elements_size(); ++index) {
        size_t previous_path_length = 0;
        if (optional_unnormalized_path) {
          previous_path_length = optional_unnormalized_path->length();
          AppendJsonPathElement(index, optional_unnormalized_path);
        }
        if (!AreJsonPropertiesNormalizedRecursive(value.array_elements(index),
                                                  optional_unnormalized_path)) {
          return false;
        }
        if (optional_unnormalized_path) {
          optional_unnormalized_path->resize(previous_path_length);
        }
      }
      return true;

    default:
      return true;
  }
}

bool AreJsonPropertiesNormalized(const JsonPropertyList& properties,
                                 std::string* optional_unnormalized_path) {
  if (optional_unnormalized_path) {
    *optional_unnormalized_path = GetRootJsonPath();
  }
  return AreJsonPropertiesNormalizedRecursive(properties,
                                              optional_unnormalized_path);
}

bool AreJsonPropertiesNormalized(const JsonValue& value,
                                 std::string* optional_unnormalized_path) {
  if (optional_unnormalized_path) {
    *optional_unnormalized_path = GetRootJsonPath();
  }
  return AreJsonPropertiesNormalizedRecursive(value,
                                              optional_unnormalized_path);
}

bool HasJsonProperty(const JsonPropertyList& properties, StringPiece name) {
  return FindByName(properties, name);
}

bool HasJsonProperty(const JsonValue& object_value, StringPiece name) {
  PJCORE_CHECK_EQ(JsonValue::TYPE_OBJECT, object_value.type());
  return HasJsonProperty(object_value.object_properties(), name);
}

const JsonValue& GetJsonProperty(const JsonPropertyList& properties,
                                 StringPiece name) {
  const JsonValue::Property* property = FindByName(properties, name);

  return property ? property->value() : JsonNull();
}

const JsonValue& GetJsonProperty(const JsonValue& object_value,
                                 StringPiece name) {
  PJCORE_CHECK_EQ(JsonValue::TYPE_OBJECT, object_value.type());
  return GetJsonProperty(object_value.object_properties(), name);
}

JsonValue* GetMutableJsonProperty(JsonPropertyList* mutable_properties,
                                  StringPiece name) {
  std::pair<JsonValue::Property*, bool> insert_result = InsertName(
      mutable_properties,
      name.as_string());  // TODO(pjcore): set_name() requires std::string

  return insert_result.first->mutable_value();
}

JsonValue* GetMutableJsonProperty(JsonValue* mutable_object_value,
                                  StringPiece name) {
  PJCORE_CHECK(mutable_object_value);
  PJCORE_CHECK_EQ(JsonValue::TYPE_OBJECT, mutable_object_value->type());
  return GetMutableJsonProperty(
      mutable_object_value->mutable_object_properties(), name);
}

void ClearJsonProperty(JsonPropertyList* mutable_properties, StringPiece name) {
  int from = 0;
  int to = from;
  while (from < mutable_properties->size()) {
    if (!(mutable_properties->Get(from).name() == name)) {
      mutable_properties->SwapElements(from, to);
      ++to;
    }
    ++from;
  }
  while (to < mutable_properties->size()) {
    mutable_properties->RemoveLast();
  }
}

void ClearJsonProperty(JsonValue* mutable_object_value, StringPiece name) {
  PJCORE_CHECK_EQ(JsonValue::TYPE_OBJECT, mutable_object_value->type());
  ClearJsonProperty(mutable_object_value->mutable_object_properties(), name);
}

}  // namespace pjcore
