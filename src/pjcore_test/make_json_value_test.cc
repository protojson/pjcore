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

#include <gtest/gtest.h>

#include "pjcore/json_util.h"

namespace pjcore {

TEST(MakeJsonValue, JsonNull) {
  JsonValue value = JsonNull();
  std::string error;
  ASSERT_TRUE(VerifyJsonType(value, &error));
  ASSERT_EQ(JsonValue::TYPE_NULL, value.type());
}

TEST(MakeJsonValue, BoolValue) {
  JsonValue value = MakeJsonValue(true);
  std::string error;
  ASSERT_TRUE(VerifyJsonType(value, &error));
  ASSERT_EQ(JsonValue::TYPE_BOOL, value.type());
  ASSERT_EQ(true, value.bool_value());
}

TEST(MakeJsonValue, SignedValue) {
  JsonValue value = MakeJsonValue(-123);
  std::string error;
  ASSERT_TRUE(VerifyJsonType(value, &error));
  ASSERT_EQ(JsonValue::TYPE_SIGNED, value.type());
  ASSERT_EQ(-123, value.signed_value());
}

TEST(MakeJsonValue, UnsignedValue) {
  JsonValue value = MakeJsonValue(456ull);
  std::string error;
  ASSERT_TRUE(VerifyJsonType(value, &error));
  ASSERT_EQ(JsonValue::TYPE_UNSIGNED, value.type());
  ASSERT_EQ(456, value.unsigned_value());
}

TEST(MakeJsonValue, DoubleValue) {
  JsonValue value = MakeJsonValue(-12.34e-5);
  std::string error;
  ASSERT_TRUE(VerifyJsonType(value, &error));
  ASSERT_EQ(JsonValue::TYPE_DOUBLE, value.type());
  ASSERT_EQ(-12.34e-5, value.double_value());
}

TEST(MakeJsonValue, StringValue) {
  JsonValue value = MakeJsonValue("alpha");
  std::string error;
  ASSERT_TRUE(VerifyJsonType(value, &error));
  ASSERT_EQ(JsonValue::TYPE_STRING, value.type());
  ASSERT_EQ("alpha", value.string_value());
}

TEST(MakeJsonValue, JsonProperty) {
  JsonValue::Property property = MakeJsonProperty("beta", "gamma");
  ASSERT_EQ("beta", property.name());
  ASSERT_EQ(JsonValue::TYPE_STRING, property.value().type());
  ASSERT_EQ("gamma", property.value().string_value());
}

TEST(MakeJsonValue, ObjectValue) {
  JsonValue value = MakeJsonObject("delta", 789, "epsilon", 135ull);
  std::string error;
  ASSERT_TRUE(VerifyJsonType(value, &error));
  ASSERT_EQ(JsonValue::TYPE_OBJECT, value.type());
  ASSERT_EQ(2, value.object_properties_size());
  ASSERT_EQ("delta", value.object_properties(0).name());
  ASSERT_EQ(JsonValue::TYPE_SIGNED, value.object_properties(0).value().type());
  ASSERT_EQ(789, value.object_properties(0).value().signed_value());
  ASSERT_EQ("epsilon", value.object_properties(1).name());
  ASSERT_EQ(JsonValue::TYPE_UNSIGNED,
            value.object_properties(1).value().type());
  ASSERT_EQ(135, value.object_properties(1).value().unsigned_value());
}

TEST(MakeJsonValue, ArrayValue) {
  JsonValue value = MakeJsonArray(24.68, true);
  std::string error;
  ASSERT_TRUE(VerifyJsonType(value, &error));
  ASSERT_EQ(JsonValue::TYPE_ARRAY, value.type());
  ASSERT_EQ(2, value.array_elements_size());
  ASSERT_EQ(JsonValue::TYPE_DOUBLE, value.array_elements(0).type());
  ASSERT_EQ(24.68, value.array_elements(0).double_value());
  ASSERT_EQ(JsonValue::TYPE_BOOL, value.array_elements(1).type());
  ASSERT_EQ(true, value.array_elements(1).bool_value());
}

}  // namespace pjcore
