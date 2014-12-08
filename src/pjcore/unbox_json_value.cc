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

#include "pjcore/unbox_json_value.h"

#include <limits>

#include "pjcore/logging.h"
#include "pjcore/number_util.h"

#ifdef min
#undef min
#endif

#ifdef max
#undef max
#endif

namespace pjcore {

bool UnboxJsonValue(const JsonValue& json_value, bool* bool_value,
                    Error* error) {
  PJCORE_CHECK(bool_value);
  *bool_value = false;
  PJCORE_CHECK(error);
  error->Clear();

  switch (json_value.type()) {
    case JsonValue::TYPE_STRING:
      PJCORE_REQUIRE(ReadNumber(json_value.string_value(), bool_value),
                     "Invalid string for bool");
      return true;

    case JsonValue::TYPE_SIGNED:
      *bool_value = json_value.signed_value() != 0;
      return true;

    case JsonValue::TYPE_UNSIGNED:
      *bool_value = json_value.unsigned_value() != 0;
      return true;

    case JsonValue::TYPE_DOUBLE:
      *bool_value = json_value.double_value() != 0;
      return true;

    case JsonValue::TYPE_BOOL:
      *bool_value = json_value.bool_value();
      return true;

    default:
      PJCORE_FAIL("Type not mapped to bool");
  }
}

bool UnboxJsonValue(const JsonValue& json_value, int32_t* signed_value,
                    Error* error) {
  PJCORE_CHECK(signed_value);
  *signed_value = 0;
  PJCORE_CHECK(error);
  error->Clear();

  int64_t candidate;
  PJCORE_REQUIRE_SILENT(UnboxJsonValue(json_value, &candidate, error),
                        "Invalid value for int64_t");

  PJCORE_REQUIRE(candidate <= std::numeric_limits<int32_t>::max(),
                 "int32_t overflow");

  PJCORE_REQUIRE(candidate >= std::numeric_limits<int32_t>::min(),
                 "int32_t underflow");

  *signed_value = static_cast<int32_t>(candidate);
  return true;
}

bool UnboxJsonValue(const JsonValue& json_value, int64_t* signed_value,
                    Error* error) {
  PJCORE_CHECK(signed_value);
  *signed_value = 0;
  PJCORE_CHECK(error);
  error->Clear();

  switch (json_value.type()) {
    case JsonValue::TYPE_STRING:
      PJCORE_REQUIRE(ReadNumber(json_value.string_value(), signed_value),
                     "Invalid string for int64_t");
      return true;

    case JsonValue::TYPE_SIGNED:
      *signed_value = json_value.signed_value();
      return true;

    case JsonValue::TYPE_UNSIGNED:
      PJCORE_REQUIRE(
          json_value.unsigned_value() <=
              static_cast<uint64_t>(std::numeric_limits<int64_t>::max()),
          "int64_t overflow");
      *signed_value = static_cast<int64_t>(json_value.unsigned_value());
      return true;

    case JsonValue::TYPE_DOUBLE: {
      PJCORE_REQUIRE(
          json_value.double_value() <=
              static_cast<double>(std::numeric_limits<int64_t>::max()),
          "int64_t overflow");

      PJCORE_REQUIRE(
          json_value.double_value() >=
              static_cast<double>(std::numeric_limits<int64_t>::min()),
          "int64_t underflow");

      int64_t candidate = static_cast<int64_t>(json_value.double_value());

      PJCORE_REQUIRE(
          static_cast<double>(candidate) == json_value.double_value(),
          "int64_t loss of precision");

      *signed_value = candidate;
      return true;
    }

    case JsonValue::TYPE_BOOL:
      *signed_value = json_value.bool_value() ? 1 : 0;
      return true;

    default:
      PJCORE_FAIL("Type not mapped to int64_t");
  }
  return true;
}

#ifdef PJCORE_DISTINCT_LONG_LONG
bool UnboxJsonValue(const JsonValue& json_value,
                    long long int* signed_value,  // NOLINT(runtime/int)
                    Error* error) {
  PJCORE_CHECK(signed_value);
  *signed_value = 0;

  int64_t candidate;
  PJCORE_REQUIRE_SILENT(UnboxJsonValue(json_value, &candidate, error),
                        "Invalid value for int64_t");

  *signed_value = static_cast<long long int>(  // NOLINT(runtime/int)
      candidate);
  return true;
}
#endif  // PJCORE_DISTINCT_LONG_LONG

bool UnboxJsonValue(const JsonValue& json_value, uint32_t* unsigned_value,
                    Error* error) {
  PJCORE_CHECK(unsigned_value);
  *unsigned_value = 0;
  PJCORE_CHECK(error);
  error->Clear();

  uint64_t candidate;
  PJCORE_REQUIRE_SILENT(UnboxJsonValue(json_value, &candidate, error),
                        "Invalid value for uint64_t");

  PJCORE_REQUIRE(candidate <= std::numeric_limits<uint32_t>::max(),
                 "uint32_t overflow");

  *unsigned_value = static_cast<uint32_t>(candidate);
  return true;
}

bool UnboxJsonValue(const JsonValue& json_value, uint64_t* unsigned_value,
                    Error* error) {
  PJCORE_CHECK(unsigned_value);
  *unsigned_value = 0;
  PJCORE_CHECK(error);
  error->Clear();

  switch (json_value.type()) {
    case JsonValue::TYPE_STRING:
      PJCORE_REQUIRE(ReadNumber(json_value.string_value(), unsigned_value),
                     "Invalid string for uint64_t");
      return true;

    case JsonValue::TYPE_SIGNED:
      PJCORE_REQUIRE(json_value.signed_value() >= 0, "uint64_t underflow");
      *unsigned_value = static_cast<uint64_t>(json_value.signed_value());
      return true;

    case JsonValue::TYPE_UNSIGNED:
      *unsigned_value = json_value.unsigned_value();
      return true;

    case JsonValue::TYPE_DOUBLE: {
      PJCORE_REQUIRE(json_value.double_value() >= 0, "uint64_t underflow");

      PJCORE_REQUIRE(
          json_value.double_value() <=
              static_cast<double>(std::numeric_limits<uint64_t>::max()),
          "uint64_t overflow");

      uint64_t candidate = static_cast<uint64_t>(json_value.double_value());
      PJCORE_REQUIRE(
          static_cast<double>(candidate) == json_value.double_value(),
          "uint64_t loss of precision");

      *unsigned_value = candidate;
      return true;
    }

    case JsonValue::TYPE_BOOL:
      *unsigned_value = json_value.bool_value() ? 1 : 0;
      return true;

    default:
      PJCORE_FAIL("Type not mapped to uint64_t");
  }
}

#ifdef PJCORE_DISTINCT_LONG_LONG
bool UnboxJsonValue(const JsonValue& json_value,
                    long long unsigned int*  // NOLINT(runtime/int)
                    unsigned_value,
                    Error* error) {
  PJCORE_CHECK(unsigned_value);
  *unsigned_value = 0;

  uint64_t candidate;
  PJCORE_REQUIRE_SILENT(UnboxJsonValue(json_value, &candidate, error),
                        "Invalid value for uint64_t");

  *unsigned_value = static_cast<long long unsigned int>(  // NOLINT(runtime/int)
      candidate);
  return true;
}
#endif  // PJCORE_DISTINCT_LONG_LONG

bool UnboxJsonValue(const JsonValue& json_value, float* float_value,
                    Error* error) {
  PJCORE_CHECK(float_value);
  *float_value = 0;
  PJCORE_CHECK(error);
  error->Clear();

  double candidate;
  PJCORE_REQUIRE_SILENT(UnboxJsonValue(json_value, &candidate, error),
                        "Invalid value for double");

  *float_value = static_cast<float>(candidate);
  return true;
}

bool UnboxJsonValue(const JsonValue& json_value, double* double_value,
                    Error* error) {
  PJCORE_CHECK(double_value);
  *double_value = 0;
  PJCORE_CHECK(error);
  error->Clear();

  switch (json_value.type()) {
    case JsonValue::TYPE_STRING:
      PJCORE_REQUIRE(ReadNumber(json_value.string_value(), double_value),
                     "Invalid string for double");
      return true;

    case JsonValue::TYPE_SIGNED:
      *double_value = static_cast<double>(json_value.signed_value());
      return true;

    case JsonValue::TYPE_UNSIGNED:
      *double_value = static_cast<double>(json_value.unsigned_value());
      return true;

    case JsonValue::TYPE_DOUBLE:
      *double_value = json_value.double_value();
      return true;

    case JsonValue::TYPE_BOOL:
      *double_value = json_value.bool_value() ? 1 : 0;
      return true;

    default:
      PJCORE_FAIL("Type not mapped to double");
  }
}

bool UnboxJsonValue(const JsonValue& json_value, std::string* string_value,
                    Error* error) {
  PJCORE_CHECK(string_value);
  string_value->clear();
  PJCORE_CHECK(error);
  error->Clear();

  switch (json_value.type()) {
    case JsonValue::TYPE_STRING:
      *string_value = json_value.string_value();
      return true;

    default:
      PJCORE_FAIL("Type not mapped to string");
  }
}

bool UnboxJsonValue(const JsonValue& json_value, JsonValue* json_value_copy,
                    Error* error) {
  PJCORE_CHECK(json_value_copy);
  PJCORE_CHECK(error);
  error->Clear();

  *json_value_copy = json_value;
  return true;
}

}  // namespace pjcore
