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

#include "gtest/gtest.h"

#include "pjcore/error_util.h"
#include "pjcore/logging.h"
#include "pjcore/string_piece_util.h"

namespace pjcore {

TEST(Base64, Write) {
  EXPECT_EQ("cGxlYXN1cmUu", WriteBase64("pleasure."));
  EXPECT_EQ("bGVhc3VyZS4=", WriteBase64("leasure."));
  EXPECT_EQ("ZWFzdXJlLg==", WriteBase64("easure."));
  EXPECT_EQ("YXN1cmUu", WriteBase64("asure."));
  EXPECT_EQ("c3VyZS4=", WriteBase64("sure."));
}

TEST(Base64, ReadSuccess) {
  std::string unencoded;
  Error error;
  EXPECT_TRUE(ReadBase64("cGxlYXN1cmUu", &unencoded, &error))
      << ErrorToString(error);
  EXPECT_EQ("pleasure.", unencoded);
  EXPECT_TRUE(ReadBase64("bGVhc3VyZS4=", &unencoded, &error))
      << ErrorToString(error);
  EXPECT_EQ("leasure.", unencoded);
  EXPECT_TRUE(ReadBase64("ZWFzdXJlLg==", &unencoded, &error))
      << ErrorToString(error);
  EXPECT_EQ("easure.", unencoded);
  EXPECT_TRUE(ReadBase64("YXN1cmUu", &unencoded, &error))
      << ErrorToString(error);
  EXPECT_EQ("asure.", unencoded);
  EXPECT_TRUE(ReadBase64("c3VyZS4=", &unencoded, &error))
      << ErrorToString(error);
  EXPECT_EQ("sure.", unencoded);
}

TEST(Base64, ReadFailure) {
  std::string unencoded;
  Error error;

  GlobalLogOverride global_log_override;
  EXPECT_FALSE(ReadBase64("cGxlYXN1cmU", &unencoded, &error));
  EXPECT_FALSE(ReadBase64("bGVhc3VyZ===", &unencoded, &error));
  EXPECT_FALSE(ReadBase64("ZWFzdXJlL/==", &unencoded, &error));
  EXPECT_FALSE(ReadBase64("YXN1cmU", &unencoded, &error));
  EXPECT_FALSE(ReadBase64("c3VyZS+=", &unencoded, &error));
}
}  // namespace pjcore
