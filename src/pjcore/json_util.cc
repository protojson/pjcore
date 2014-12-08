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

#include "pjcore/json_util.h"

#include <algorithm>
#include <limits>
#include <utility>

#include "pjcore/logging.h"
#include "pjcore/make_json_value.h"
#include "pjcore/name_value_util.h"
#include "pjcore/number_util.h"
#include "pjcore/repeated_field_util.h"
#include "pjcore/third_party/chromium/macros.h"
#include "pjcore/third_party/gtest/are_almost_equal.h"
#include "pjcore/unique_keep_last.h"

#ifdef min
#undef min
#endif

#ifdef max
#undef max
#endif

namespace pjcore {

namespace {

const std::string kRootJsonPath("$");

}  // unnamed namespace

bool VerifyJsonType(const JsonValue& value, std::string* error) {
  PJCORE_CHECK(error);

  static const struct {
    bool (JsonValue::*type_method)() const;

    const char* type_error;
  } kTypeInfos[] = {
        {&JsonValue::has_type, ""},
        {&JsonValue::has_bool_value, "No bool_value for TYPE_BOOL"},
        {&JsonValue::has_signed_value, "No signed_value for TYPE_SIGNED"},
        {&JsonValue::has_unsigned_value, "No unsigned_value for TYPE_UNSIGNED"},
        {&JsonValue::has_double_value, "No double_value for TYPE_DOUBLE"},
        {&JsonValue::has_string_value, "No string_value for TYPE_STRING"},
        {&JsonValue::has_type, ""},
        {&JsonValue::has_type, ""},
    };

  if (value.has_type()) {
    PJCORE_REQUIRE_STRING(
        static_cast<int>(value.type()) >= 0 &&
            static_cast<int>(value.type()) < ARRAYSIZE_UNSAFE(kTypeInfos),
        std::string("Unknown type ") +
            WriteNumber(static_cast<int>(value.type())));

    PJCORE_REQUIRE_STRING(
        (value.*kTypeInfos[static_cast<int>(value.type())].type_method)(),
        kTypeInfos[static_cast<int>(value.type())].type_error);
  }

  if (value.has_string_value()) {
    PJCORE_REQUIRE_STRING(value.type() == JsonValue::TYPE_STRING,
                          std::string("Unexpected string_value for ") +
                              JsonValue_Type_Name(value.type()));
  }

  if (value.has_signed_value()) {
    PJCORE_REQUIRE_STRING(value.type() == JsonValue::TYPE_SIGNED,
                          std::string("Unexpected signed_value for ") +
                              JsonValue_Type_Name(value.type()));
  }

  if (value.has_unsigned_value()) {
    PJCORE_REQUIRE_STRING(value.type() == JsonValue::TYPE_UNSIGNED,
                          std::string("Unexpected unsigned_value for ") +
                              JsonValue_Type_Name(value.type()));
  }

  if (value.has_double_value()) {
    PJCORE_REQUIRE_STRING(value.type() == JsonValue::TYPE_DOUBLE,
                          std::string("Unexpected double_value for ") +
                              JsonValue_Type_Name(value.type()));
  }

  if (value.array_elements_size()) {
    PJCORE_REQUIRE_STRING(value.type() == JsonValue::TYPE_ARRAY,
                          std::string("Unexpected array_elements for ") +
                              JsonValue_Type_Name(value.type()));
  }

  if (value.object_properties_size()) {
    PJCORE_REQUIRE_STRING(value.type() == JsonValue::TYPE_OBJECT,
                          std::string("Unexpected object_properties for ") +
                              JsonValue_Type_Name(value.type()));
  }

  if (value.has_bool_value()) {
    PJCORE_REQUIRE_STRING(value.type() == JsonValue::TYPE_BOOL,
                          std::string("Unexpected bool_value for ") +
                              JsonValue_Type_Name(value.type()));
  }

  return true;
}

bool IsJsonNumber(const JsonValue& value) {
  return value.type() == JsonValue::TYPE_SIGNED ||
         value.type() == JsonValue::TYPE_UNSIGNED ||
         value.type() == JsonValue::TYPE_DOUBLE;
}

bool IsJsonNumber(const JsonValue& value);

static bool AreJsonValuesEqualRecursive(const JsonValue& left,
                                        const JsonValue& right,
                                        std::string* optional_diff_path) {
  switch (left.type()) {
    case JsonValue::TYPE_NULL:
      return right.type() == JsonValue::TYPE_NULL;

    case JsonValue::TYPE_STRING:
      return right.type() == JsonValue::TYPE_STRING &&
             left.string_value() == right.string_value();

    case JsonValue::TYPE_SIGNED:
      switch (right.type()) {
        case JsonValue::TYPE_SIGNED:
          return left.signed_value() == right.signed_value();

        case JsonValue::TYPE_UNSIGNED:
          return left.signed_value() >= 0 &&
                 static_cast<uint64_t>(left.signed_value()) ==
                     right.unsigned_value();

        case JsonValue::TYPE_DOUBLE:
          return right.double_value() >=
                     static_cast<double>(std::numeric_limits<int64_t>::min()) &&
                 right.double_value() <=
                     static_cast<double>(std::numeric_limits<int64_t>::max()) &&
                 static_cast<double>(left.signed_value()) ==
                     right.double_value() &&
                 left.signed_value() ==
                     static_cast<int64_t>(right.double_value());

        default:
          return false;
      }

    case JsonValue::TYPE_UNSIGNED:
      switch (right.type()) {
        case JsonValue::TYPE_SIGNED:
          return right.signed_value() >= 0 &&
                 left.unsigned_value() ==
                     static_cast<uint64_t>(right.signed_value());

        case JsonValue::TYPE_UNSIGNED:
          return left.unsigned_value() == right.unsigned_value();

        case JsonValue::TYPE_DOUBLE:
          return right.double_value() >= 0 &&
                 right.double_value() <=
                     static_cast<double>(
                         std::numeric_limits<uint64_t>::max()) &&
                 static_cast<double>(left.unsigned_value()) ==
                     right.double_value() &&
                 left.unsigned_value() ==
                     static_cast<uint64_t>(right.double_value());

        default:
          return false;
      }

    case JsonValue::TYPE_DOUBLE:
      switch (right.type()) {
        case JsonValue::TYPE_SIGNED:
          return left.double_value() >=
                     static_cast<double>(std::numeric_limits<int64_t>::min()) &&
                 left.double_value() <=
                     static_cast<double>(std::numeric_limits<int64_t>::max()) &&
                 left.double_value() ==
                     static_cast<double>(right.signed_value()) &&
                 static_cast<int64_t>(left.double_value()) ==
                     right.signed_value();

        case JsonValue::TYPE_UNSIGNED:
          return left.double_value() >= 0 &&
                 left.double_value() <=
                     static_cast<double>(
                         std::numeric_limits<uint64_t>::max()) &&
                 left.double_value() ==
                     static_cast<double>(right.unsigned_value()) &&
                 static_cast<uint64_t>(left.double_value()) ==
                     right.unsigned_value();

        case JsonValue::TYPE_DOUBLE:
          if (left.double_value() != left.double_value()) {
            return right.double_value() != right.double_value();
          } else if (right.double_value() != right.double_value()) {
            return false;
          } else {
            return AreAlmostEqual(left.double_value(), right.double_value());
          }

        default:
          return false;
      }

    case JsonValue::TYPE_OBJECT: {
      if (right.type() != JsonValue::TYPE_OBJECT) {
        return false;
      }
      google::protobuf::RepeatedPtrField<JsonValue::Property>::const_iterator
          left_it = left.object_properties().begin(),
          right_it = right.object_properties().begin();

      while (left_it != left.object_properties().end()) {
        if (right_it == right.object_properties().end() ||
            left_it->name() < right_it->name()) {
          if (optional_diff_path) {
            AppendJsonPathProperty(left_it->name(), optional_diff_path);
          }
          return false;
        }
        if (right_it->name() < left_it->name()) {
          if (optional_diff_path) {
            AppendJsonPathProperty(right_it->name(), optional_diff_path);
          }
          return false;
        }

        size_t previous_path_length = 0;
        if (optional_diff_path) {
          previous_path_length = optional_diff_path->length();
          AppendJsonPathProperty(left_it->name(), optional_diff_path);
        }
        if (!AreJsonValuesEqualRecursive(left_it->value(), right_it->value(),
                                         optional_diff_path)) {
          return false;
        }
        if (optional_diff_path) {
          optional_diff_path->resize(previous_path_length);
        }
        ++left_it;
        ++right_it;
      }

      if (right_it != right.object_properties().end()) {
        if (optional_diff_path) {
          AppendJsonPathProperty(right_it->name(), optional_diff_path);
        }
        return false;
      }

      return true;
    }

    case JsonValue::TYPE_ARRAY:
      if (right.type() != JsonValue::TYPE_ARRAY) {
        return false;
      }

      for (int index = 0; index < std::min(left.array_elements_size(),
                                           right.array_elements_size());
           ++index) {
        size_t previous_path_length = 0;
        if (optional_diff_path) {
          previous_path_length = optional_diff_path->length();
          AppendJsonPathElement(index, optional_diff_path);
        }
        if (!AreJsonValuesEqualRecursive(left.array_elements(index),
                                         right.array_elements(index),
                                         optional_diff_path)) {
          return false;
        }
        if (optional_diff_path) {
          optional_diff_path->resize(previous_path_length);
        }
      }
      if (left.array_elements_size() != right.array_elements_size()) {
        if (optional_diff_path) {
          AppendJsonPathElement(
              std::min(left.array_elements_size(), right.array_elements_size()),
              optional_diff_path);
        }
        return false;
      }
      return true;

    case JsonValue::TYPE_BOOL:
      return left.bool_value() == right.bool_value();

    default:
      return false;
  }
}

bool AreJsonValuesEqual(const JsonValue& left, const JsonValue& right,
                        std::string* optional_diff_path) {
  if (optional_diff_path) {
    *optional_diff_path = GetRootJsonPath();
  }

  return AreJsonValuesEqualRecursive(left, right, optional_diff_path);
}

const std::string& GetRootJsonPath() { return kRootJsonPath; }

void AppendJsonPathProperty(StringPiece property_name, std::string* json_path) {
  PJCORE_CHECK(json_path);
  json_path->push_back('.');
  property_name.AppendToString(json_path);
}

void AppendJsonPathElement(int64_t element_index, std::string* json_path) {
  PJCORE_CHECK(json_path);
  json_path->push_back('[');
  AppendNumber(element_index, json_path);
  json_path->push_back(']');
}

std::string StripQuotesUnescapeTabsAndSlashes(StringPiece str) {
  std::string result;
  result.reserve(str.size());

  for (size_t offset = 0; offset < str.size(); ++offset) {
    switch (str[offset]) {
      case '"':
        break;

      case '\\':
        if (offset + 1 >= str.size()) {
          result.push_back('\\');
        } else {
          ++offset;

          switch (str[offset]) {
            case '"':
            case '\\':
            case '/':
              result.push_back(str[offset]);
              break;

            case 't':
              result.push_back('\t');
              break;

            default:
              result.push_back('\\');
              result.push_back(str[offset]);
              break;
          }
        }
        break;

      default:
        result.push_back(str[offset]);
        break;
    }
  }

  return result;
}

}  // namespace pjcore
