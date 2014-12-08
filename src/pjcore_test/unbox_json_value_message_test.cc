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

#include "pjcore/third_party/chromium/string_piece.h"

#include "pjcore_test/test_message.pb.h"
#include "pjcore/error_util.h"
#include "pjcore/json_util.h"
#include "pjcore/json_writer.h"
#include "pjcore/logging.h"
#include "pjcore/make_json_value.h"

namespace pjcore {

namespace {

template <typename Unboxed>
::testing::AssertionResult TestUnboxSuccess(
    const JsonValue& value, const Unboxed& expected_unboxed,
    const JsonValue& expected_unknown_object_properties = MakeJsonObject()) {
  Unboxed actual_unboxed;
  Error error;
  JsonValue actual_unknown_object_properties = MakeJsonObject();
  if (!UnboxJsonValue(
          value, &actual_unboxed, &error,
          actual_unknown_object_properties.mutable_object_properties())) {
    return ::testing::AssertionFailure() << ErrorToString(error);
  }

  std::string expected_str;
  if (!expected_unboxed.SerializeToString(&expected_str)) {
    return ::testing::AssertionFailure()
           << "Failed to serialize expected unboxed value: "
           << expected_unboxed.DebugString();
  }

  std::string actual_str;
  if (!actual_unboxed.SerializeToString(&actual_str)) {
    return ::testing::AssertionFailure()
           << "Failed to serialize actual unboxed value: "
           << actual_unboxed.DebugString();
  }

  if (actual_str != expected_str) {
    return ::testing::AssertionFailure()
           << actual_unboxed.DebugString()
           << " != " << expected_unboxed.DebugString();
  }

  std::string diff_path;
  if (!AreJsonValuesEqual(actual_unknown_object_properties,
                          expected_unknown_object_properties, &diff_path)) {
    return ::testing::AssertionFailure()
           << "Unexpected unknown properties at " << diff_path << ": "
           << WriteJson(actual_unknown_object_properties)
           << " != " << WriteJson(expected_unknown_object_properties);
  }

  return ::testing::AssertionSuccess();
}

template <typename Unboxed>
::testing::AssertionResult TestUnboxFailure(const JsonValue& value,
                                            StringPiece expected_description) {
  Unboxed actual_unboxed;
  Error error;
  google::protobuf::RepeatedPtrField<JsonValue::Property>
      unknown_object_properties;

  {
    GlobalLogOverride global_log_override;

    if (UnboxJsonValue(value, &actual_unboxed, &error,
                       &unknown_object_properties)) {
      return ::testing::AssertionFailure()
             << "Unexpected success: " << actual_unboxed.DebugString();
    }
  }

  const Error* description_error = &error;
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

TEST(UnboxJsonValueMessage, OptionalInt32) {
  TestMessage test_message;
  EXPECT_TRUE(TestUnboxSuccess(MakeJsonObject("optional_int32", JsonNull()),
                               test_message));

  test_message.set_optional_int32(0);
  EXPECT_TRUE(
      TestUnboxSuccess(MakeJsonObject("optional_int32", 0), test_message));

  test_message.set_optional_int32(std::numeric_limits<int32_t>::min());
  EXPECT_TRUE(TestUnboxSuccess(
      MakeJsonObject("optional_int32", std::numeric_limits<int32_t>::min()),
      test_message));

  test_message.set_optional_int32(std::numeric_limits<int32_t>::max());
  EXPECT_TRUE(TestUnboxSuccess(
      MakeJsonObject("optional_int32", std::numeric_limits<int32_t>::max()),
      test_message));

  EXPECT_TRUE(TestUnboxFailure<TestMessage>(
      MakeJsonObject("optional_int32", "alpha"), ""));
}

TEST(UnboxJsonValueMessage, OptionalInt64) {
  TestMessage test_message;
  EXPECT_TRUE(TestUnboxSuccess(MakeJsonObject("optional_int64", JsonNull()),
                               test_message));

  test_message.set_optional_int64(0);
  EXPECT_TRUE(
      TestUnboxSuccess(MakeJsonObject("optional_int64", 0), test_message));

  test_message.set_optional_int64(std::numeric_limits<int64_t>::min());
  EXPECT_TRUE(TestUnboxSuccess(
      MakeJsonObject("optional_int64", std::numeric_limits<int64_t>::min()),
      test_message));

  test_message.set_optional_int64(std::numeric_limits<int64_t>::max());
  EXPECT_TRUE(TestUnboxSuccess(
      MakeJsonObject("optional_int64", std::numeric_limits<int64_t>::max()),
      test_message));

  EXPECT_TRUE(TestUnboxFailure<TestMessage>(
      MakeJsonObject("optional_int64", 1.23), ""));
}

TEST(UnboxJsonValueMessage, OptionalUInt32) {
  TestMessage test_message;
  EXPECT_TRUE(TestUnboxSuccess(MakeJsonObject("optional_uint32", JsonNull()),
                               test_message));

  test_message.set_optional_uint32(0);
  EXPECT_TRUE(
      TestUnboxSuccess(MakeJsonObject("optional_uint32", 0), test_message));

  test_message.set_optional_uint32(std::numeric_limits<uint32_t>::max());
  EXPECT_TRUE(TestUnboxSuccess(
      MakeJsonObject("optional_uint32", std::numeric_limits<uint32_t>::max()),
      test_message));

  EXPECT_TRUE(
      TestUnboxFailure<TestMessage>(MakeJsonObject("optional_uint32", -1), ""));
}

TEST(UnboxJsonValueMessage, OptionalUInt64) {
  TestMessage test_message;
  EXPECT_TRUE(TestUnboxSuccess(MakeJsonObject("optional_uint64", JsonNull()),
                               test_message));

  test_message.set_optional_uint64(0);
  EXPECT_TRUE(
      TestUnboxSuccess(MakeJsonObject("optional_uint64", 0), test_message));

  test_message.set_optional_uint64(std::numeric_limits<uint64_t>::max());
  EXPECT_TRUE(TestUnboxSuccess(
      MakeJsonObject("optional_uint64", std::numeric_limits<uint64_t>::max()),
      test_message));

  EXPECT_TRUE(TestUnboxFailure<TestMessage>(
      MakeJsonObject("optional_uint64", MakeJsonObject()), ""));
}

TEST(UnboxJsonValueMessage, OptionalDouble) {
  TestMessage test_message;
  EXPECT_TRUE(TestUnboxSuccess(MakeJsonObject("optional_double", JsonNull()),
                               test_message));

  test_message.set_optional_double(0);
  EXPECT_TRUE(
      TestUnboxSuccess(MakeJsonObject("optional_double", 0), test_message));

  test_message.set_optional_double(1.23e-4);
  EXPECT_TRUE(TestUnboxSuccess(MakeJsonObject("optional_double", 1.23e-4),
                               test_message));

  test_message.set_optional_double(std::numeric_limits<double>::infinity());
  EXPECT_TRUE(
      TestUnboxSuccess(MakeJsonObject("optional_double",
                                      std::numeric_limits<double>::infinity()),
                       test_message));

  test_message.set_optional_double(-std::numeric_limits<double>::infinity());
  EXPECT_TRUE(
      TestUnboxSuccess(MakeJsonObject("optional_double",
                                      -std::numeric_limits<double>::infinity()),
                       test_message));

  test_message.set_optional_double(std::numeric_limits<double>::quiet_NaN());
  EXPECT_TRUE(
      TestUnboxSuccess(MakeJsonObject("optional_double",
                                      std::numeric_limits<double>::quiet_NaN()),
                       test_message));

  EXPECT_TRUE(TestUnboxFailure<TestMessage>(
      MakeJsonObject("optional_double", MakeJsonArray()), ""));
}

TEST(UnboxJsonValueMessage, OptionalFloat) {
  TestMessage test_message;
  EXPECT_TRUE(TestUnboxSuccess(MakeJsonObject("optional_float", JsonNull()),
                               test_message));

  test_message.set_optional_float(0);
  EXPECT_TRUE(
      TestUnboxSuccess(MakeJsonObject("optional_float", 0), test_message));

  test_message.set_optional_float(1.23e-4);
  EXPECT_TRUE(TestUnboxSuccess(MakeJsonObject("optional_float", 1.23e-4f),
                               test_message));

  test_message.set_optional_float(std::numeric_limits<float>::infinity());
  EXPECT_TRUE(TestUnboxSuccess(
      MakeJsonObject("optional_float", std::numeric_limits<float>::infinity()),
      test_message));

  test_message.set_optional_float(-std::numeric_limits<float>::infinity());
  EXPECT_TRUE(TestUnboxSuccess(
      MakeJsonObject("optional_float", -std::numeric_limits<float>::infinity()),
      test_message));

  test_message.set_optional_float(std::numeric_limits<float>::quiet_NaN());
  EXPECT_TRUE(TestUnboxSuccess(
      MakeJsonObject("optional_float", std::numeric_limits<float>::quiet_NaN()),
      test_message));

  EXPECT_TRUE(TestUnboxFailure<TestMessage>(
      MakeJsonObject("optional_float", "beta"), ""));
}

TEST(UnboxJsonValueMessage, OptionalBool) {
  TestMessage test_message;
  EXPECT_TRUE(TestUnboxSuccess(MakeJsonObject("optional_bool", JsonNull()),
                               test_message));

  test_message.set_optional_bool(true);
  EXPECT_TRUE(
      TestUnboxSuccess(MakeJsonObject("optional_bool", true), test_message));

  test_message.set_optional_bool(false);
  EXPECT_TRUE(
      TestUnboxSuccess(MakeJsonObject("optional_bool", false), test_message));

  EXPECT_TRUE(TestUnboxFailure<TestMessage>(
      MakeJsonObject("optional_bool", "gamma"), ""));
}

TEST(UnboxJsonValueMessage, OptionalEnum) {
  TestMessage test_message;
  EXPECT_TRUE(TestUnboxSuccess(MakeJsonObject("optional_enum", JsonNull()),
                               test_message));

  EXPECT_TRUE(TestUnboxSuccess(MakeJsonObject("optional_enum", "unknown"),
                               test_message));

  EXPECT_TRUE(
      TestUnboxSuccess(MakeJsonObject("optional_enum", 3), test_message));

  test_message.set_optional_enum(TestMessage::TEST_ALPHA);
  EXPECT_TRUE(TestUnboxSuccess(MakeJsonObject("optional_enum", "TEST_ALPHA"),
                               test_message));

  EXPECT_TRUE(
      TestUnboxSuccess(MakeJsonObject("optional_enum", 1), test_message));

  test_message.set_optional_enum(TestMessage::TEST_BETA);
  EXPECT_TRUE(TestUnboxSuccess(MakeJsonObject("optional_enum", "TEST_BETA"),
                               test_message));

  EXPECT_TRUE(
      TestUnboxSuccess(MakeJsonObject("optional_enum", 2.0), test_message));

  EXPECT_TRUE(
      TestUnboxFailure<TestMessage>(MakeJsonObject("optional_enum", 1.23), ""));
}

TEST(UnboxJsonValueMessage, OptionalString) {
  TestMessage test_message;
  EXPECT_TRUE(TestUnboxSuccess(MakeJsonObject("optional_string", JsonNull()),
                               test_message));

  test_message.set_optional_string("");
  EXPECT_TRUE(
      TestUnboxSuccess(MakeJsonObject("optional_string", ""), test_message));

  test_message.set_optional_string("alpha");
  EXPECT_TRUE(TestUnboxSuccess(MakeJsonObject("optional_string", "alpha"),
                               test_message));

  test_message.set_optional_string("\xe4\xbd\xa0\xe5\xa5\xbd");
  EXPECT_TRUE(TestUnboxSuccess(
      MakeJsonObject("optional_string", "\xe4\xbd\xa0\xe5\xa5\xbd"),
      test_message));

  EXPECT_TRUE(TestUnboxFailure<TestMessage>(
      MakeJsonObject("optional_string", 123), ""));
}

TEST(UnboxJsonValueMessage, OptionalBytes) {
  TestMessage test_message;
  EXPECT_TRUE(TestUnboxSuccess(MakeJsonObject("optional_bytes", JsonNull()),
                               test_message));

  test_message.set_optional_bytes("");
  EXPECT_TRUE(
      TestUnboxSuccess(MakeJsonObject("optional_bytes", ""), test_message));

  test_message.set_optional_bytes("\xe4\xbd\xa0\xe5\xa5\xbd");
  EXPECT_TRUE(TestUnboxSuccess(MakeJsonObject("optional_bytes", "5L2g5aW9"),
                               test_message));

  EXPECT_TRUE(
      TestUnboxFailure<TestMessage>(MakeJsonObject("optional_bytes", 456), ""));
}

TEST(UnboxJsonValueMessage, OptionalMessage) {
  TestMessage test_message;
  EXPECT_TRUE(TestUnboxSuccess(MakeJsonObject("optional_message", JsonNull()),
                               test_message));

  EXPECT_TRUE(TestUnboxSuccess(MakeJsonObject(), test_message));

  test_message.mutable_optional_message();
  EXPECT_TRUE(TestUnboxSuccess(
      MakeJsonObject("optional_message", MakeJsonObject()), test_message));

  test_message.mutable_optional_message()->set_optional_bool(true);
  EXPECT_TRUE(TestUnboxSuccess(
      MakeJsonObject("optional_message", MakeJsonObject("optional_bool", true)),
      test_message));

  test_message.mutable_optional_message()->mutable_optional_message();
  EXPECT_TRUE(TestUnboxSuccess(
      MakeJsonObject("optional_message",
                     MakeJsonObject("optional_bool", true, "optional_message",
                                    MakeJsonObject())),
      test_message));

  test_message.mutable_optional_message()
      ->mutable_optional_message()
      ->set_optional_bool(false);
  EXPECT_TRUE(TestUnboxSuccess(
      MakeJsonObject("optional_message",
                     MakeJsonObject("optional_bool", true, "optional_message",
                                    MakeJsonObject("optional_bool", false))),
      test_message));

  EXPECT_TRUE(TestUnboxFailure<TestMessage>(MakeJsonArray(), ""));
}

TEST(UnboxJsonValueMessage, RepeatedInt32) {
  TestMessage test_message;
  EXPECT_TRUE(TestUnboxSuccess(MakeJsonObject("repeated_int32", JsonNull()),
                               test_message));

  test_message.add_repeated_int32(0);
  EXPECT_TRUE(TestUnboxSuccess(
      MakeJsonObject("repeated_int32", MakeJsonArray(0)), test_message));

  test_message.add_repeated_int32(std::numeric_limits<int32_t>::min());
  EXPECT_TRUE(TestUnboxSuccess(
      MakeJsonObject("repeated_int32",
                     MakeJsonArray(0, std::numeric_limits<int32_t>::min())),
      test_message));

  test_message.add_repeated_int32(std::numeric_limits<int32_t>::max());
  EXPECT_TRUE(TestUnboxSuccess(
      MakeJsonObject("repeated_int32",
                     MakeJsonArray(0, std::numeric_limits<int32_t>::min(),
                                   std::numeric_limits<int32_t>::max())),
      test_message));
}

TEST(UnboxJsonValueMessage, RepeatedInt64) {
  TestMessage test_message;
  EXPECT_TRUE(TestUnboxSuccess(MakeJsonObject("repeated_int64", JsonNull()),
                               test_message));

  test_message.add_repeated_int64(0);
  EXPECT_TRUE(TestUnboxSuccess(
      MakeJsonObject("repeated_int64", MakeJsonArray(0)), test_message));

  test_message.add_repeated_int64(std::numeric_limits<int64_t>::min());
  EXPECT_TRUE(TestUnboxSuccess(
      MakeJsonObject("repeated_int64",
                     MakeJsonArray(0, std::numeric_limits<int64_t>::min())),
      test_message));

  test_message.add_repeated_int64(std::numeric_limits<int64_t>::max());
  EXPECT_TRUE(TestUnboxSuccess(
      MakeJsonObject("repeated_int64",
                     MakeJsonArray(0, std::numeric_limits<int64_t>::min(),
                                   std::numeric_limits<int64_t>::max())),
      test_message));
}

TEST(UnboxJsonValueMessage, RepeatedUInt32) {
  TestMessage test_message;
  EXPECT_TRUE(TestUnboxSuccess(MakeJsonObject("repeated_uint32", JsonNull()),
                               test_message));

  test_message.add_repeated_uint32(0);
  EXPECT_TRUE(TestUnboxSuccess(
      MakeJsonObject("repeated_uint32", MakeJsonArray(0)), test_message));

  test_message.add_repeated_uint32(std::numeric_limits<uint32_t>::max());
  EXPECT_TRUE(TestUnboxSuccess(
      MakeJsonObject("repeated_uint32",
                     MakeJsonArray(0, std::numeric_limits<uint32_t>::max())),
      test_message));
}

TEST(UnboxJsonValueMessage, RepeatedUInt64) {
  TestMessage test_message;
  EXPECT_TRUE(TestUnboxSuccess(MakeJsonObject("repeated_uint64", JsonNull()),
                               test_message));

  test_message.add_repeated_uint64(0);
  EXPECT_TRUE(TestUnboxSuccess(
      MakeJsonObject("repeated_uint64", MakeJsonArray(0)), test_message));

  test_message.add_repeated_uint64(std::numeric_limits<uint64_t>::max());
  EXPECT_TRUE(TestUnboxSuccess(
      MakeJsonObject("repeated_uint64",
                     MakeJsonArray(0, std::numeric_limits<uint64_t>::max())),
      test_message));
}

TEST(UnboxJsonValueMessage, RepeatedDouble) {
  TestMessage test_message;
  EXPECT_TRUE(TestUnboxSuccess(MakeJsonObject("repeated_double", JsonNull()),
                               test_message));

  test_message.add_repeated_double(0);
  EXPECT_TRUE(TestUnboxSuccess(
      MakeJsonObject("repeated_double", MakeJsonArray(0)), test_message));

  test_message.add_repeated_double(1.23e-4);
  EXPECT_TRUE(TestUnboxSuccess(
      MakeJsonObject("repeated_double", MakeJsonArray(0, 1.23e-4)),
      test_message));

  test_message.add_repeated_double(std::numeric_limits<double>::infinity());
  EXPECT_TRUE(TestUnboxSuccess(
      MakeJsonObject(
          "repeated_double",
          MakeJsonArray(0, 1.23e-4, std::numeric_limits<double>::infinity())),
      test_message));

  test_message.add_repeated_double(-std::numeric_limits<double>::infinity());
  EXPECT_TRUE(TestUnboxSuccess(
      MakeJsonObject(
          "repeated_double",
          MakeJsonArray(0, 1.23e-4, std::numeric_limits<double>::infinity(),
                        -std::numeric_limits<double>::infinity())),
      test_message));

  test_message.add_repeated_double(std::numeric_limits<double>::quiet_NaN());
  EXPECT_TRUE(TestUnboxSuccess(
      MakeJsonObject(
          "repeated_double",
          MakeJsonArray(0, 1.23e-4, std::numeric_limits<double>::infinity(),
                        -std::numeric_limits<double>::infinity(),
                        std::numeric_limits<double>::quiet_NaN())),
      test_message));
}

TEST(UnboxJsonValueMessage, RepeatedFloat) {
  TestMessage test_message;
  EXPECT_TRUE(TestUnboxSuccess(MakeJsonObject("repeated_float", JsonNull()),
                               test_message));

  test_message.add_repeated_float(0);
  EXPECT_TRUE(TestUnboxSuccess(
      MakeJsonObject("repeated_float", MakeJsonArray(0)), test_message));

  test_message.add_repeated_float(1.23e-4);
  EXPECT_TRUE(TestUnboxSuccess(
      MakeJsonObject("repeated_float", MakeJsonArray(0, 1.23e-4f)),
      test_message));

  test_message.add_repeated_float(std::numeric_limits<float>::infinity());
  EXPECT_TRUE(TestUnboxSuccess(
      MakeJsonObject(
          "repeated_float",
          MakeJsonArray(0, 1.23e-4f, std::numeric_limits<float>::infinity())),
      test_message));

  test_message.add_repeated_float(-std::numeric_limits<float>::infinity());
  EXPECT_TRUE(TestUnboxSuccess(
      MakeJsonObject(
          "repeated_float",
          MakeJsonArray(0, 1.23e-4f, std::numeric_limits<float>::infinity(),
                        -std::numeric_limits<float>::infinity())),
      test_message));

  test_message.add_repeated_float(std::numeric_limits<float>::quiet_NaN());
  EXPECT_TRUE(TestUnboxSuccess(
      MakeJsonObject(
          "repeated_float",
          MakeJsonArray(0, 1.23e-4f, std::numeric_limits<float>::infinity(),
                        -std::numeric_limits<float>::infinity(),
                        std::numeric_limits<float>::quiet_NaN())),
      test_message));
}

TEST(UnboxJsonValueMessage, RepeatedBool) {
  TestMessage test_message;
  EXPECT_TRUE(TestUnboxSuccess(MakeJsonObject("repeated_bool", JsonNull()),
                               test_message));

  test_message.add_repeated_bool(true);
  EXPECT_TRUE(TestUnboxSuccess(
      MakeJsonObject("repeated_bool", MakeJsonArray(true)), test_message));

  test_message.add_repeated_bool(false);
  EXPECT_TRUE(TestUnboxSuccess(
      MakeJsonObject("repeated_bool", MakeJsonArray(true, false)),
      test_message));
}

TEST(UnboxJsonValueMessage, RepeatedEnum) {
  TestMessage test_message;
  EXPECT_TRUE(TestUnboxSuccess(MakeJsonObject("repeated_enum", JsonNull()),
                               test_message));

  test_message.add_repeated_enum(TestMessage::TEST_ALPHA);
  EXPECT_TRUE(TestUnboxSuccess(
      MakeJsonObject("repeated_enum", MakeJsonArray("TEST_ALPHA")),
      test_message));

  test_message.add_repeated_enum(TestMessage::TEST_BETA);
  EXPECT_TRUE(TestUnboxSuccess(
      MakeJsonObject("repeated_enum", MakeJsonArray("TEST_ALPHA", "TEST_BETA")),
      test_message));
}

TEST(UnboxJsonValueMessage, RepeatedString) {
  TestMessage test_message;
  EXPECT_TRUE(TestUnboxSuccess(MakeJsonObject("repeated_string", JsonNull()),
                               test_message));

  test_message.add_repeated_string("");
  EXPECT_TRUE(TestUnboxSuccess(
      MakeJsonObject("repeated_string", MakeJsonArray("")), test_message));

  test_message.add_repeated_string("alpha");
  EXPECT_TRUE(TestUnboxSuccess(
      MakeJsonObject("repeated_string", MakeJsonArray("", "alpha")),
      test_message));

  test_message.add_repeated_string("\xe4\xbd\xa0\xe5\xa5\xbd");
  EXPECT_TRUE(TestUnboxSuccess(
      MakeJsonObject("repeated_string",
                     MakeJsonArray("", "alpha", "\xe4\xbd\xa0\xe5\xa5\xbd")),
      test_message));
}

TEST(UnboxJsonValueMessage, RepeatedBytes) {
  TestMessage test_message;
  EXPECT_TRUE(TestUnboxSuccess(MakeJsonObject("repeated_bytes", JsonNull()),
                               test_message));

  test_message.add_repeated_bytes("");
  EXPECT_TRUE(TestUnboxSuccess(
      MakeJsonObject("repeated_bytes", MakeJsonArray("")), test_message));

  test_message.add_repeated_bytes("\xe4\xbd\xa0\xe5\xa5\xbd");
  EXPECT_TRUE(TestUnboxSuccess(
      MakeJsonObject("repeated_bytes", MakeJsonArray("", "5L2g5aW9")),
      test_message));
}

TEST(UnboxJsonValueMessage, RepeatedMessage) {
  TestMessage test_message;
  EXPECT_TRUE(TestUnboxSuccess(MakeJsonObject("repeated_message", JsonNull()),
                               test_message));

  EXPECT_TRUE(TestUnboxSuccess(MakeJsonObject(), test_message));

  test_message.add_repeated_message();
  EXPECT_TRUE(TestUnboxSuccess(
      MakeJsonObject("repeated_message", MakeJsonArray(MakeJsonObject())),
      test_message));

  test_message.mutable_repeated_message(0)->set_optional_bool(true);
  EXPECT_TRUE(TestUnboxSuccess(
      MakeJsonObject("repeated_message",
                     MakeJsonArray(MakeJsonObject("optional_bool", true))),
      test_message));

  test_message.mutable_repeated_message(0)->add_repeated_message();
  EXPECT_TRUE(TestUnboxSuccess(
      MakeJsonObject("repeated_message",
                     MakeJsonArray(MakeJsonObject(
                         "optional_bool", true, "repeated_message",
                         MakeJsonArray(MakeJsonObject())))),
      test_message));

  test_message.mutable_repeated_message(0)
      ->mutable_repeated_message(0)
      ->set_optional_bool(false);
  EXPECT_TRUE(TestUnboxSuccess(
      MakeJsonObject(
          "repeated_message",
          MakeJsonArray(MakeJsonObject(
              "optional_bool", true, "repeated_message",
              MakeJsonArray(MakeJsonObject("optional_bool", false))))),
      test_message));
}

TEST(UnboxJsonValueMessage, JsonValue) {
  EXPECT_TRUE(TestUnboxSuccess<JsonValue>(JsonNull(), JsonNull()));

  EXPECT_TRUE(
      TestUnboxSuccess<JsonValue>(MakeJsonValue(true), MakeJsonValue(true)));

  EXPECT_TRUE(
      TestUnboxSuccess<JsonValue>(MakeJsonValue(-123), MakeJsonValue(-123)));

  EXPECT_TRUE(TestUnboxSuccess<JsonValue>(MakeJsonValue(456ull),
                                          MakeJsonValue(456ull)));

  EXPECT_TRUE(TestUnboxSuccess<JsonValue>(MakeJsonValue(-12.34e-5),
                                          MakeJsonValue(-12.34e-5)));

  EXPECT_TRUE(TestUnboxSuccess<JsonValue>(MakeJsonValue("alpha"),
                                          MakeJsonValue("alpha")));

  EXPECT_TRUE(TestUnboxSuccess<JsonValue>(
      MakeJsonObject("delta", 789, "epsilon", 135ull),
      MakeJsonObject("delta", 789, "epsilon", 135ull)));

  EXPECT_TRUE(TestUnboxSuccess<JsonValue>(MakeJsonArray(24.68, true),
                                          MakeJsonArray(24.68, true)));
}

TEST(UnboxJsonValueMessage, WithoutObjectProperties) {
  TestMessage test_message;
  EXPECT_TRUE(TestUnboxSuccess(MakeJsonObject("alpha", JsonNull()),
                               test_message,
                               MakeJsonObject("alpha", JsonNull())));

  EXPECT_TRUE(TestUnboxSuccess(
      MakeJsonObject("alpha", JsonNull(), "beta", true), test_message,
      MakeJsonObject("alpha", JsonNull(), "beta", true)));

  EXPECT_TRUE(TestUnboxSuccess(
      MakeJsonObject("alpha", JsonNull(), "beta", true, "alpha", -123),
      test_message,
      MakeJsonObject("alpha", JsonNull(), "beta", true, "alpha", -123)));
}

TEST(UnboxJsonValueMessage, WithObjectProperties) {
  TestMessageWithObjectProperties test_message;
  *test_message.add_object_properties() = MakeJsonProperty("alpha", JsonNull());
  EXPECT_TRUE(
      TestUnboxSuccess(MakeJsonObject("alpha", JsonNull()), test_message));

  *test_message.add_object_properties() = MakeJsonProperty("beta", true);
  EXPECT_TRUE(TestUnboxSuccess(
      MakeJsonObject("alpha", JsonNull(), "beta", true), test_message));

  *test_message.add_object_properties() = MakeJsonProperty("alpha", -123);
  EXPECT_TRUE(TestUnboxSuccess(
      MakeJsonObject("alpha", JsonNull(), "beta", true, "alpha", -123),
      test_message));
}

TEST(UnboxJsonValueMessage, TypeFailure) {
  EXPECT_TRUE(TestUnboxFailure<TestMessage>(JsonValue::default_instance(), ""));
  EXPECT_TRUE(TestUnboxFailure<TestMessage>(JsonNull(), ""));
  EXPECT_TRUE(TestUnboxFailure<TestMessage>(MakeJsonValue(true), ""));
  EXPECT_TRUE(TestUnboxFailure<TestMessage>(MakeJsonValue(-123), ""));
  EXPECT_TRUE(TestUnboxFailure<TestMessage>(MakeJsonValue(456ull), ""));
  EXPECT_TRUE(TestUnboxFailure<TestMessage>(MakeJsonValue(7.89), ""));
  EXPECT_TRUE(TestUnboxFailure<TestMessage>(MakeJsonValue("alpha"), ""));
  EXPECT_TRUE(TestUnboxFailure<TestMessage>(MakeJsonArray(1, 2, 3), ""));
}

TEST(UnboxJsonValueMessage, StringMap) {
  TestMessageWithStringMap test_message;
  TestMessageWithStringMap::Entry* entry;

  EXPECT_TRUE(TestUnboxSuccess(MakeJsonObject(), test_message));
  EXPECT_TRUE(TestUnboxSuccess(MakeJsonObject("entries", MakeJsonObject()),
                               test_message));

  entry = test_message.add_entries();
  entry->set_name("alpha");
  entry->set_data(3);

  EXPECT_TRUE(TestUnboxSuccess(
      MakeJsonObject("entries",
                     MakeJsonObject("alpha", MakeJsonObject("data", 3))),
      test_message));

  entry = test_message.add_entries();
  entry->set_name("beta");
  entry->set_data(5);

  EXPECT_TRUE(TestUnboxSuccess(
      MakeJsonObject("entries",
                     MakeJsonObject("alpha", MakeJsonObject("data", 3), "beta",
                                    MakeJsonObject("data", 5))),
      test_message));
}

TEST(UnboxJsonValueMessage, IntMap) {
  TestMessageWithIntMap test_message;
  TestMessageWithIntMap::Entry* entry;

  EXPECT_TRUE(TestUnboxSuccess(MakeJsonObject(), test_message));
  EXPECT_TRUE(TestUnboxSuccess(MakeJsonObject("entries", MakeJsonObject()),
                               test_message));

  entry = test_message.add_entries();
  entry->set_name(3);
  entry->set_data("alpha");

  EXPECT_TRUE(TestUnboxSuccess(
      MakeJsonObject("entries",
                     MakeJsonObject("3", MakeJsonObject("data", "alpha"))),
      test_message));

  entry = test_message.add_entries();
  entry->set_name(5);
  entry->set_data("beta");

  EXPECT_TRUE(TestUnboxSuccess(
      MakeJsonObject("entries",
                     MakeJsonObject("3", MakeJsonObject("data", "alpha"), "5",
                                    MakeJsonObject("data", "beta"))),
      test_message));
}

}  // namespace pjcore
