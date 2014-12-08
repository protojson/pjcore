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
#include "pjcore_test/test_message.pb.h"

namespace pjcore {

TEST(MakeJsonValueMessage, OptionalInt32) {
  TestMessage test_message;
  test_message.set_optional_int32(0);
  EXPECT_TRUE(
      AreJsonValuesEqual(test_message, MakeJsonObject("optional_int32", 0)))
      << test_message.DebugString();

  test_message.set_optional_int32(std::numeric_limits<int32_t>::min());
  EXPECT_TRUE(AreJsonValuesEqual(
      test_message,
      MakeJsonObject("optional_int32", std::numeric_limits<int32_t>::min())))
      << test_message.DebugString();

  test_message.set_optional_int32(std::numeric_limits<int32_t>::max());
  EXPECT_TRUE(AreJsonValuesEqual(
      test_message,
      MakeJsonObject("optional_int32", std::numeric_limits<int32_t>::max())))
      << test_message.DebugString();
}

TEST(MakeJsonValueMessage, OptionalInt64) {
  TestMessage test_message;
  test_message.set_optional_int64(0);
  EXPECT_TRUE(
      AreJsonValuesEqual(test_message, MakeJsonObject("optional_int64", 0)))
      << test_message.DebugString();

  test_message.set_optional_int64(std::numeric_limits<int64_t>::min());
  EXPECT_TRUE(AreJsonValuesEqual(
      test_message,
      MakeJsonObject("optional_int64", std::numeric_limits<int64_t>::min())))
      << test_message.DebugString();

  test_message.set_optional_int64(std::numeric_limits<int64_t>::max());
  EXPECT_TRUE(AreJsonValuesEqual(
      test_message,
      MakeJsonObject("optional_int64", std::numeric_limits<int64_t>::max())))
      << test_message.DebugString();
}

TEST(MakeJsonValueMessage, OptionalUInt32) {
  TestMessage test_message;
  test_message.set_optional_uint32(0);
  EXPECT_TRUE(
      AreJsonValuesEqual(test_message, MakeJsonObject("optional_uint32", 0)))
      << test_message.DebugString();

  test_message.set_optional_uint32(std::numeric_limits<uint32_t>::max());
  EXPECT_TRUE(AreJsonValuesEqual(
      test_message,
      MakeJsonObject("optional_uint32", std::numeric_limits<uint32_t>::max())))
      << test_message.DebugString();
}

TEST(MakeJsonValueMessage, OptionalUInt64) {
  TestMessage test_message;
  test_message.set_optional_uint64(0);
  EXPECT_TRUE(
      AreJsonValuesEqual(test_message, MakeJsonObject("optional_uint64", 0)))
      << test_message.DebugString();

  test_message.set_optional_uint64(std::numeric_limits<uint64_t>::max());
  EXPECT_TRUE(AreJsonValuesEqual(
      test_message,
      MakeJsonObject("optional_uint64", std::numeric_limits<uint64_t>::max())))
      << test_message.DebugString();
}

TEST(MakeJsonValueMessage, OptionalDouble) {
  TestMessage test_message;
  test_message.set_optional_double(0);
  EXPECT_TRUE(
      AreJsonValuesEqual(test_message, MakeJsonObject("optional_double", 0)))
      << test_message.DebugString();

  test_message.set_optional_double(1.23e-4);
  EXPECT_TRUE(AreJsonValuesEqual(test_message,
                                 MakeJsonObject("optional_double", 1.23e-4)))
      << test_message.DebugString();

  test_message.set_optional_double(std::numeric_limits<double>::infinity());
  EXPECT_TRUE(AreJsonValuesEqual(
      test_message, MakeJsonObject("optional_double",
                                   std::numeric_limits<double>::infinity())))
      << test_message.DebugString();

  test_message.set_optional_double(-std::numeric_limits<double>::infinity());
  EXPECT_TRUE(AreJsonValuesEqual(
      test_message, MakeJsonObject("optional_double",
                                   -std::numeric_limits<double>::infinity())))
      << test_message.DebugString();

  test_message.set_optional_double(std::numeric_limits<double>::quiet_NaN());
  EXPECT_TRUE(AreJsonValuesEqual(
      test_message, MakeJsonObject("optional_double",
                                   std::numeric_limits<double>::quiet_NaN())))
      << test_message.DebugString();
}

TEST(MakeJsonValueMessage, OptionalFloat) {
  TestMessage test_message;
  test_message.set_optional_float(0);
  EXPECT_TRUE(
      AreJsonValuesEqual(test_message, MakeJsonObject("optional_float", 0)))
      << test_message.DebugString();

  test_message.set_optional_float(1.23e-4);
  EXPECT_TRUE(AreJsonValuesEqual(test_message,
                                 MakeJsonObject("optional_float", 1.23e-4f)))
      << test_message.DebugString();

  test_message.set_optional_float(std::numeric_limits<float>::infinity());
  EXPECT_TRUE(AreJsonValuesEqual(
      test_message,
      MakeJsonObject("optional_float", std::numeric_limits<float>::infinity())))
      << test_message.DebugString();

  test_message.set_optional_float(-std::numeric_limits<float>::infinity());
  EXPECT_TRUE(AreJsonValuesEqual(
      test_message, MakeJsonObject("optional_float",
                                   -std::numeric_limits<float>::infinity())))
      << test_message.DebugString();

  test_message.set_optional_float(std::numeric_limits<float>::quiet_NaN());
  EXPECT_TRUE(AreJsonValuesEqual(
      test_message, MakeJsonObject("optional_float",
                                   std::numeric_limits<float>::quiet_NaN())))
      << test_message.DebugString();
}

TEST(MakeJsonValueMessage, OptionalBool) {
  TestMessage test_message;
  test_message.set_optional_bool(true);
  EXPECT_TRUE(
      AreJsonValuesEqual(test_message, MakeJsonObject("optional_bool", true)))
      << test_message.DebugString();

  test_message.set_optional_bool(false);
  EXPECT_TRUE(
      AreJsonValuesEqual(test_message, MakeJsonObject("optional_bool", false)))
      << test_message.DebugString();
}

TEST(MakeJsonValueMessage, OptionalEnum) {
  TestMessage test_message;
  test_message.set_optional_enum(TestMessage::TEST_ALPHA);
  EXPECT_TRUE(AreJsonValuesEqual(test_message,
                                 MakeJsonObject("optional_enum", "TEST_ALPHA")))
      << test_message.DebugString();

  test_message.set_optional_enum(TestMessage::TEST_BETA);
  EXPECT_TRUE(AreJsonValuesEqual(test_message,
                                 MakeJsonObject("optional_enum", "TEST_BETA")))
      << test_message.DebugString();
}

TEST(MakeJsonValueMessage, OptionalString) {
  TestMessage test_message;
  test_message.set_optional_string("");
  EXPECT_TRUE(
      AreJsonValuesEqual(test_message, MakeJsonObject("optional_string", "")))
      << test_message.DebugString();

  test_message.set_optional_string("alpha");
  EXPECT_TRUE(AreJsonValuesEqual(test_message,
                                 MakeJsonObject("optional_string", "alpha")))
      << test_message.DebugString();

  test_message.set_optional_string("\xe4\xbd\xa0\xe5\xa5\xbd");
  EXPECT_TRUE(AreJsonValuesEqual(
      test_message,
      MakeJsonObject("optional_string", "\xe4\xbd\xa0\xe5\xa5\xbd")))
      << test_message.DebugString();
}

TEST(MakeJsonValueMessage, OptionalBytes) {
  TestMessage test_message;
  test_message.set_optional_bytes("");
  EXPECT_TRUE(
      AreJsonValuesEqual(test_message, MakeJsonObject("optional_bytes", "")))
      << test_message.DebugString();

  test_message.set_optional_bytes("\xe4\xbd\xa0\xe5\xa5\xbd");
  EXPECT_TRUE(AreJsonValuesEqual(test_message,
                                 MakeJsonObject("optional_bytes", "5L2g5aW9")))
      << test_message.DebugString();
}

TEST(MakeJsonValueMessage, OptionalMessage) {
  TestMessage test_message;
  EXPECT_TRUE(AreJsonValuesEqual(test_message, MakeJsonObject()))
      << test_message.DebugString();

  test_message.mutable_optional_message();
  EXPECT_TRUE(AreJsonValuesEqual(
      test_message, MakeJsonObject("optional_message", MakeJsonObject())))
      << test_message.DebugString();

  test_message.mutable_optional_message()->set_optional_bool(true);
  EXPECT_TRUE(AreJsonValuesEqual(
      test_message, MakeJsonObject("optional_message",
                                   MakeJsonObject("optional_bool", true))))
      << test_message.DebugString();

  test_message.mutable_optional_message()->mutable_optional_message();
  EXPECT_TRUE(AreJsonValuesEqual(
      test_message,
      MakeJsonObject("optional_message",
                     MakeJsonObject("optional_bool", true, "optional_message",
                                    MakeJsonObject()))))
      << test_message.DebugString();

  test_message.mutable_optional_message()
      ->mutable_optional_message()
      ->set_optional_bool(false);
  EXPECT_TRUE(AreJsonValuesEqual(
      test_message,
      MakeJsonObject("optional_message",
                     MakeJsonObject("optional_bool", true, "optional_message",
                                    MakeJsonObject("optional_bool", false)))))
      << test_message.DebugString();
}

TEST(MakeJsonValueMessage, RepeatedInt32) {
  TestMessage test_message;
  test_message.add_repeated_int32(0);
  EXPECT_TRUE(AreJsonValuesEqual(
      test_message, MakeJsonObject("repeated_int32", MakeJsonArray(0))))
      << test_message.DebugString();

  test_message.add_repeated_int32(std::numeric_limits<int32_t>::min());
  EXPECT_TRUE(AreJsonValuesEqual(
      test_message,
      MakeJsonObject("repeated_int32",
                     MakeJsonArray(0, std::numeric_limits<int32_t>::min()))))
      << test_message.DebugString();

  test_message.add_repeated_int32(std::numeric_limits<int32_t>::max());
  EXPECT_TRUE(AreJsonValuesEqual(
      test_message,
      MakeJsonObject("repeated_int32",
                     MakeJsonArray(0, std::numeric_limits<int32_t>::min(),
                                   std::numeric_limits<int32_t>::max()))))
      << test_message.DebugString();
}

TEST(MakeJsonValueMessage, RepeatedInt64) {
  TestMessage test_message;
  test_message.add_repeated_int64(0);
  EXPECT_TRUE(AreJsonValuesEqual(
      test_message, MakeJsonObject("repeated_int64", MakeJsonArray(0))))
      << test_message.DebugString();

  test_message.add_repeated_int64(std::numeric_limits<int64_t>::min());
  EXPECT_TRUE(AreJsonValuesEqual(
      test_message,
      MakeJsonObject("repeated_int64",
                     MakeJsonArray(0, std::numeric_limits<int64_t>::min()))))
      << test_message.DebugString();

  test_message.add_repeated_int64(std::numeric_limits<int64_t>::max());
  EXPECT_TRUE(AreJsonValuesEqual(
      test_message,
      MakeJsonObject("repeated_int64",
                     MakeJsonArray(0, std::numeric_limits<int64_t>::min(),
                                   std::numeric_limits<int64_t>::max()))))
      << test_message.DebugString();
}

TEST(MakeJsonValueMessage, RepeatedUInt32) {
  TestMessage test_message;
  test_message.add_repeated_uint32(0);
  EXPECT_TRUE(AreJsonValuesEqual(
      test_message, MakeJsonObject("repeated_uint32", MakeJsonArray(0))))
      << test_message.DebugString();

  test_message.add_repeated_uint32(std::numeric_limits<uint32_t>::max());
  EXPECT_TRUE(AreJsonValuesEqual(
      test_message,
      MakeJsonObject("repeated_uint32",
                     MakeJsonArray(0, std::numeric_limits<uint32_t>::max()))))
      << test_message.DebugString();
}

TEST(MakeJsonValueMessage, RepeatedUInt64) {
  TestMessage test_message;
  test_message.add_repeated_uint64(0);
  EXPECT_TRUE(AreJsonValuesEqual(
      test_message, MakeJsonObject("repeated_uint64", MakeJsonArray(0))))
      << test_message.DebugString();

  test_message.add_repeated_uint64(std::numeric_limits<uint64_t>::max());
  EXPECT_TRUE(AreJsonValuesEqual(
      test_message,
      MakeJsonObject("repeated_uint64",
                     MakeJsonArray(0, std::numeric_limits<uint64_t>::max()))))
      << test_message.DebugString();
}

TEST(MakeJsonValueMessage, RepeatedDouble) {
  TestMessage test_message;
  test_message.add_repeated_double(0);
  EXPECT_TRUE(AreJsonValuesEqual(
      test_message, MakeJsonObject("repeated_double", MakeJsonArray(0))))
      << test_message.DebugString();

  test_message.add_repeated_double(1.23e-4);
  EXPECT_TRUE(AreJsonValuesEqual(
      test_message,
      MakeJsonObject("repeated_double", MakeJsonArray(0, 1.23e-4))))
      << test_message.DebugString();

  test_message.add_repeated_double(std::numeric_limits<double>::infinity());
  EXPECT_TRUE(AreJsonValuesEqual(
      test_message,
      MakeJsonObject(
          "repeated_double",
          MakeJsonArray(0, 1.23e-4, std::numeric_limits<double>::infinity()))))
      << test_message.DebugString();

  test_message.add_repeated_double(-std::numeric_limits<double>::infinity());
  EXPECT_TRUE(AreJsonValuesEqual(
      test_message,
      MakeJsonObject(
          "repeated_double",
          MakeJsonArray(0, 1.23e-4, std::numeric_limits<double>::infinity(),
                        -std::numeric_limits<double>::infinity()))))
      << test_message.DebugString();

  test_message.add_repeated_double(std::numeric_limits<double>::quiet_NaN());
  EXPECT_TRUE(AreJsonValuesEqual(
      test_message,
      MakeJsonObject(
          "repeated_double",
          MakeJsonArray(0, 1.23e-4, std::numeric_limits<double>::infinity(),
                        -std::numeric_limits<double>::infinity(),
                        std::numeric_limits<double>::quiet_NaN()))))
      << test_message.DebugString();
}

TEST(MakeJsonValueMessage, RepeatedFloat) {
  TestMessage test_message;
  test_message.add_repeated_float(0);
  EXPECT_TRUE(AreJsonValuesEqual(
      test_message, MakeJsonObject("repeated_float", MakeJsonArray(0))))
      << test_message.DebugString();

  test_message.add_repeated_float(1.23e-4);
  EXPECT_TRUE(AreJsonValuesEqual(
      test_message,
      MakeJsonObject("repeated_float", MakeJsonArray(0, 1.23e-4f))))
      << test_message.DebugString();

  test_message.add_repeated_float(std::numeric_limits<float>::infinity());
  EXPECT_TRUE(AreJsonValuesEqual(
      test_message,
      MakeJsonObject(
          "repeated_float",
          MakeJsonArray(0, 1.23e-4f, std::numeric_limits<float>::infinity()))))
      << test_message.DebugString();

  test_message.add_repeated_float(-std::numeric_limits<float>::infinity());
  EXPECT_TRUE(AreJsonValuesEqual(
      test_message,
      MakeJsonObject(
          "repeated_float",
          MakeJsonArray(0, 1.23e-4f, std::numeric_limits<float>::infinity(),
                        -std::numeric_limits<float>::infinity()))))
      << test_message.DebugString();

  test_message.add_repeated_float(std::numeric_limits<float>::quiet_NaN());
  EXPECT_TRUE(AreJsonValuesEqual(
      test_message,
      MakeJsonObject(
          "repeated_float",
          MakeJsonArray(0, 1.23e-4f, std::numeric_limits<float>::infinity(),
                        -std::numeric_limits<float>::infinity(),
                        std::numeric_limits<float>::quiet_NaN()))))
      << test_message.DebugString();
}

TEST(MakeJsonValueMessage, RepeatedBool) {
  TestMessage test_message;
  test_message.add_repeated_bool(true);
  EXPECT_TRUE(AreJsonValuesEqual(
      test_message, MakeJsonObject("repeated_bool", MakeJsonArray(true))))
      << test_message.DebugString();

  test_message.add_repeated_bool(false);
  EXPECT_TRUE(AreJsonValuesEqual(
      test_message,
      MakeJsonObject("repeated_bool", MakeJsonArray(true, false))))
      << test_message.DebugString();
}

TEST(MakeJsonValueMessage, RepeatedEnum) {
  TestMessage test_message;
  test_message.add_repeated_enum(TestMessage::TEST_ALPHA);
  EXPECT_TRUE(AreJsonValuesEqual(
      test_message,
      MakeJsonObject("repeated_enum", MakeJsonArray("TEST_ALPHA"))))
      << test_message.DebugString();

  test_message.add_repeated_enum(TestMessage::TEST_BETA);
  EXPECT_TRUE(AreJsonValuesEqual(
      test_message, MakeJsonObject("repeated_enum",
                                   MakeJsonArray("TEST_ALPHA", "TEST_BETA"))))
      << test_message.DebugString();
}

TEST(MakeJsonValueMessage, RepeatedString) {
  TestMessage test_message;
  test_message.add_repeated_string("");
  EXPECT_TRUE(AreJsonValuesEqual(
      test_message, MakeJsonObject("repeated_string", MakeJsonArray(""))))
      << test_message.DebugString();

  test_message.add_repeated_string("alpha");
  EXPECT_TRUE(AreJsonValuesEqual(
      test_message,
      MakeJsonObject("repeated_string", MakeJsonArray("", "alpha"))))
      << test_message.DebugString();

  test_message.add_repeated_string("\xe4\xbd\xa0\xe5\xa5\xbd");
  EXPECT_TRUE(AreJsonValuesEqual(
      test_message,
      MakeJsonObject("repeated_string",
                     MakeJsonArray("", "alpha", "\xe4\xbd\xa0\xe5\xa5\xbd"))))
      << test_message.DebugString();
}

TEST(MakeJsonValueMessage, RepeatedBytes) {
  TestMessage test_message;
  test_message.add_repeated_bytes("");
  EXPECT_TRUE(AreJsonValuesEqual(
      test_message, MakeJsonObject("repeated_bytes", MakeJsonArray(""))))
      << test_message.DebugString();

  test_message.add_repeated_bytes("\xe4\xbd\xa0\xe5\xa5\xbd");
  EXPECT_TRUE(AreJsonValuesEqual(
      test_message,
      MakeJsonObject("repeated_bytes", MakeJsonArray("", "5L2g5aW9"))))
      << test_message.DebugString();
}

TEST(MakeJsonValueMessage, RepeatedMessage) {
  TestMessage test_message;
  EXPECT_TRUE(AreJsonValuesEqual(test_message, MakeJsonObject()))
      << test_message.DebugString();

  test_message.add_repeated_message();
  EXPECT_TRUE(AreJsonValuesEqual(
      test_message,
      MakeJsonObject("repeated_message", MakeJsonArray(MakeJsonObject()))))
      << test_message.DebugString();

  test_message.mutable_repeated_message(0)->set_optional_bool(true);
  EXPECT_TRUE(AreJsonValuesEqual(
      test_message,
      MakeJsonObject("repeated_message",
                     MakeJsonArray(MakeJsonObject("optional_bool", true)))))
      << test_message.DebugString();

  test_message.mutable_repeated_message(0)->add_repeated_message();
  EXPECT_TRUE(AreJsonValuesEqual(
      test_message,
      MakeJsonObject("repeated_message",
                     MakeJsonArray(MakeJsonObject(
                         "optional_bool", true, "repeated_message",
                         MakeJsonArray(MakeJsonObject()))))))
      << test_message.DebugString();

  test_message.mutable_repeated_message(0)
      ->mutable_repeated_message(0)
      ->set_optional_bool(false);
  EXPECT_TRUE(AreJsonValuesEqual(
      test_message,
      MakeJsonObject(
          "repeated_message",
          MakeJsonArray(MakeJsonObject(
              "optional_bool", true, "repeated_message",
              MakeJsonArray(MakeJsonObject("optional_bool", false)))))))
      << test_message.DebugString();
}

TEST(MakeJsonValueMessage, JsonValue) {
  EXPECT_TRUE(AreJsonValuesEqual(
      static_cast<const google::protobuf::Message&>(JsonNull()), JsonNull()));

  EXPECT_TRUE(AreJsonValuesEqual(
      static_cast<const google::protobuf::Message&>(MakeJsonValue(true)),
      true));

  EXPECT_TRUE(AreJsonValuesEqual(
      static_cast<const google::protobuf::Message&>(MakeJsonValue(-123)),
      -123));

  EXPECT_TRUE(AreJsonValuesEqual(
      static_cast<const google::protobuf::Message&>(MakeJsonValue(456ull)),
      456ull));

  EXPECT_TRUE(AreJsonValuesEqual(
      static_cast<const google::protobuf::Message&>(MakeJsonValue(-12.34e-5)),
      -12.34e-5));

  EXPECT_TRUE(AreJsonValuesEqual(
      static_cast<const google::protobuf::Message&>(MakeJsonValue("alpha")),
      "alpha"));

  EXPECT_TRUE(
      AreJsonValuesEqual(static_cast<const google::protobuf::Message&>(
                             MakeJsonObject("delta", 789, "epsilon", 135ull)),
                         MakeJsonObject("delta", 789, "epsilon", 135ull)));

  EXPECT_TRUE(AreJsonValuesEqual(
      static_cast<const google::protobuf::Message&>(MakeJsonArray(24.68, true)),
      MakeJsonArray(24.68, true)));
}

TEST(MakeJsonValueMessage, WithObjectProperties) {
  TestMessageWithObjectProperties test_message;
  *test_message.add_object_properties() = MakeJsonProperty("alpha", JsonNull());
  EXPECT_TRUE(
      AreJsonValuesEqual(test_message, MakeJsonObject("alpha", JsonNull())))
      << test_message.DebugString();

  *test_message.add_object_properties() = MakeJsonProperty("beta", true);
  EXPECT_TRUE(AreJsonValuesEqual(
      test_message, MakeJsonObject("alpha", JsonNull(), "beta", true)))
      << test_message.DebugString();

  *test_message.add_object_properties() = MakeJsonProperty("alpha", -123);
  EXPECT_TRUE(AreJsonValuesEqual(
      test_message,
      MakeJsonObject("alpha", JsonNull(), "beta", true, "alpha", -123)))
      << test_message.DebugString();
}

}  // namespace pjcore
