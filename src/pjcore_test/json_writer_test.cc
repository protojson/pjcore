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

#include "pjcore/json_writer.h"

#include <gtest/gtest.h>

#include "pjcore/error_util.h"
#include "pjcore/json_reader.h"
#include "pjcore/json_util.h"

namespace pjcore {

namespace {

template <typename Value>
::testing::AssertionResult TestReadBack(
    const Value& value,
    const JsonWriterConfig& config = JsonWriterConfig::default_instance()) {
  std::string str = WriteJson(value, config);

  JsonValue another;
  Error error;
  if (!ReadJson(str, &another, &error)) {
    return ::testing::AssertionFailure() << "Failed to parse with "
                                         << ErrorToString(error) << ": " << str;
  }

  std::string diff_path;
  if (!AreJsonValuesEqual(value, another, &diff_path)) {
    return ::testing::AssertionFailure() << "Difference at " << diff_path
                                         << ": " << WriteJson(another);
  }

  return ::testing::AssertionSuccess();
}

}  // unnamed namespace

TEST(JsonWriter, Null) { EXPECT_EQ("null", WriteJson(JsonNull())); }

TEST(JsonWriter, Bool) {
  EXPECT_EQ("true", WriteJson(true));
  EXPECT_EQ("false", WriteJson(false));
}

TEST(JsonWriter, Zero) { EXPECT_EQ("0", WriteJson(0)); }

TEST(JsonWriter, One) { EXPECT_EQ("1", WriteJson(1)); }

TEST(JsonWriter, MinusOne) { EXPECT_EQ("-1", WriteJson(-1)); }

TEST(JsonWriter, Hundred) { EXPECT_EQ("100", WriteJson(100)); }

TEST(JsonWriter, OneTenth) { EXPECT_EQ("0.1", WriteJson(.1)); }

TEST(JsonWriter, SignedLimits) {
  EXPECT_EQ("2147483647", WriteJson(2147483647));
  EXPECT_EQ("2147483648", WriteJson(2147483648ll));
  EXPECT_EQ("-2147483648", WriteJson(-2147483648ll));
  EXPECT_EQ("-2147483649", WriteJson(-2147483649ll));
  EXPECT_EQ("9223372036854775807", WriteJson(9223372036854775807ll));
  EXPECT_EQ("-9223372036854775808", WriteJson(-9223372036854775807ll - 1));
  EXPECT_EQ("9223372036854775808", WriteJson(9223372036854775808ull));

  EXPECT_TRUE(TestReadBack(-9223372036854775809.));
}

TEST(JsonWriter, UnsignedLimits) {
  EXPECT_EQ("4294967295", WriteJson(4294967295ll));
  EXPECT_EQ("4294967296", WriteJson(4294967296ll));
  EXPECT_EQ("18446744073709551615", WriteJson(18446744073709551615ull));

  EXPECT_TRUE(TestReadBack(18446744073709551616.));
}

TEST(JsonWriter, String) {
  EXPECT_EQ("\"\"", WriteJson(""));
  EXPECT_EQ("\"alpha\"", WriteJson("alpha"));
  EXPECT_EQ("\" alpha beta \"", WriteJson(" alpha beta "));

  EXPECT_EQ("\"\xe4\xbd\xa0\xe5\xa5\xbd\"",
            WriteJson("\xe4\xbd\xa0\xe5\xa5\xbd"));
}

TEST(JsonWriter, StringEscape) {
  EXPECT_EQ("\"\\\"\"", WriteJson("\""));
  EXPECT_EQ("\"\\\\\"", WriteJson("\\"));
  EXPECT_EQ("\"\\/\"", WriteJson("/"));
  EXPECT_EQ("\"\\b\"", WriteJson("\b"));
  EXPECT_EQ("\"\\f\"", WriteJson("\f"));
  EXPECT_EQ("\"\\n\"", WriteJson("\n"));
  EXPECT_EQ("\"\\r\"", WriteJson("\r"));
  EXPECT_EQ("\"\\t\"", WriteJson("\t"));
}

TEST(JsonWriter, Object) {
  EXPECT_EQ("{}", WriteJson(MakeJsonObject()));

  EXPECT_EQ("{\"alpha\":\"beta\"}", WriteJson(MakeJsonObject("alpha", "beta")));

  EXPECT_EQ("{\"alpha\":\"beta\",\"gamma\":\"delta\"}",
            WriteJson(MakeJsonObject("alpha", "beta", "gamma", "delta")));

  EXPECT_EQ("{\"alpha\":\"beta\",\"gamma\":[\"delta\"]}",
            WriteJson(MakeJsonObject("alpha", "beta", "gamma",
                                     MakeJsonArray("delta"))));
}

TEST(JsonWriter, Array) {
  EXPECT_EQ("[]", WriteJson(MakeJsonArray()));
  EXPECT_EQ("[\"alpha\"]", WriteJson(MakeJsonArray("alpha")));

  EXPECT_EQ("[\"alpha\",\"beta\"]", WriteJson(MakeJsonArray("alpha", "beta")));

  EXPECT_EQ("[\"alpha\",{\"beta\":\"gamma\"}]",
            WriteJson(MakeJsonArray("alpha", MakeJsonObject("beta", "gamma"))));
}

TEST(JsonWriter, ByteOrderMark) {
  JsonWriterConfig include_byte_order_mark;
  include_byte_order_mark.set_include_byte_order_mark(true);

  EXPECT_EQ("\xef\xbb\xbf\x37", WriteJson(7, include_byte_order_mark));
}

TEST(JsonWriter, EscapeUnicode) {
  JsonWriterConfig escape_unicode;
  escape_unicode.set_escape_unicode(true);

  EXPECT_EQ("\"\\ud834\\udd1e\"",
            WriteJson("\xf0\x9d\x84\x9e", escape_unicode));

  EXPECT_TRUE(TestReadBack("\xf0\x9d\x84\x9e", escape_unicode));
}

TEST(JsonWriter, Space) {
  JsonWriterConfig space;
  space.set_space(true);

  EXPECT_EQ("{}", WriteJson(MakeJsonObject(), space));

  EXPECT_EQ("{\"alpha\": \"beta\"}",
            WriteJson(MakeJsonObject("alpha", "beta"), space));

  EXPECT_EQ(
      "{\"alpha\": \"beta\", \"gamma\": \"delta\"}",
      WriteJson(MakeJsonObject("alpha", "beta", "gamma", "delta"), space));

  EXPECT_EQ("{\"alpha\": \"beta\", \"gamma\": [\"delta\"]}",
            WriteJson(MakeJsonObject("alpha", "beta", "gamma",
                                     MakeJsonArray("delta")),
                      space));

  EXPECT_EQ("[]", WriteJson(MakeJsonArray(), space));
  EXPECT_EQ("[\"alpha\"]", WriteJson(MakeJsonArray("alpha"), space));

  EXPECT_EQ("[\"alpha\", \"beta\"]",
            WriteJson(MakeJsonArray("alpha", "beta"), space));

  EXPECT_EQ("[\"alpha\", {\"beta\": \"gamma\"}]",
            WriteJson(MakeJsonArray("alpha", MakeJsonObject("beta", "gamma")),
                      space));
}

TEST(JsonWriter, Indent) {
  JsonWriterConfig indent;
  indent.set_indent(1);

  EXPECT_EQ("{}", WriteJson(MakeJsonObject(), indent));

  EXPECT_EQ("{\n \"alpha\":\"beta\"\n}",
            WriteJson(MakeJsonObject("alpha", "beta"), indent));

  EXPECT_EQ(
      "{\n \"alpha\":\"beta\",\n \"gamma\":\"delta\"\n}",
      WriteJson(MakeJsonObject("alpha", "beta", "gamma", "delta"), indent));

  EXPECT_EQ("{\n \"alpha\":\"beta\",\n \"gamma\":[\n  \"delta\"\n ]\n}",
            WriteJson(MakeJsonObject("alpha", "beta", "gamma",
                                     MakeJsonArray("delta")),
                      indent));

  EXPECT_EQ("[]", WriteJson(MakeJsonArray(), indent));
  EXPECT_EQ("[\n \"alpha\"\n]", WriteJson(MakeJsonArray("alpha"), indent));

  EXPECT_EQ("[\n \"alpha\",\n \"beta\"\n]",
            WriteJson(MakeJsonArray("alpha", "beta"), indent));

  EXPECT_EQ("[\n \"alpha\",\n {\n  \"beta\":\"gamma\"\n }\n]",
            WriteJson(MakeJsonArray("alpha", MakeJsonObject("beta", "gamma")),
                      indent));
}

TEST(JsonWriter, SpaceAndIndent) {
  JsonWriterConfig space_and_indent;
  space_and_indent.set_space(true);
  space_and_indent.set_indent(1);

  EXPECT_EQ("{}", WriteJson(MakeJsonObject(), space_and_indent));

  EXPECT_EQ("{\n \"alpha\": \"beta\"\n}",
            WriteJson(MakeJsonObject("alpha", "beta"), space_and_indent));

  EXPECT_EQ("{\n \"alpha\": \"beta\",\n \"gamma\": \"delta\"\n}",
            WriteJson(MakeJsonObject("alpha", "beta", "gamma", "delta"),
                      space_and_indent));

  EXPECT_EQ("{\n \"alpha\": \"beta\",\n \"gamma\": [\n  \"delta\"\n ]\n}",
            WriteJson(MakeJsonObject("alpha", "beta", "gamma",
                                     MakeJsonArray("delta")),
                      space_and_indent));

  EXPECT_EQ("[]", WriteJson(MakeJsonArray(), space_and_indent));
  EXPECT_EQ("[\n \"alpha\"\n]",
            WriteJson(MakeJsonArray("alpha"), space_and_indent));

  EXPECT_EQ("[\n \"alpha\",\n \"beta\"\n]",
            WriteJson(MakeJsonArray("alpha", "beta"), space_and_indent));

  EXPECT_EQ("[\n \"alpha\",\n {\n  \"beta\": \"gamma\"\n }\n]",
            WriteJson(MakeJsonArray("alpha", MakeJsonObject("beta", "gamma")),
                      space_and_indent));
}

TEST(JsonWriter, NaNAndInfinity) {
  JsonWriterConfig null_for_nan_and_infinity;
  null_for_nan_and_infinity.set_null_for_nan_and_infinity(true);

  EXPECT_EQ("NaN", WriteJson(JsonNaN()));
  EXPECT_EQ("null", WriteJson(JsonNaN(), null_for_nan_and_infinity));

  EXPECT_EQ("Infinity", WriteJson(JsonInfinity()));
  EXPECT_EQ("null", WriteJson(JsonInfinity(), null_for_nan_and_infinity));

  EXPECT_EQ("-Infinity", WriteJson(JsonNegativeInfinity()));

  EXPECT_EQ("null",
            WriteJson(JsonNegativeInfinity(), null_for_nan_and_infinity));
}

}  // namespace pjcore
