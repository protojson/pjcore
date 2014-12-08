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

#include <gtest/gtest.h>

#include "pjcore/logging.h"

namespace pjcore {

TEST(VerifyJsonType, Valid) {
  std::string error;

  EXPECT_TRUE(VerifyJsonType(JsonValue::default_instance(), &error));
  EXPECT_TRUE(VerifyJsonType(JsonNull(), &error));
  EXPECT_TRUE(VerifyJsonType(MakeJsonValue(true), &error));
  EXPECT_TRUE(VerifyJsonType(MakeJsonValue(-123), &error));
  EXPECT_TRUE(VerifyJsonType(MakeJsonValue(456ull), &error));
  EXPECT_TRUE(VerifyJsonType(MakeJsonValue(7.89), &error));
  EXPECT_TRUE(VerifyJsonType(MakeJsonValue("alpha"), &error));
  EXPECT_TRUE(VerifyJsonType(MakeJsonObject("alpha", "beta"), &error));
  EXPECT_TRUE(VerifyJsonType(MakeJsonArray(1, 2, 3), &error));
}

TEST(VerifyJsonType, Invalid) {
  std::string error;

  GlobalLogOverride global_log_override;

  {
    JsonValue value = JsonNull();
    value.set_bool_value(true);
    EXPECT_FALSE(VerifyJsonType(value, &error));
  }

  {
    JsonValue value = MakeJsonValue(true);
    value.set_signed_value(-123);
    EXPECT_FALSE(VerifyJsonType(value, &error));
  }

  {
    JsonValue value = MakeJsonValue(-123);
    value.set_unsigned_value(456ull);
    EXPECT_FALSE(VerifyJsonType(value, &error));
  }

  {
    JsonValue value = MakeJsonValue(456ull);
    value.set_double_value(7.89);
    EXPECT_FALSE(VerifyJsonType(value, &error));
  }

  {
    JsonValue value = MakeJsonValue(7.89);
    value.set_string_value("alpha");
    EXPECT_FALSE(VerifyJsonType(value, &error));
  }

  {
    JsonValue value = MakeJsonValue("alpha");
    *value.add_object_properties() = MakeJsonProperty("alpha", "beta");
    EXPECT_FALSE(VerifyJsonType(value, &error));
  }

  {
    JsonValue value = MakeJsonObject("alpha", "beta");
    *value.add_array_elements() = MakeJsonValue(1);
    EXPECT_FALSE(VerifyJsonType(value, &error));
  }
}

TEST(IsJsonNumber, Test) {
  EXPECT_FALSE(IsJsonNumber(JsonNull()));
  EXPECT_FALSE(IsJsonNumber(MakeJsonValue(true)));
  EXPECT_TRUE(IsJsonNumber(MakeJsonValue(-123)));
  EXPECT_TRUE(IsJsonNumber(MakeJsonValue(456ull)));
  EXPECT_TRUE(IsJsonNumber(MakeJsonValue(7.89)));
  EXPECT_FALSE(IsJsonNumber(MakeJsonValue("alpha")));
  EXPECT_FALSE(IsJsonNumber(MakeJsonObject("alpha", "beta")));
  EXPECT_FALSE(IsJsonNumber(MakeJsonArray(1, 2, 3)));
}

TEST(AreJsonValuesEqual, JsonNull) {
  EXPECT_TRUE(AreJsonValuesEqual(JsonNull(), JsonNull()));

  EXPECT_FALSE(AreJsonValuesEqual(JsonNull(), true));
}

TEST(AreJsonValuesEqual, BoolValue) {
  EXPECT_TRUE(AreJsonValuesEqual(true, true));
  EXPECT_FALSE(AreJsonValuesEqual(true, false));

  EXPECT_FALSE(AreJsonValuesEqual(true, 1));
}

TEST(AreJsonValuesEqual, SignedValue) {
  EXPECT_TRUE(AreJsonValuesEqual(-123, -123));
  EXPECT_FALSE(AreJsonValuesEqual(-123, -321));
  EXPECT_TRUE(AreJsonValuesEqual(123, 123ull));
  EXPECT_FALSE(AreJsonValuesEqual(123, 124ull));
  EXPECT_TRUE(AreJsonValuesEqual(123, 123.0));
  EXPECT_FALSE(AreJsonValuesEqual(123, 123.1));

  EXPECT_FALSE(AreJsonValuesEqual(-123, "alpha"));
  EXPECT_FALSE(AreJsonValuesEqual(-123, "-123"));
}

TEST(AreJsonValuesEqual, UnsignedValue) {
  EXPECT_TRUE(AreJsonValuesEqual(456ull, 456ull));
  EXPECT_FALSE(AreJsonValuesEqual(456ull, 654ull));
  EXPECT_TRUE(AreJsonValuesEqual(456ull, 456));
  EXPECT_FALSE(AreJsonValuesEqual(456ull, 457));
  EXPECT_TRUE(AreJsonValuesEqual(456ull, 456.0));
  EXPECT_FALSE(AreJsonValuesEqual(456ull, 456.1));

  EXPECT_FALSE(AreJsonValuesEqual(456, "alpha"));
  EXPECT_FALSE(AreJsonValuesEqual(456, "456"));
}

TEST(AreJsonValuesEqual, DoubleValue) {
  EXPECT_TRUE(AreJsonValuesEqual(7.89, 7.89));
  EXPECT_FALSE(AreJsonValuesEqual(7.89, 9.87));
  EXPECT_TRUE(AreJsonValuesEqual(789.0, 789));
  EXPECT_FALSE(AreJsonValuesEqual(789.1, 789));
  EXPECT_TRUE(AreJsonValuesEqual(789.0, 789ull));
  EXPECT_FALSE(AreJsonValuesEqual(789.1, 789ull));

  EXPECT_TRUE(AreJsonValuesEqual(JsonNaN(), JsonNaN()));
  EXPECT_FALSE(AreJsonValuesEqual(123.0, JsonNaN()));

  EXPECT_TRUE(AreJsonValuesEqual(JsonInfinity(), JsonInfinity()));
  EXPECT_FALSE(AreJsonValuesEqual(JsonNaN(), JsonInfinity()));

  EXPECT_TRUE(
      AreJsonValuesEqual(JsonNegativeInfinity(), JsonNegativeInfinity()));

  EXPECT_FALSE(AreJsonValuesEqual(JsonInfinity(), JsonNegativeInfinity()));

  EXPECT_FALSE(AreJsonValuesEqual(7.89, "alpha"));
  EXPECT_FALSE(AreJsonValuesEqual(7.89, "7.89"));
}

TEST(AreJsonValuesEqual, StringValue) {
  EXPECT_TRUE(AreJsonValuesEqual("alpha", "alpha"));
  EXPECT_FALSE(AreJsonValuesEqual("alpha", "beta'"));

  EXPECT_FALSE(AreJsonValuesEqual("alpha", MakeJsonObject("alpha", "beta")));
}

TEST(AreJsonValuesEqual, ObjectValue) {
  EXPECT_TRUE(AreJsonValuesEqual(MakeJsonObject(), MakeJsonObject()));

  EXPECT_FALSE(
      AreJsonValuesEqual(MakeJsonObject(), MakeJsonObject("alpha", "beta")));

  EXPECT_TRUE(AreJsonValuesEqual(MakeJsonObject("alpha", "beta"),
                                 MakeJsonObject("alpha", "beta")));

  EXPECT_FALSE(AreJsonValuesEqual(MakeJsonObject("alpha", "beta"),
                                  MakeJsonObject("beta", "alpha")));

  EXPECT_FALSE(AreJsonValuesEqual(MakeJsonObject("alpha", "beta"),
                                  MakeJsonObject("alpha", "gamma")));

  EXPECT_TRUE(
      AreJsonValuesEqual(MakeJsonObject("alpha", "beta", "gamma", "delta"),
                         MakeJsonObject("alpha", "beta", "gamma", "delta")));

  EXPECT_FALSE(
      AreJsonValuesEqual(MakeJsonObject("alpha", "beta", "gamma", "delta"),
                         MakeJsonObject("gamma", "delta", "alpha", "beta")));

  EXPECT_FALSE(AreJsonValuesEqual(MakeJsonObject("alpha", "beta"),
                                  MakeJsonArray("alpha", "beta")));
}

TEST(AreJsonValuesEqual, ArrayValue) {
  EXPECT_TRUE(AreJsonValuesEqual(MakeJsonArray(), MakeJsonArray()));
  EXPECT_FALSE(AreJsonValuesEqual(MakeJsonArray(), MakeJsonArray(1)));
  EXPECT_TRUE(AreJsonValuesEqual(MakeJsonArray(1), MakeJsonArray(1)));
  EXPECT_FALSE(AreJsonValuesEqual(MakeJsonArray(1, 2), MakeJsonArray(1)));
  EXPECT_TRUE(AreJsonValuesEqual(MakeJsonArray(1, 2), MakeJsonArray(1, 2)));
  EXPECT_FALSE(AreJsonValuesEqual(MakeJsonArray(1, 2), MakeJsonArray(2, 1)));

  EXPECT_FALSE(AreJsonValuesEqual(MakeJsonArray(1, 2), JsonNull()));
}

TEST(StripQuotesUnescapeTabsAndSlashes, Test) {
  EXPECT_EQ("alpha", StripQuotesUnescapeTabsAndSlashes("\"alpha\""));
  EXPECT_EQ("\"", StripQuotesUnescapeTabsAndSlashes("\"\\\"\""));
  EXPECT_EQ("\t", StripQuotesUnescapeTabsAndSlashes("\"\\t\""));
  EXPECT_EQ("\\", StripQuotesUnescapeTabsAndSlashes("\"\\\\\""));
  EXPECT_EQ("/", StripQuotesUnescapeTabsAndSlashes("\"\\/\""));
}

}  // namespace pjcore
