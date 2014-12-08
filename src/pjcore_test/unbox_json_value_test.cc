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

#include <gtest/gtest.h>

#include "pjcore/third_party/gtest/are_almost_equal.h"

#include "pjcore/error_util.h"
#include "pjcore/logging.h"
#include "pjcore/make_json_value.h"

namespace pjcore {

namespace {

template <typename Unboxed>
::testing::AssertionResult TestUnboxSuccess(const JsonValue &boxed,
                                            Unboxed expected_unboxed) {
  Unboxed actual_unboxed;
  Error error;

  if (!UnboxJsonValue(boxed, &actual_unboxed, &error)) {
    return ::testing::AssertionFailure() << ErrorToString(error);
  }

  if (!AreAlmostEqual(expected_unboxed, actual_unboxed)) {
    return ::testing::AssertionFailure() << actual_unboxed;
  }

  return ::testing::AssertionSuccess();
}

template <typename Unboxed>
::testing::AssertionResult TestUnboxFailure(const JsonValue &boxed,
                                            StringPiece expected_description) {
  Unboxed actual_unboxed;
  Error error;

  {
    GlobalLogOverride global_log_override;

    if (UnboxJsonValue(boxed, &actual_unboxed, &error)) {
      return ::testing::AssertionFailure()
             << "Unexpected success: " << actual_unboxed;
    }
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

TEST(UnboxJsonValue, BoolValue) {
  EXPECT_TRUE(TestUnboxSuccess(MakeJsonValue(true), true));
  EXPECT_TRUE(TestUnboxSuccess(MakeJsonValue(1), true));
  EXPECT_TRUE(TestUnboxSuccess(MakeJsonValue(1ull), true));
  EXPECT_TRUE(TestUnboxSuccess(MakeJsonValue(1.0), true));
  EXPECT_TRUE(TestUnboxSuccess(MakeJsonValue("true"), true));

  EXPECT_TRUE(TestUnboxSuccess(MakeJsonValue(false), false));
  EXPECT_TRUE(TestUnboxSuccess(MakeJsonValue(0), false));
  EXPECT_TRUE(TestUnboxSuccess(MakeJsonValue(0ull), false));
  EXPECT_TRUE(TestUnboxSuccess(MakeJsonValue(0.0), false));
  EXPECT_TRUE(TestUnboxSuccess(MakeJsonValue("false"), false));

  EXPECT_TRUE(TestUnboxFailure<bool>(JsonNull(), ""));
  EXPECT_TRUE(TestUnboxFailure<bool>(MakeJsonValue("alpha"), ""));
  EXPECT_TRUE(TestUnboxFailure<bool>(MakeJsonObject(), ""));
  EXPECT_TRUE(TestUnboxFailure<bool>(MakeJsonArray(), ""));
}

TEST(UnboxJsonValue, SignedValue) {
  EXPECT_TRUE(TestUnboxSuccess(MakeJsonValue(true), 1));
  EXPECT_TRUE(TestUnboxSuccess(MakeJsonValue(1), 1));
  EXPECT_TRUE(TestUnboxSuccess(MakeJsonValue(1ull), 1));
  EXPECT_TRUE(TestUnboxSuccess(MakeJsonValue(1.0), 1));
  EXPECT_TRUE(TestUnboxSuccess(MakeJsonValue("1"), 1));

  EXPECT_TRUE(
      TestUnboxSuccess<int32_t>(MakeJsonValue(2147483647ll), 2147483647));
  EXPECT_TRUE(
      TestUnboxSuccess<int32_t>(MakeJsonValue("2147483647"), 2147483647));
  EXPECT_TRUE(
      TestUnboxSuccess<int32_t>(MakeJsonValue(-2147483648ll), -2147483648));
  EXPECT_TRUE(
      TestUnboxSuccess<int32_t>(MakeJsonValue("-2147483648"), -2147483648));

  EXPECT_TRUE(
      TestUnboxFailure<int32_t>(MakeJsonValue(2147483648ll), "overflow"));
  EXPECT_TRUE(
      TestUnboxFailure<int32_t>(MakeJsonValue("2147483648"), "overflow"));
  EXPECT_TRUE(
      TestUnboxFailure<int32_t>(MakeJsonValue(-2147483649ll), "underflow"));
  EXPECT_TRUE(
      TestUnboxFailure<int32_t>(MakeJsonValue("-2147483649"), "underflow"));

  EXPECT_TRUE(TestUnboxSuccess(MakeJsonValue(9223372036854775807ll),
                               9223372036854775807ll));
  EXPECT_TRUE(TestUnboxSuccess(MakeJsonValue(9223372036854775807ull),
                               9223372036854775807ll));
  EXPECT_TRUE(TestUnboxSuccess(MakeJsonValue("9223372036854775807"),
                               9223372036854775807ll));
  EXPECT_TRUE(TestUnboxSuccess(MakeJsonValue(-9223372036854775807ll - 1),
                               -9223372036854775807ll - 1));
  EXPECT_TRUE(TestUnboxSuccess(MakeJsonValue("-9223372036854775808"),
                               -9223372036854775807ll - 1));

  EXPECT_TRUE(
      TestUnboxFailure<int64_t>(MakeJsonValue(9223372036854775808ull), ""));
  EXPECT_TRUE(
      TestUnboxFailure<int64_t>(MakeJsonValue("9223372036854775808"), ""));

  EXPECT_TRUE(
      TestUnboxFailure<int64_t>(MakeJsonValue("-9223372036854775809"), ""));

  EXPECT_TRUE(TestUnboxFailure<int64_t>(JsonNull(), ""));
  EXPECT_TRUE(TestUnboxFailure<int64_t>(MakeJsonValue("alpha"), ""));
}

TEST(UnboxJsonValue, UnsignedValue) {
  EXPECT_TRUE(TestUnboxSuccess(MakeJsonValue(true), 1u));
  EXPECT_TRUE(TestUnboxSuccess(MakeJsonValue(1ull), 1u));
  EXPECT_TRUE(TestUnboxSuccess(MakeJsonValue(1), 1u));
  EXPECT_TRUE(TestUnboxSuccess(MakeJsonValue(1.0), 1u));
  EXPECT_TRUE(TestUnboxSuccess(MakeJsonValue("1"), 1u));

  EXPECT_TRUE(
      TestUnboxSuccess<uint32_t>(MakeJsonValue(4294967295ll), 4294967295u));
  EXPECT_TRUE(
      TestUnboxSuccess<uint32_t>(MakeJsonValue(4294967295ull), 4294967295u));
  EXPECT_TRUE(
      TestUnboxSuccess<uint32_t>(MakeJsonValue("4294967295"), 4294967295u));
  EXPECT_TRUE(
      TestUnboxFailure<uint32_t>(MakeJsonValue(4294967296ll), "overflow"));
  EXPECT_TRUE(
      TestUnboxFailure<uint32_t>(MakeJsonValue(4294967296ull), "overflow"));
  EXPECT_TRUE(
      TestUnboxFailure<uint32_t>(MakeJsonValue("4294967296"), "overflow"));

  EXPECT_TRUE(TestUnboxSuccess(MakeJsonValue(18446744073709551615ull),
                               18446744073709551615ull));
  EXPECT_TRUE(TestUnboxSuccess(MakeJsonValue("18446744073709551615"),
                               18446744073709551615ull));

  EXPECT_TRUE(
      TestUnboxFailure<uint64_t>(MakeJsonValue("18446744073709551616"), ""));

  EXPECT_TRUE(TestUnboxFailure<uint64_t>(JsonNull(), ""));
  EXPECT_TRUE(TestUnboxFailure<uint64_t>(MakeJsonValue("alpha"), ""));
}

TEST(UnboxJsonValue, DoubleValue) {
  EXPECT_TRUE(TestUnboxSuccess(MakeJsonValue(true), 1.0));
  EXPECT_TRUE(TestUnboxSuccess(MakeJsonValue(1ull), 1.0));
  EXPECT_TRUE(TestUnboxSuccess(MakeJsonValue(1), 1.0));
  EXPECT_TRUE(TestUnboxSuccess(MakeJsonValue(1.0), 1.0));
  EXPECT_TRUE(TestUnboxSuccess(MakeJsonValue("1"), 1.0));

  EXPECT_TRUE(TestUnboxFailure<double>(JsonNull(), ""));
  EXPECT_TRUE(TestUnboxFailure<double>(MakeJsonValue("alpha"), ""));
}

TEST(UnboxJsonValue, StringValue) {
  EXPECT_TRUE(TestUnboxSuccess(MakeJsonValue("alpha"), std::string("alpha")));

  EXPECT_TRUE(TestUnboxFailure<std::string>(JsonNull(), ""));
  EXPECT_TRUE(TestUnboxFailure<std::string>(MakeJsonValue(true), ""));
  EXPECT_TRUE(TestUnboxFailure<std::string>(MakeJsonValue(1ull), ""));
  EXPECT_TRUE(TestUnboxFailure<std::string>(MakeJsonValue(1), ""));
  EXPECT_TRUE(TestUnboxFailure<std::string>(MakeJsonValue(1.0), ""));
}

}  // namespace pjcore
