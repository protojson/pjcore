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

#include "pjcore/json_reader.h"

#include <gtest/gtest.h>

#include "pjcore/error_util.h"
#include "pjcore/logging.h"
#include "pjcore/json_util.h"
#include "pjcore/json_writer.h"
#include "pjcore/make_json_value.h"
#include "pjcore/text_location.h"

namespace pjcore {

namespace {

template <typename Value>
::testing::AssertionResult TestReadSuccess(
    StringPiece str, const Value &expected_value,
    const JsonReaderConfig &config = JsonReaderConfig::default_instance()) {
  JsonValue actual_value;
  Error error;

  if (!ReadJson(str, &actual_value, &error, config)) {
    return ::testing::AssertionFailure() << ErrorToString(error);
  }

  std::string diff_path;
  if (!AreJsonValuesEqual(actual_value, expected_value, &diff_path)) {
    return ::testing::AssertionFailure() << "Difference at " << diff_path
                                         << ": " << WriteJson(actual_value);
  }

  return ::testing::AssertionSuccess();
}

::testing::AssertionResult TestReadFailure(
    StringPiece str, uint64_t expected_offset, StringPiece expected_description,
    const JsonReaderConfig &config = JsonReaderConfig::default_instance()) {
  PJCORE_CHECK_LE(expected_offset, str.size());
  TextLocation expected_location =
      MakeTextLocation(str.substr(0, expected_offset));
  PJCORE_CHECK_EQ(expected_offset, expected_location.offset());

  JsonValue actual_value;
  Error error;

  {
    GlobalLogOverride global_log_override;
    if (ReadJson(str, &actual_value, &error, config)) {
      return ::testing::AssertionFailure()
             << "Unexpected success: " << WriteJson(actual_value);
    }
  }

  const Error *location_error = &error;
  while (!location_error->has_text_location()) {
    if (!location_error->has_cause()) {
      return ::testing::AssertionFailure()
             << "No location: " << ErrorToString(error);
    }
    location_error = &location_error->cause();
  }

  const TextLocation &actual_location = location_error->text_location();

  if (expected_location.offset() != actual_location.offset() ||
      expected_location.line() != actual_location.line() ||
      expected_location.column() != actual_location.column()) {
    return ::testing::AssertionFailure()
           << "Unexpected error location: " << WriteJson(actual_location);
  }

  const Error *description_error = &error;
  while (StringPiece(description_error->description())
             .find(expected_description) == StringPiece::npos) {
    if (!description_error->has_cause()) {
      return ::testing::AssertionFailure()
             << "Unexpected description: " << ErrorToString(error);
    }
    description_error = &description_error->cause();
  }

  return ::testing::AssertionSuccess();
}

}  // unnamed namespace

TEST(JsonReader, ByteOrderMark) {
  EXPECT_TRUE(TestReadSuccess("\xef\xbb\xbf\x37", 7));

  EXPECT_TRUE(TestReadFailure(StringPiece("\xfe\xff\x00\x37", 4), 0,
                              "Big-Endian UTF-16"));

  EXPECT_TRUE(TestReadFailure(StringPiece("\xff\xfe\x37\x00", 4), 0,
                              "Little-Endian UTF-16"));

  EXPECT_TRUE(
      TestReadFailure(StringPiece("\x00\x00\xfe\xff\x00\x00\x00\x37", 8), 0,
                      "Big-Endian UTF-32"));

  EXPECT_TRUE(
      TestReadFailure(StringPiece("\xff\xfe\x00\x00\x37\x00\x00\x00", 8), 0,
                      "Little-Endian UTF-32"));
}

TEST(JsonReader, Null) {
  EXPECT_TRUE(TestReadSuccess("null", JsonNull()));

  EXPECT_TRUE(TestReadFailure("NULL", 0, ""));
  EXPECT_TRUE(TestReadFailure("nul", 0, ""));
  EXPECT_TRUE(TestReadFailure("nulll", 4, ""));
}

TEST(JsonReader, Bool) {
  EXPECT_TRUE(TestReadSuccess("true", true));
  EXPECT_TRUE(TestReadSuccess("false", false));

  EXPECT_TRUE(TestReadFailure("TRUE", 0, ""));
  EXPECT_TRUE(TestReadFailure("FALSE", 0, ""));
  EXPECT_TRUE(TestReadFailure("tru", 0, ""));
  EXPECT_TRUE(TestReadFailure("fals", 0, ""));
  EXPECT_TRUE(TestReadFailure("truee", 4, ""));
  EXPECT_TRUE(TestReadFailure("falsee", 5, ""));
}

TEST(JsonReader, Zero) {
  EXPECT_TRUE(TestReadSuccess("0", 0));
  EXPECT_TRUE(TestReadSuccess("0.", 0.));
  EXPECT_TRUE(TestReadSuccess("0.0", 0.));
  EXPECT_TRUE(TestReadSuccess("-0", 0));
  EXPECT_TRUE(TestReadSuccess("-0.", 0.));
  EXPECT_TRUE(TestReadSuccess("-0.0", 0.));
}

TEST(JsonReader, One) {
  EXPECT_TRUE(TestReadSuccess("1", 1));
  EXPECT_TRUE(TestReadSuccess("1.", 1.));
  EXPECT_TRUE(TestReadSuccess("1.0", 1.));
}

TEST(JsonReader, MinusOne) {
  EXPECT_TRUE(TestReadSuccess("-1", -1));
  EXPECT_TRUE(TestReadSuccess("-1.", -1.));
  EXPECT_TRUE(TestReadSuccess("-1.0", -1.));
}

TEST(JsonReader, Hundred) {
  EXPECT_TRUE(TestReadSuccess("100", 100));
  EXPECT_TRUE(TestReadSuccess("100.", 100.));
  EXPECT_TRUE(TestReadSuccess("1.0e2", 100.));
  EXPECT_TRUE(TestReadSuccess("10e1", 100.));
  EXPECT_TRUE(TestReadSuccess("1E+2", 100.));
}

TEST(JsonReader, OneTenth) {
  EXPECT_TRUE(TestReadSuccess("0.1", .1));
  EXPECT_TRUE(TestReadSuccess("0.10", .1));
  EXPECT_TRUE(TestReadSuccess("1e-1", .1));
  EXPECT_TRUE(TestReadSuccess("10E-2", .1));
  EXPECT_TRUE(TestReadSuccess("0.01e+1", .1));
  EXPECT_TRUE(TestReadSuccess("0.001e2", .1));
}

TEST(JsonReader, SignedLimits) {
  EXPECT_TRUE(TestReadSuccess("2147483647", 2147483647));
  EXPECT_TRUE(TestReadSuccess("2147483648", 2147483648ll));
  EXPECT_TRUE(TestReadSuccess("-2147483648", -2147483648ll));
  EXPECT_TRUE(TestReadSuccess("-2147483649", -2147483649ll));
  EXPECT_TRUE(TestReadSuccess("9223372036854775807", 9223372036854775807ll));

  EXPECT_TRUE(
      TestReadSuccess("-9223372036854775808", -9223372036854775807ll - 1));

  EXPECT_TRUE(TestReadSuccess("9223372036854775808", 9223372036854775808ull));

  EXPECT_TRUE(TestReadSuccess("-9223372036854775809", -9223372036854775809.));
}

TEST(JsonReader, UnsignedLimits) {
  EXPECT_TRUE(TestReadSuccess("4294967295", 4294967295ll));
  EXPECT_TRUE(TestReadSuccess("4294967296", 4294967296ll));

  EXPECT_TRUE(TestReadSuccess("18446744073709551615", 18446744073709551615ull));

  EXPECT_TRUE(TestReadSuccess("18446744073709551616", 18446744073709551616.));
}

TEST(JsonReader, InvalidNumber) {
  EXPECT_TRUE(TestReadFailure(".", 0, ""));
  EXPECT_TRUE(TestReadFailure("00", 0, ""));
  EXPECT_TRUE(TestReadFailure("-", 0, ""));
  EXPECT_TRUE(TestReadFailure("1e", 0, ""));
  EXPECT_TRUE(TestReadFailure("1e-", 0, ""));
  EXPECT_TRUE(TestReadFailure("1e+", 0, ""));
  EXPECT_TRUE(TestReadFailure("1e+.", 0, ""));
}

TEST(JsonReader, String) {
  EXPECT_TRUE(TestReadSuccess("\"\"", ""));
  EXPECT_TRUE(TestReadSuccess("\"alpha\"", "alpha"));
  EXPECT_TRUE(TestReadSuccess("\" alpha beta \"", " alpha beta "));

  EXPECT_TRUE(TestReadSuccess("\"\xe4\xbd\xa0\xe5\xa5\xbd\"",
                              "\xe4\xbd\xa0\xe5\xa5\xbd"));
}

TEST(JsonReader, StringEscape) {
  EXPECT_TRUE(TestReadSuccess("\"\\\"\"", "\""));
  EXPECT_TRUE(TestReadSuccess("\"\\\\\"", "\\"));
  EXPECT_TRUE(TestReadSuccess("\"\\/\"", "/"));
  EXPECT_TRUE(TestReadSuccess("\"\\b\"", "\b"));
  EXPECT_TRUE(TestReadSuccess("\"\\f\"", "\f"));
  EXPECT_TRUE(TestReadSuccess("\"\\n\"", "\n"));
  EXPECT_TRUE(TestReadSuccess("\"\\r\"", "\r"));
  EXPECT_TRUE(TestReadSuccess("\"\\t\"", "\t"));
}

TEST(JsonReader, StringCodePoint) {
  EXPECT_TRUE(TestReadSuccess("\"\\u002f\"", "/"));
  EXPECT_TRUE(TestReadSuccess("\"\\u002F\"", "/"));
  EXPECT_TRUE(TestReadSuccess("\"\\ud834\\udd1e\"", "\xf0\x9d\x84\x9e"));
  EXPECT_TRUE(TestReadSuccess("\"\\u002f0\"", "/0"));
  EXPECT_TRUE(TestReadSuccess("\"\\u002f0f\"", "/0f"));
}

TEST(JsonReader, InvalidString) {
  EXPECT_TRUE(TestReadFailure("\"", 1, ""));
  EXPECT_TRUE(TestReadFailure("''", 0, ""));
  EXPECT_TRUE(TestReadFailure("u\"\"", 0, ""));
  EXPECT_TRUE(TestReadFailure("\"\\u\"", 3, ""));
  EXPECT_TRUE(TestReadFailure("\"\\u1\"", 4, ""));
  EXPECT_TRUE(TestReadFailure("\"\\u12\"", 5, ""));
  EXPECT_TRUE(TestReadFailure("\"\\u123\"", 6, ""));
  EXPECT_TRUE(TestReadFailure("\"\\u123z\"", 6, ""));
  EXPECT_TRUE(TestReadFailure("\"\\ud834\"", 7, ""));
  EXPECT_TRUE(TestReadFailure("\"\\ud834\\ubfff\"", 13, ""));
}

TEST(JsonReader, Object) {
  EXPECT_TRUE(TestReadSuccess("{}", MakeJsonObject()));

  EXPECT_TRUE(
      TestReadSuccess("{\"alpha\":\"beta\"}", MakeJsonObject("alpha", "beta")));

  EXPECT_TRUE(
      TestReadSuccess("{\"alpha\":\"beta\",\"gamma\":\"delta\"}",
                      MakeJsonObject("alpha", "beta", "gamma", "delta")));

  EXPECT_TRUE(
      TestReadSuccess("{\"gamma\":\"delta\",\"alpha\":\"beta\"}",
                      MakeJsonObject("alpha", "beta", "gamma", "delta")));

  EXPECT_TRUE(TestReadSuccess("{\"alpha\":\"beta\",\"alpha\":\"delta\"}",
                              MakeJsonObject("alpha", "beta")));

  EXPECT_TRUE(TestReadSuccess("{\"alpha\":[\"beta\"]}",
                              MakeJsonObject("alpha", MakeJsonArray("beta"))));
}

TEST(JsonReader, InvalidObject) {
  EXPECT_TRUE(TestReadFailure("{", 1, ""));
  EXPECT_TRUE(TestReadFailure("{,", 1, ""));
  EXPECT_TRUE(TestReadFailure("{1:2}", 1, ""));
  EXPECT_TRUE(TestReadFailure("{\"alpha\":", 9, ""));
  EXPECT_TRUE(TestReadFailure("{\"alpha\":}", 9, ""));
  EXPECT_TRUE(TestReadFailure("{\"alpha\":1", 10, ""));
  EXPECT_TRUE(TestReadFailure("{\"alpha\":1,", 11, ""));
  EXPECT_TRUE(TestReadFailure("{\"alpha\":1 \"beta\"}", 11, ""));
}

TEST(JsonReader, Array) {
  EXPECT_TRUE(TestReadSuccess("[]", MakeJsonArray()));
  EXPECT_TRUE(TestReadSuccess("[\"alpha\"]", MakeJsonArray("alpha")));

  EXPECT_TRUE(
      TestReadSuccess("[\"alpha\",\"beta\"]", MakeJsonArray("alpha", "beta")));

  EXPECT_TRUE(
      TestReadSuccess("[\"alpha\",{\"beta\":\"gamma\"}]",
                      MakeJsonArray("alpha", MakeJsonObject("beta", "gamma"))));
}

TEST(JsonReader, InvalidArray) {
  EXPECT_TRUE(TestReadFailure("[", 1, ""));
  EXPECT_TRUE(TestReadFailure("[,", 1, ""));
  EXPECT_TRUE(TestReadFailure("[1", 2, ""));
  EXPECT_TRUE(TestReadFailure("[1,", 3, ""));
  EXPECT_TRUE(TestReadFailure("[1,2", 4, ""));
}

TEST(JsonReader, Whitespace) {
  EXPECT_TRUE(TestReadSuccess(" null", JsonNull()));
  EXPECT_TRUE(TestReadSuccess("true ", true));
  EXPECT_TRUE(TestReadSuccess(" false ", false));
  EXPECT_TRUE(TestReadSuccess("\t0\t", 0));
  EXPECT_TRUE(TestReadSuccess("\r1\r", 1));
  EXPECT_TRUE(TestReadSuccess("\n-1\n", -1));
  EXPECT_TRUE(TestReadSuccess("{ }", MakeJsonObject()));

  EXPECT_TRUE(
      TestReadSuccess(" { \"alpha\" : \"beta\" , \"gamma\" : \"delta\" }",
                      MakeJsonObject("alpha", "beta", "gamma", "delta")));

  EXPECT_TRUE(TestReadSuccess("[ ]", MakeJsonArray()));

  EXPECT_TRUE(TestReadSuccess(" [ \"alpha\" ] ", MakeJsonArray("alpha")));
  EXPECT_TRUE(TestReadSuccess(" [ \"alpha\" , \"beta\" ]",
                              MakeJsonArray("alpha", "beta")));
}

TEST(JsonReader, Comments) {
  JsonReaderConfig disallow_comments;
  disallow_comments.set_disallow_comments(true);

  EXPECT_TRUE(TestReadSuccess("/**/null", JsonNull()));
  EXPECT_TRUE(TestReadFailure("/**/null", 0, "", disallow_comments));

  EXPECT_TRUE(TestReadSuccess("true//", true));
  EXPECT_TRUE(TestReadFailure("true//", 4, "", disallow_comments));

  EXPECT_TRUE(TestReadSuccess("true//false", true));
  EXPECT_TRUE(TestReadFailure("true//\nfalse", 7, ""));

  EXPECT_TRUE(TestReadSuccess("true#", true));
  EXPECT_TRUE(TestReadFailure("true#", 4, "", disallow_comments));

  EXPECT_TRUE(TestReadSuccess("true#false", true));
  EXPECT_TRUE(TestReadFailure("true#\nfalse", 6, ""));
}

TEST(JsonReader, TrailingCommas) {
  JsonReaderConfig disallow_trailing_commas;
  disallow_trailing_commas.set_disallow_trailing_commas(true);

  EXPECT_TRUE(TestReadFailure("[,]", 1, ""));
  EXPECT_TRUE(TestReadFailure("[,]", 1, "", disallow_trailing_commas));

  EXPECT_TRUE(TestReadFailure("{,}", 1, ""));
  EXPECT_TRUE(TestReadFailure("{,}", 1, "", disallow_trailing_commas));

  EXPECT_TRUE(TestReadSuccess("[0,]", MakeJsonArray(0)));
  EXPECT_TRUE(TestReadFailure("[0,]", 3, "", disallow_trailing_commas));

  EXPECT_TRUE(TestReadFailure("{\"alpha\":\"beta\",}", 16, "",
                              disallow_trailing_commas));

  EXPECT_TRUE(TestReadSuccess("{\"alpha\":\"beta\",}",
                              MakeJsonObject("alpha", "beta")));
}

TEST(JsonReader, PropertiesAsIs) {
  JsonReaderConfig properties_as_is;
  properties_as_is.set_properties_as_is(true);

  EXPECT_TRUE(TestReadSuccess("{}", MakeJsonObject(), properties_as_is));

  EXPECT_TRUE(TestReadSuccess("{\"alpha\":\"beta\"}",
                              MakeJsonObject("alpha", "beta"),
                              properties_as_is));

  EXPECT_TRUE(TestReadSuccess("{\"alpha\":\"beta\",\"gamma\":\"delta\"}",
                              MakeJsonObject("alpha", "beta", "gamma", "delta"),
                              properties_as_is));

  EXPECT_TRUE(TestReadSuccess("{\"gamma\":\"delta\",\"alpha\":\"beta\"}",
                              MakeJsonObject("gamma", "delta", "alpha", "beta"),
                              properties_as_is));

  EXPECT_TRUE(TestReadSuccess("{\"alpha\":\"beta\",\"alpha\":\"delta\"}",
                              MakeJsonObject("alpha", "beta", "alpha", "delta"),
                              properties_as_is));
}

TEST(JsonReader, ControlCharacters) {
  JsonReaderConfig allow_control_characters;
  allow_control_characters.set_allow_control_characters(true);

  EXPECT_TRUE(TestReadFailure("\"\b\"", 1, ""));
  EXPECT_TRUE(TestReadFailure("\"\f\"", 1, ""));
  EXPECT_TRUE(TestReadFailure("\"\n\"", 1, ""));
  EXPECT_TRUE(TestReadFailure("\"\r\"", 1, ""));
  EXPECT_TRUE(TestReadFailure("\"\t\"", 1, ""));
  EXPECT_TRUE(TestReadFailure(StringPiece("\"\x00\"", 3), 1, ""));
  EXPECT_TRUE(TestReadFailure("\"\x01\"", 1, ""));

  EXPECT_TRUE(TestReadSuccess("\"\b\"", "\b", allow_control_characters));
  EXPECT_TRUE(TestReadSuccess("\"\f\"", "\f", allow_control_characters));
  EXPECT_TRUE(TestReadSuccess("\"\n\"", "\n", allow_control_characters));
  EXPECT_TRUE(TestReadSuccess("\"\r\"", "\r", allow_control_characters));
  EXPECT_TRUE(TestReadSuccess("\"\t\"", "\t", allow_control_characters));
  EXPECT_TRUE(TestReadSuccess(StringPiece("\"\x00\"", 3),
                              StringPiece("\x00", 1),
                              allow_control_characters));
  EXPECT_TRUE(TestReadSuccess("\"\x01\"", "\x01", allow_control_characters));
}

TEST(JsonReader, NaNAndInfinity) {
  JsonReaderConfig disallow_nan_and_infinity;
  disallow_nan_and_infinity.set_disallow_nan_and_infinity(true);

  EXPECT_TRUE(TestReadSuccess("NaN", JsonNaN()));
  EXPECT_TRUE(TestReadSuccess("nAn", JsonNaN()));
  EXPECT_TRUE(TestReadFailure("NaN", 0, "NaN", disallow_nan_and_infinity));
  EXPECT_TRUE(TestReadFailure("nAn", 0, "NaN", disallow_nan_and_infinity));
  EXPECT_TRUE(TestReadFailure("-nan", 0, "", disallow_nan_and_infinity));

  EXPECT_TRUE(TestReadSuccess("Inf", JsonInfinity()));
  EXPECT_TRUE(TestReadSuccess("inf", JsonInfinity()));
  EXPECT_TRUE(TestReadSuccess("Infinity", JsonInfinity()));
  EXPECT_TRUE(TestReadSuccess("infinity", JsonInfinity()));

  EXPECT_TRUE(TestReadFailure("Inf", 0, "Infinity", disallow_nan_and_infinity));

  EXPECT_TRUE(TestReadFailure("inf", 0, "Infinity", disallow_nan_and_infinity));

  EXPECT_TRUE(
      TestReadFailure("Infinity", 0, "Infinity", disallow_nan_and_infinity));

  EXPECT_TRUE(
      TestReadFailure("infinity", 0, "Infinity", disallow_nan_and_infinity));

  EXPECT_TRUE(TestReadSuccess("-Inf", JsonNegativeInfinity()));
  EXPECT_TRUE(TestReadSuccess("-inf", JsonNegativeInfinity()));
  EXPECT_TRUE(TestReadSuccess("-Infinity", JsonNegativeInfinity()));
  EXPECT_TRUE(TestReadSuccess("-infinity", JsonNegativeInfinity()));

  EXPECT_TRUE(
      TestReadFailure("-Inf", 0, "Infinity", disallow_nan_and_infinity));

  EXPECT_TRUE(
      TestReadFailure("-inf", 0, "Infinity", disallow_nan_and_infinity));

  EXPECT_TRUE(
      TestReadFailure("-Infinity", 0, "Infinity", disallow_nan_and_infinity));

  EXPECT_TRUE(
      TestReadFailure("-infinity", 0, "Infinity", disallow_nan_and_infinity));
}

}  // namespace pjcore
