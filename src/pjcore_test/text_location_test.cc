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

#include "pjcore/text_location.h"

#include <gtest/gtest.h>

namespace pjcore {

namespace {

::testing::AssertionResult TestAdvance(uint64_t initial_offset,
                                       uint64_t initial_line,
                                       uint64_t initial_column, char ch,
                                       uint64_t expected_offset,
                                       uint64_t expected_line,
                                       uint64_t expected_column) {
  TextLocation location =
      MakeTextLocation(initial_offset, initial_line, initial_column);
  AdvanceTextLocation(&location, ch);
  if (expected_offset == location.offset() &&
      expected_line == location.line() &&
      expected_column == location.column()) {
    return ::testing::AssertionSuccess();
  } else {
    return ::testing::AssertionFailure() << location.offset() << ", "
                                         << location.line() << ", "
                                         << location.column();
  }
}

::testing::AssertionResult TestAdvance(uint64_t initial_offset,
                                       uint64_t initial_line,
                                       uint64_t initial_column, StringPiece str,
                                       uint64_t expected_offset,
                                       uint64_t expected_line,
                                       uint64_t expected_column) {
  TextLocation location =
      MakeTextLocation(initial_offset, initial_line, initial_column);
  AdvanceTextLocation(&location, str);
  if (expected_offset == location.offset() &&
      expected_line == location.line() &&
      expected_column == location.column()) {
    return ::testing::AssertionSuccess();
  } else {
    return ::testing::AssertionFailure() << location.offset() << ", "
                                         << location.line() << ", "
                                         << location.column();
  }
}

}  // unnamed namespace

TEST(TextLocation, DefaultInstance) {
  EXPECT_EQ(0, TextLocation::default_instance().offset());
  EXPECT_EQ(1, TextLocation::default_instance().line());
  EXPECT_EQ(1, TextLocation::default_instance().column());
}

TEST(TextLocation, DefaultConstructor) {
  TextLocation text_location;

  EXPECT_EQ(0, text_location.offset());
  EXPECT_EQ(1, text_location.line());
  EXPECT_EQ(1, text_location.column());
}

TEST(TextLocation, MakeDefault) {
  TextLocation text_location = MakeTextLocation();

  EXPECT_TRUE(text_location.has_offset());
  EXPECT_TRUE(text_location.has_line());
  EXPECT_TRUE(text_location.has_column());

  EXPECT_EQ(0, text_location.offset());
  EXPECT_EQ(1, text_location.line());
  EXPECT_EQ(1, text_location.column());
}

TEST(TextLocation, MakeExplicit) {
  TextLocation text_location = MakeTextLocation(1, 2, 3);

  EXPECT_TRUE(text_location.has_offset());
  EXPECT_TRUE(text_location.has_line());
  EXPECT_TRUE(text_location.has_column());

  EXPECT_EQ(1, text_location.offset());
  EXPECT_EQ(2, text_location.line());
  EXPECT_EQ(3, text_location.column());
}

TEST(TextLocation, MakeAdvance) {
  TextLocation text_location = MakeTextLocation("alpha\n\tbeta");

  EXPECT_TRUE(text_location.has_offset());
  EXPECT_TRUE(text_location.has_line());
  EXPECT_TRUE(text_location.has_column());

  EXPECT_EQ(11, text_location.offset());
  EXPECT_EQ(2, text_location.line());
  EXPECT_EQ(13, text_location.column());
}

TEST(TextLocation, Advance) {
  EXPECT_TRUE(TestAdvance(0, 1, 1, "Hello, world!", 13, 1, 14));
  EXPECT_TRUE(TestAdvance(0, 1, 1, "Hello,\nworld!\n", 14, 3, 1));
  EXPECT_TRUE(TestAdvance(0, 1, 1, "Hello,\tworld!\n", 14, 2, 1));
  EXPECT_TRUE(TestAdvance(0, 1, 1, "Hello,\nworld!\t", 14, 2, 9));
  EXPECT_TRUE(TestAdvance(0, 1, 1, "Hello,\tworld!\t", 14, 1, 17));
  EXPECT_TRUE(TestAdvance(1, 3, 4, "", 1, 3, 4));
  EXPECT_TRUE(TestAdvance(1, 3, 4, "a", 2, 3, 5));
  EXPECT_TRUE(TestAdvance(1, 3, 4, "ab", 3, 3, 6));
  EXPECT_TRUE(TestAdvance(1, 3, 4, "abc", 4, 3, 7));
  EXPECT_TRUE(TestAdvance(1, 3, 4, "abcd", 5, 3, 8));
  EXPECT_TRUE(TestAdvance(1, 3, 4, "abcde", 6, 3, 9));
}

TEST(TextLocation, AdvanceNewline) {
  EXPECT_TRUE(TestAdvance(0, 1, 1, '\n', 1, 2, 1));
  EXPECT_TRUE(TestAdvance(1, 1, 2, '\n', 2, 2, 1));
  EXPECT_TRUE(TestAdvance(1, 2, 1, '\n', 2, 3, 1));

  EXPECT_TRUE(TestAdvance(0, 1, 1, "\n", 1, 2, 1));
  EXPECT_TRUE(TestAdvance(0, 1, 1, " \n", 2, 2, 1));
  EXPECT_TRUE(TestAdvance(0, 1, 1, "\n\n", 2, 3, 1));
}

TEST(TextLocation, AdvanceTab) {
  EXPECT_TRUE(TestAdvance(0, 1, 1, '\t', 1, 1, 9));
  EXPECT_TRUE(TestAdvance(1, 1, 2, '\t', 2, 1, 9));
  EXPECT_TRUE(TestAdvance(2, 1, 3, '\t', 3, 1, 9));
  EXPECT_TRUE(TestAdvance(3, 1, 4, '\t', 4, 1, 9));
  EXPECT_TRUE(TestAdvance(4, 1, 5, '\t', 5, 1, 9));
  EXPECT_TRUE(TestAdvance(5, 1, 6, '\t', 6, 1, 9));
  EXPECT_TRUE(TestAdvance(6, 1, 7, '\t', 7, 1, 9));
  EXPECT_TRUE(TestAdvance(7, 1, 8, '\t', 8, 1, 9));
  EXPECT_TRUE(TestAdvance(8, 1, 9, '\t', 9, 1, 17));

  EXPECT_TRUE(TestAdvance(0, 1, 1, "\t", 1, 1, 9));
  EXPECT_TRUE(TestAdvance(0, 1, 1, "a\t", 2, 1, 9));
  EXPECT_TRUE(TestAdvance(0, 1, 1, "ab\t", 3, 1, 9));
  EXPECT_TRUE(TestAdvance(0, 1, 1, "abc\t", 4, 1, 9));
  EXPECT_TRUE(TestAdvance(0, 1, 1, "abcd\t", 5, 1, 9));
  EXPECT_TRUE(TestAdvance(0, 1, 1, "abcde\t", 6, 1, 9));
  EXPECT_TRUE(TestAdvance(0, 1, 1, "abcdef\t", 7, 1, 9));
  EXPECT_TRUE(TestAdvance(0, 1, 1, "abcdefg\t", 8, 1, 9));
  EXPECT_TRUE(TestAdvance(0, 1, 1, "abcdefgh\t", 9, 1, 17));
}

TEST(TextLocation, AdvanceUtf8) {
  EXPECT_TRUE(TestAdvance(0, 1, 1, '\xe4', 1, 1, 2));
  EXPECT_TRUE(TestAdvance(1, 1, 2, '\xbd', 2, 1, 2));
  EXPECT_TRUE(TestAdvance(2, 1, 2, '\xa0', 3, 1, 2));

  EXPECT_TRUE(TestAdvance(0, 1, 1, "\xe4\xbd\xa0\xe5\xa5\xbd", 6, 1, 3));
}

}  // namespace pjcore
