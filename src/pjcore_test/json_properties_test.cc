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

#include <gtest/gtest.h>

#include "pjcore/json_util.h"

namespace pjcore {

TEST(NormalizeJsonProperties, Flat) {
  JsonValue root = MakeJsonObject("gamma", "delta", "alpha", -123, "beta",
                                  456ull, "alpha", 7.89);

  NormalizeJsonProperties(&root);
  std::string error;
  ASSERT_TRUE(VerifyJsonType(root, &error));
  ASSERT_EQ(JsonValue::TYPE_OBJECT, root.type());
  ASSERT_EQ(3, root.object_properties_size());
  ASSERT_EQ("alpha", root.object_properties(0).name());
  ASSERT_EQ(JsonValue::TYPE_SIGNED, root.object_properties(0).value().type());
  ASSERT_EQ(-123, root.object_properties(0).value().signed_value());
  ASSERT_EQ("beta", root.object_properties(1).name());
  ASSERT_EQ(JsonValue::TYPE_UNSIGNED, root.object_properties(1).value().type());
  ASSERT_EQ(456, root.object_properties(1).value().unsigned_value());
  ASSERT_EQ("gamma", root.object_properties(2).name());
  ASSERT_EQ(JsonValue::TYPE_STRING, root.object_properties(2).value().type());
  ASSERT_EQ("delta", root.object_properties(2).value().string_value());
}

TEST(NormalizeJsonProperties, Nested) {
  JsonValue root = MakeJsonArray(MakeJsonObject(
      "beta",
      MakeJsonArray(MakeJsonObject("epsilon", -123, "epsilon", 456.0))));

  NormalizeJsonProperties(&root);
  std::string error;
  ASSERT_TRUE(VerifyJsonType(root, &error));
  ASSERT_EQ(JsonValue::TYPE_ARRAY, root.type());
  ASSERT_EQ(1, root.array_elements_size());
  ASSERT_EQ(JsonValue::TYPE_OBJECT, root.array_elements(0).type());
  ASSERT_EQ(1, root.array_elements(0).object_properties_size());
  ASSERT_EQ("beta", root.array_elements(0).object_properties(0).name());
  ASSERT_EQ(JsonValue::TYPE_ARRAY,
            root.array_elements(0).object_properties(0).value().type());
  ASSERT_EQ(1, root.array_elements(0)
                   .object_properties(0)
                   .value()
                   .array_elements_size());
  ASSERT_EQ(JsonValue::TYPE_OBJECT, root.array_elements(0)
                                        .object_properties(0)
                                        .value()
                                        .array_elements(0)
                                        .type());
  ASSERT_EQ(1, root.array_elements(0)
                   .object_properties(0)
                   .value()
                   .array_elements(0)
                   .object_properties_size());
  ASSERT_EQ("epsilon", root.array_elements(0)
                           .object_properties(0)
                           .value()
                           .array_elements(0)
                           .object_properties(0)
                           .name());
  ASSERT_EQ(JsonValue::TYPE_SIGNED, root.array_elements(0)
                                        .object_properties(0)
                                        .value()
                                        .array_elements(0)
                                        .object_properties(0)
                                        .value()
                                        .type());
  ASSERT_EQ(-123, root.array_elements(0)
                      .object_properties(0)
                      .value()
                      .array_elements(0)
                      .object_properties(0)
                      .value()
                      .signed_value());
}

TEST(AreJsonPropertiesNormalized, NoProperties) {
  ASSERT_TRUE(AreJsonPropertiesNormalized(JsonNull()));
  ASSERT_TRUE(AreJsonPropertiesNormalized(MakeJsonValue("alpha")));
  ASSERT_TRUE(AreJsonPropertiesNormalized(MakeJsonValue(-123)));
  ASSERT_TRUE(AreJsonPropertiesNormalized(MakeJsonValue(456ull)));
  ASSERT_TRUE(AreJsonPropertiesNormalized(MakeJsonValue(7.89)));
  ASSERT_TRUE(AreJsonPropertiesNormalized(MakeJsonValue(true)));
  ASSERT_TRUE(AreJsonPropertiesNormalized(MakeJsonArray("beta", false)));
}

TEST(AreJsonPropertiesNormalized, NormalizedProperties) {
  JsonValue root = MakeJsonObject(
      "alpha", -123, "beta",
      MakeJsonArray(MakeJsonObject("delta", 78.9, "epsilon", false)));

  ASSERT_TRUE(AreJsonPropertiesNormalized(root));
}

TEST(AreJsonPropertiesNormalized, WrongOrder) {
  JsonValue root = MakeJsonObject(
      "alpha", -123, "beta",
      MakeJsonArray(MakeJsonObject("epsilon", false, "delta", 78.9)));

  std::string path;
  ASSERT_FALSE(AreJsonPropertiesNormalized(root, &path));
  ASSERT_EQ("$.beta[0].delta", path);
}

TEST(AreJsonPropertiesNormalized, Duplicates) {
  JsonValue root = MakeJsonObject(
      "alpha", -123, "beta",
      MakeJsonArray(MakeJsonObject("epsilon", false, "epsilon", 78.9)));

  std::string path;
  ASSERT_FALSE(AreJsonPropertiesNormalized(root, &path));
  ASSERT_EQ("$.beta[0].epsilon", path);
}

TEST(JsonProperties, HasJsonProperty) {
  ASSERT_FALSE(HasJsonProperty(MakeJsonObject(), "alpha"));

  ASSERT_TRUE(HasJsonProperty(MakeJsonObject("alpha", 1), "alpha"));
  ASSERT_FALSE(HasJsonProperty(MakeJsonObject("alpha", 1), "beta"));

  ASSERT_TRUE(HasJsonProperty(MakeJsonObject("beta", 1, "alpha", 2), "alpha"));
  ASSERT_TRUE(HasJsonProperty(MakeJsonObject("beta", 1, "alpha", 2), "beta"));
  ASSERT_FALSE(HasJsonProperty(MakeJsonObject("beta", 1, "alpha", 2), "gamma"));

  ASSERT_TRUE(HasJsonProperty(MakeJsonObject("beta", 1, "alpha", 2), "alpha"));
  ASSERT_TRUE(HasJsonProperty(MakeJsonObject("beta", 1, "alpha", 2), "beta"));
  ASSERT_FALSE(HasJsonProperty(MakeJsonObject("beta", 1, "alpha", 2), "gamma"));

  ASSERT_TRUE(HasJsonProperty(MakeJsonObject("alpha", JsonNull()), "alpha"));
}

TEST(JsonProperties, GetJsonProperty) {
  ASSERT_TRUE(AreJsonValuesEqual(GetJsonProperty(MakeJsonObject(), "alpha"),
                                 JsonNull()));

  std::string diff_path;

  ASSERT_FALSE(AreJsonValuesEqual(GetJsonProperty(MakeJsonObject(), "alpha"),
                                  "alpha", &diff_path))
      << diff_path;
}

TEST(JsonProperties, GetMutableJsonProperty) {
  // TODO(pjcore): implement
}

TEST(JsonProperties, SetJsonProperty) {
  // TODO(pjcore): implement
}

TEST(JsonProperties, ClearJsonProperty) {
  // TODO(pjcore): implement
}

}  // namespace pjcore
