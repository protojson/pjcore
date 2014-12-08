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

#include <limits>

#include "gtest/gtest.h"

#include "pjcore/number_util.h"

namespace pjcore {

namespace {

/**
 * Tests reading and writing of a number.
 * @param raw_str string to read.
 * @param expected_prefix_length expected length of prefix if successful or 0
 *          for expected failure.
 * @param expected_value expected value if successful or 0 for expected failure.
 * @param raw_expected_rewrite expected rewrite of the number if successful.
 */
template <typename Number>
::testing::AssertionResult TestNumber(const char *raw_str,
                                      size_t expected_prefix_length = 0,
                                      Number expected_value = 0,
                                      const char *raw_expected_rewrite = NULL) {
  StringPiece str(raw_str);

  Number actual_value;

  size_t actual_prefix_length = ReadNumberPrefix(str, &actual_value);

  if (actual_prefix_length > str.length()) {
    return ::testing::AssertionFailure()
           << "read outside of string: " << actual_prefix_length << " > "
           << str.length();
  }

  if (actual_prefix_length != expected_prefix_length) {
    return ::testing::AssertionFailure()
           << "incorrect prefix length: " << actual_prefix_length
           << " != " << expected_prefix_length;
  }

  if (expected_value == expected_value) {
    if (actual_value != expected_value) {
      return ::testing::AssertionFailure()
             << "incorrect value: " << actual_value << " != " << expected_value;
    }
  } else {
    // NaN expected that's the only number that's not equal to self.
    if (actual_value == actual_value) {
      return ::testing::AssertionFailure() << "NaN expected: " << actual_value;
    }
  }

  if (expected_prefix_length && actual_prefix_length) {
    StringPiece expected_rewrite(raw_expected_rewrite);

    WriteNumberBuffer buffer;
    StringPiece actual_rewrite = WriteNumberToBuffer(actual_value, &buffer);

    if (actual_rewrite != expected_rewrite) {
      return ::testing::AssertionFailure()
             << "incorrect rewrite: " << actual_rewrite.as_string()
             << " != " << expected_rewrite.as_string();
    }
  }

  return ::testing::AssertionSuccess();
}

/** Maximal integer value for 53-bit exponent field in double. */
static const int64_t kDoubleSignificandMaxInt = 0x1fffffffffffffll;

/** The same as double. */
static const double kDoubleSignificandMax =
    static_cast<double>(kDoubleSignificandMaxInt);

/** Maximal integer value for 24-bit exponent field in float. */
static const int64_t kFloatSignificandMaxInt = 0xffffffll;

/** The same as float. */
static const float kFloatSignificandMax =
    static_cast<float>(kFloatSignificandMaxInt);

}  // unnamed namespace

TEST(NumberUtil, int64_t) {
  // Regular values, and edge cases.
  EXPECT_TRUE(TestNumber<int64_t>("0", 1, 0, "0"));
  EXPECT_TRUE(TestNumber<int64_t>("1", 1, 1, "1"));
  EXPECT_TRUE(TestNumber<int64_t>("-1", 2, -1, "-1"));
  EXPECT_TRUE(TestNumber<int64_t>("12", 2, 12, "12"));
  EXPECT_TRUE(TestNumber<int64_t>("-12", 3, -12, "-12"));

  EXPECT_TRUE(TestNumber<int64_t>("9223372036854775807", 19,
                                  std::numeric_limits<int64_t>::max(),
                                  "9223372036854775807"));

  EXPECT_TRUE(TestNumber<int64_t>("-9223372036854775808", 20,
                                  std::numeric_limits<int64_t>::min(),
                                  "-9223372036854775808"));

  // Trailing characters.
  EXPECT_TRUE(TestNumber<int64_t>("0\0", 1, 0, "0"));
  EXPECT_TRUE(TestNumber<int64_t>("1 ", 1, 1, "1"));
  EXPECT_TRUE(TestNumber<int64_t>("-1+", 2, -1, "-1"));
  EXPECT_TRUE(TestNumber<int64_t>("12A", 2, 12, "12"));
  EXPECT_TRUE(TestNumber<int64_t>("-12.3", 3, -12, "-12"));

  EXPECT_TRUE(TestNumber<int64_t>("9223372036854775807^", 19,
                                  std::numeric_limits<int64_t>::max(),
                                  "9223372036854775807"));

  EXPECT_TRUE(TestNumber<int64_t>("-9223372036854775808\xff", 20,
                                  std::numeric_limits<int64_t>::min(),
                                  "-9223372036854775808"));

  // Leading zeroes.
  EXPECT_TRUE(TestNumber<int64_t>("000", 3, 0, "0"));
  EXPECT_TRUE(TestNumber<int64_t>("001", 3, 1, "1"));
  EXPECT_TRUE(TestNumber<int64_t>("-001", 4, -1, "-1"));
  EXPECT_TRUE(TestNumber<int64_t>("0012", 4, 12, "12"));
  EXPECT_TRUE(TestNumber<int64_t>("-0012", 5, -12, "-12"));

  EXPECT_TRUE(TestNumber<int64_t>("009223372036854775807", 21,
                                  std::numeric_limits<int64_t>::max(),
                                  "9223372036854775807"));

  EXPECT_TRUE(TestNumber<int64_t>("-009223372036854775808", 22,
                                  std::numeric_limits<int64_t>::min(),
                                  "-9223372036854775808"));

  // Invalid characters.
  EXPECT_TRUE(TestNumber<int64_t>(""));
  EXPECT_TRUE(TestNumber<int64_t>("+1"));
  EXPECT_TRUE(TestNumber<int64_t>("\0"));
  EXPECT_TRUE(TestNumber<int64_t>(" "));
  EXPECT_TRUE(TestNumber<int64_t>("+"));
  EXPECT_TRUE(TestNumber<int64_t>("A"));
  EXPECT_TRUE(TestNumber<int64_t>("."));

  // Overflow.
  EXPECT_TRUE(TestNumber<int64_t>("9223372036854775808"));
  EXPECT_TRUE(TestNumber<int64_t>("-9223372036854775809"));
}

TEST(NumberUtil, int32_t) {
  // Regular values, and edge cases.
  EXPECT_TRUE(TestNumber<int32_t>("0", 1, 0, "0"));
  EXPECT_TRUE(TestNumber<int32_t>("1", 1, 1, "1"));
  EXPECT_TRUE(TestNumber<int32_t>("-1", 2, -1, "-1"));
  EXPECT_TRUE(TestNumber<int32_t>("12", 2, 12, "12"));
  EXPECT_TRUE(TestNumber<int32_t>("-12", 3, -12, "-12"));

  EXPECT_TRUE(TestNumber<int32_t>(
      "2147483647", 10, std::numeric_limits<int32_t>::max(), "2147483647"));

  EXPECT_TRUE(TestNumber<int32_t>(
      "-2147483648", 11, std::numeric_limits<int32_t>::min(), "-2147483648"));

  // Trailing characters.
  EXPECT_TRUE(TestNumber<int32_t>("0\0", 1, 0, "0"));
  EXPECT_TRUE(TestNumber<int32_t>("1 ", 1, 1, "1"));
  EXPECT_TRUE(TestNumber<int32_t>("-1+", 2, -1, "-1"));
  EXPECT_TRUE(TestNumber<int32_t>("12A", 2, 12, "12"));
  EXPECT_TRUE(TestNumber<int32_t>("-12.3", 3, -12, "-12"));

  EXPECT_TRUE(TestNumber<int32_t>(
      "2147483647^", 10, std::numeric_limits<int32_t>::max(), "2147483647"));

  EXPECT_TRUE(TestNumber<int32_t>("-2147483648\xff", 11,
                                  std::numeric_limits<int32_t>::min(),
                                  "-2147483648"));

  // Leading zeroes.
  EXPECT_TRUE(TestNumber<int32_t>("000", 3, 0, "0"));
  EXPECT_TRUE(TestNumber<int32_t>("001", 3, 1, "1"));
  EXPECT_TRUE(TestNumber<int32_t>("-001", 4, -1, "-1"));
  EXPECT_TRUE(TestNumber<int32_t>("0012", 4, 12, "12"));
  EXPECT_TRUE(TestNumber<int32_t>("-0012", 5, -12, "-12"));

  EXPECT_TRUE(TestNumber<int32_t>(
      "002147483647", 12, std::numeric_limits<int32_t>::max(), "2147483647"));

  EXPECT_TRUE(TestNumber<int32_t>(
      "-002147483648", 13, std::numeric_limits<int32_t>::min(), "-2147483648"));

  // Invalid characters.
  EXPECT_TRUE(TestNumber<int32_t>(""));
  EXPECT_TRUE(TestNumber<int32_t>("+1"));

  // Trailing characters.
  EXPECT_TRUE(TestNumber<int32_t>("\0"));
  EXPECT_TRUE(TestNumber<int32_t>(" "));
  EXPECT_TRUE(TestNumber<int32_t>("+"));
  EXPECT_TRUE(TestNumber<int32_t>("A"));
  EXPECT_TRUE(TestNumber<int32_t>("."));

  // Overflow.
  EXPECT_TRUE(TestNumber<int32_t>("2147483648"));
  EXPECT_TRUE(TestNumber<int32_t>("-2147483649"));
}

TEST(NumberUtil, uint64_t) {
  // Regular values, and edge cases.
  EXPECT_TRUE(TestNumber<uint64_t>("0", 1, 0, "0"));
  EXPECT_TRUE(TestNumber<uint64_t>("1", 1, 1, "1"));
  EXPECT_TRUE(TestNumber<uint64_t>("12", 2, 12, "12"));
  EXPECT_TRUE(TestNumber<uint64_t>("1234", 4, 1234, "1234"));
  EXPECT_TRUE(TestNumber<uint64_t>("12345678", 8, 12345678, "12345678"));

  EXPECT_TRUE(TestNumber<uint64_t>("1234567890123456", 16, 1234567890123456,
                                   "1234567890123456"));

  EXPECT_TRUE(TestNumber<uint64_t>("18446744073709551615", 20,
                                   std::numeric_limits<uint64_t>::max(),
                                   "18446744073709551615"));

  // Trailing characters.
  EXPECT_TRUE(TestNumber<uint64_t>("0\0", 1, 0, "0"));
  EXPECT_TRUE(TestNumber<uint64_t>("1 ", 1, 1, "1"));
  EXPECT_TRUE(TestNumber<uint64_t>("12+", 2, 12, "12"));
  EXPECT_TRUE(TestNumber<uint64_t>("1234A", 4, 1234, "1234"));
  EXPECT_TRUE(TestNumber<uint64_t>("12345678.", 8, 12345678, "12345678"));

  EXPECT_TRUE(TestNumber<uint64_t>("1234567890123456^", 16, 1234567890123456,
                                   "1234567890123456"));

  EXPECT_TRUE(TestNumber<uint64_t>("18446744073709551615\xff", 20,
                                   std::numeric_limits<uint64_t>::max(),
                                   "18446744073709551615"));

  // Leading zeroes.
  EXPECT_TRUE(TestNumber<uint64_t>("000", 3, 0, "0"));
  EXPECT_TRUE(TestNumber<uint64_t>("001", 3, 1, "1"));
  EXPECT_TRUE(TestNumber<uint64_t>("0012", 4, 12, "12"));
  EXPECT_TRUE(TestNumber<uint64_t>("001234", 6, 1234, "1234"));
  EXPECT_TRUE(TestNumber<uint64_t>("0012345678", 10, 12345678, "12345678"));

  EXPECT_TRUE(TestNumber<uint64_t>("001234567890123456", 18, 1234567890123456,
                                   "1234567890123456"));

  EXPECT_TRUE(TestNumber<uint64_t>("0018446744073709551615", 22,
                                   std::numeric_limits<uint64_t>::max(),
                                   "18446744073709551615"));

  // Invalid characters.
  EXPECT_TRUE(TestNumber<uint64_t>(""));
  EXPECT_TRUE(TestNumber<uint64_t>("-1"));
  EXPECT_TRUE(TestNumber<uint64_t>("+1"));
  EXPECT_TRUE(TestNumber<uint64_t>("\0"));
  EXPECT_TRUE(TestNumber<uint64_t>(" "));
  EXPECT_TRUE(TestNumber<uint64_t>("+"));
  EXPECT_TRUE(TestNumber<uint64_t>("A"));
  EXPECT_TRUE(TestNumber<uint64_t>("."));

  // Overflow.
  EXPECT_TRUE(TestNumber<uint64_t>("18446744073709551616"));
}

TEST(NumberUtil, uint32_t) {
  // Regular values, and edge cases.
  EXPECT_TRUE(TestNumber<uint32_t>("0", 1, 0, "0"));
  EXPECT_TRUE(TestNumber<uint32_t>("1", 1, 1, "1"));
  EXPECT_TRUE(TestNumber<uint32_t>("12", 2, 12, "12"));
  EXPECT_TRUE(TestNumber<uint32_t>("1234", 4, 1234, "1234"));
  EXPECT_TRUE(TestNumber<uint32_t>("12345678", 8, 12345678, "12345678"));
  EXPECT_TRUE(TestNumber<uint32_t>("123456789", 9, 123456789, "123456789"));

  EXPECT_TRUE(TestNumber<uint32_t>(
      "4294967295", 10, std::numeric_limits<uint32_t>::max(), "4294967295"));

  // Trailing characters.
  EXPECT_TRUE(TestNumber<uint32_t>("0\0", 1, 0, "0"));
  EXPECT_TRUE(TestNumber<uint32_t>("1 ", 1, 1, "1"));
  EXPECT_TRUE(TestNumber<uint32_t>("12+", 2, 12, "12"));
  EXPECT_TRUE(TestNumber<uint32_t>("1234A", 4, 1234, "1234"));
  EXPECT_TRUE(TestNumber<uint32_t>("12345678.", 8, 12345678, "12345678"));
  EXPECT_TRUE(TestNumber<uint32_t>("123456789^", 9, 123456789, "123456789"));

  EXPECT_TRUE(TestNumber<uint32_t>("4294967295\xff", 10,
                                   std::numeric_limits<uint32_t>::max(),
                                   "4294967295"));

  // Leading zeroes.
  EXPECT_TRUE(TestNumber<uint32_t>("000", 3, 0, "0"));
  EXPECT_TRUE(TestNumber<uint32_t>("001", 3, 1, "1"));
  EXPECT_TRUE(TestNumber<uint32_t>("0012", 4, 12, "12"));
  EXPECT_TRUE(TestNumber<uint32_t>("001234", 6, 1234, "1234"));
  EXPECT_TRUE(TestNumber<uint32_t>("0012345678", 10, 12345678, "12345678"));
  EXPECT_TRUE(TestNumber<uint32_t>("00123456789", 11, 123456789, "123456789"));

  EXPECT_TRUE(TestNumber<uint32_t>(
      "004294967295", 12, std::numeric_limits<uint32_t>::max(), "4294967295"));

  // Invalid characters.
  EXPECT_TRUE(TestNumber<uint32_t>(""));
  EXPECT_TRUE(TestNumber<uint32_t>("-1"));
  EXPECT_TRUE(TestNumber<uint32_t>("+1"));
  EXPECT_TRUE(TestNumber<uint32_t>("\0"));
  EXPECT_TRUE(TestNumber<uint32_t>(" "));
  EXPECT_TRUE(TestNumber<uint32_t>("+"));
  EXPECT_TRUE(TestNumber<uint32_t>("A"));
  EXPECT_TRUE(TestNumber<uint32_t>("."));

  // Overflow.
  EXPECT_TRUE(TestNumber<uint32_t>("4294967296"));
}

TEST(NumberUtil, double) {
  ASSERT_EQ(kDoubleSignificandMaxInt,
            static_cast<int64_t>(kDoubleSignificandMax));

  // Regular values, and edge cases.
  EXPECT_TRUE(TestNumber<double>("0", 1, 0, "0"));
  EXPECT_TRUE(TestNumber<double>("1", 1, 1, "1"));
  EXPECT_TRUE(TestNumber<double>("-1", 2, -1, "-1"));
  EXPECT_TRUE(TestNumber<double>("1234", 4, 1234, "1234"));
  EXPECT_TRUE(TestNumber<double>("-12345678", 9, -12345678, "-12345678"));

  EXPECT_TRUE(TestNumber<double>("9007199254740991", 16, kDoubleSignificandMax,
                                 "9007199254740991"));

  EXPECT_TRUE(TestNumber<double>("-9007199254740991", 17,
                                 -kDoubleSignificandMax, "-9007199254740991"));

  // Trailing characters.
  EXPECT_TRUE(TestNumber<double>("0\0", 1, 0, "0"));
  EXPECT_TRUE(TestNumber<double>("1 ", 1, 1, "1"));
  EXPECT_TRUE(TestNumber<double>("-1+", 2, -1, "-1"));
  EXPECT_TRUE(TestNumber<double>("1234A", 4, 1234, "1234"));

  // Slash to delimit prefix as a period would actually match.
  EXPECT_TRUE(TestNumber<double>("-12345678/", 9, -12345678, "-12345678"));

  EXPECT_TRUE(TestNumber<double>("9007199254740991^", 16, kDoubleSignificandMax,
                                 "9007199254740991"));

  EXPECT_TRUE(TestNumber<double>("-9007199254740991\xff", 17,
                                 -kDoubleSignificandMax, "-9007199254740991"));

  // Leading zeroes.
  EXPECT_TRUE(TestNumber<double>("000", 3, 0, "0"));
  EXPECT_TRUE(TestNumber<double>("001", 3, 1, "1"));
  EXPECT_TRUE(TestNumber<double>("-001", 4, -1, "-1"));
  EXPECT_TRUE(TestNumber<double>("001234", 6, 1234, "1234"));
  EXPECT_TRUE(TestNumber<double>("-0012345678", 11, -12345678, "-12345678"));

  EXPECT_TRUE(TestNumber<double>("009007199254740991", 18,
                                 kDoubleSignificandMax, "9007199254740991"));

  EXPECT_TRUE(TestNumber<double>("-009007199254740991", 19,
                                 -kDoubleSignificandMax, "-9007199254740991"));

  // Decimal point.
  EXPECT_TRUE(TestNumber<double>(".", 1, 0, "0"));
  EXPECT_TRUE(TestNumber<double>(".1", 2, .1, "0.1"));
  EXPECT_TRUE(TestNumber<double>("-1.", 3, -1, "-1"));
  EXPECT_TRUE(TestNumber<double>("12.34", 5, 12.34, "12.34"));
  EXPECT_TRUE(TestNumber<double>("-1234.5678", 10, -1234.5678, "-1234.5678"));

  // DoubleToBuffer is not precise on the last digit; this test case works
  // on OS X, let's see if that holds for other architectures.
  EXPECT_TRUE(TestNumber<double>("90071992.54740991", 17,
                                 kDoubleSignificandMax * 1e-8,
                                 "90071992.5474099"));

  // Works on OS X.
  EXPECT_TRUE(TestNumber<double>("-90071992.54740991", 18,
                                 -kDoubleSignificandMax * 1e-8,
                                 "-90071992.5474099"));

  EXPECT_TRUE(TestNumber<double>("0e1", 3, 0, "0"));
  EXPECT_TRUE(TestNumber<double>("1e+1", 4, 10, "10"));
  EXPECT_TRUE(TestNumber<double>("-1e-1", 5, -0.1, "-0.1"));
  EXPECT_TRUE(TestNumber<double>("1234E0", 6, 1234, "1234"));

  EXPECT_TRUE(TestNumber<double>("-12345678E+123", 14, -12345678e123,
                                 "-1.2345678e+130"));

  EXPECT_TRUE(TestNumber<double>("9007199254740991E-123", 21,
                                 kDoubleSignificandMax * 1e-123,
                                 "9.007199254740991e-108"));

  // Works on OS X
  EXPECT_TRUE(TestNumber<double>("-9007199254740991E123", 21,
                                 -kDoubleSignificandMax * 1e123,
                                 "-9.00719925474099e+138"));

  // Positive and negative infinity.
  EXPECT_TRUE(TestNumber<double>(
      "inf", 3, std::numeric_limits<double>::infinity(), "inf"));

  EXPECT_TRUE(TestNumber<double>(
      "-inf", 4, -std::numeric_limits<double>::infinity(), "-inf"));

  EXPECT_TRUE(TestNumber<double>(
      "InF", 3, std::numeric_limits<double>::infinity(), "inf"));

  EXPECT_TRUE(TestNumber<double>(
      "-iNf", 4, -std::numeric_limits<double>::infinity(), "-inf"));

  EXPECT_TRUE(TestNumber<double>(
      "INFINITY", 8, std::numeric_limits<double>::infinity(), "inf"));

  EXPECT_TRUE(TestNumber<double>(
      "-INFINITY", 9, -std::numeric_limits<double>::infinity(), "-inf"));

  EXPECT_TRUE(TestNumber<double>(
      "InFINItY", 8, std::numeric_limits<double>::infinity(), "inf"));

  EXPECT_TRUE(TestNumber<double>(
      "-INfINiTY", 9, -std::numeric_limits<double>::infinity(), "-inf"));

  EXPECT_TRUE(TestNumber<double>(
      "infin", 3, std::numeric_limits<double>::infinity(), "inf"));

  EXPECT_TRUE(TestNumber<double>(
      "-infinit", 4, -std::numeric_limits<double>::infinity(), "-inf"));

  EXPECT_TRUE(TestNumber<double>(
      "infinityx", 8, std::numeric_limits<double>::infinity(), "inf"));

  EXPECT_TRUE(TestNumber<double>(
      "-infinity0", 9, -std::numeric_limits<double>::infinity(), "-inf"));

  // Explicit NaN.
  EXPECT_TRUE(TestNumber<double>(
      "nan", 3, std::numeric_limits<double>::quiet_NaN(), "nan"));

  EXPECT_TRUE(TestNumber<double>(
      "NaN", 3, std::numeric_limits<double>::quiet_NaN(), "nan"));

  EXPECT_TRUE(TestNumber<double>(
      "nanx", 3, std::numeric_limits<double>::quiet_NaN(), "nan"));

  // Implicit NaN and number-too-small.
  EXPECT_TRUE(TestNumber<double>(
      "1e999", 5, std::numeric_limits<double>::infinity(), "inf"));

  EXPECT_TRUE(TestNumber<double>("1e-999", 6, 0, "0"));

  // Invalid characters.
  EXPECT_TRUE(TestNumber<double>(""));
  EXPECT_TRUE(TestNumber<double>("+1"));
  EXPECT_TRUE(TestNumber<double>("\0"));
  EXPECT_TRUE(TestNumber<double>(" "));
  EXPECT_TRUE(TestNumber<double>("+"));
  EXPECT_TRUE(TestNumber<double>("A"));
}

TEST(NumberUtil, float) {
  ASSERT_EQ(kFloatSignificandMaxInt,
            static_cast<int64_t>(kFloatSignificandMax));

  // Regular values, and edge cases.
  EXPECT_TRUE(TestNumber<float>("0", 1, 0, "0"));
  EXPECT_TRUE(TestNumber<float>("1", 1, 1, "1"));
  EXPECT_TRUE(TestNumber<float>("-1", 2, -1, "-1"));
  EXPECT_TRUE(TestNumber<float>("1234", 4, 1234, "1234"));
  EXPECT_TRUE(TestNumber<float>("-1234567", 8, -1234567, "-1234567"));

  EXPECT_TRUE(
      TestNumber<float>("16777215", 8, kFloatSignificandMax, "16777215"));

  EXPECT_TRUE(
      TestNumber<float>("-16777215", 9, -kFloatSignificandMax, "-16777215"));

  // Trailing characters.
  EXPECT_TRUE(TestNumber<float>("0\0", 1, 0, "0"));
  EXPECT_TRUE(TestNumber<float>("1 ", 1, 1, "1"));
  EXPECT_TRUE(TestNumber<float>("-1+", 2, -1, "-1"));
  EXPECT_TRUE(TestNumber<float>("1234A", 4, 1234, "1234"));

  // Slash to delimit prefix as a period would actually match.
  EXPECT_TRUE(TestNumber<float>("-1234567/", 8, -1234567, "-1234567"));

  EXPECT_TRUE(
      TestNumber<float>("16777215^", 8, kFloatSignificandMax, "16777215"));

  EXPECT_TRUE(TestNumber<float>("-16777215\xff", 9, -kFloatSignificandMax,
                                "-16777215"));

  // Leading zeroes.
  EXPECT_TRUE(TestNumber<float>("000", 3, 0, "0"));
  EXPECT_TRUE(TestNumber<float>("001", 3, 1, "1"));
  EXPECT_TRUE(TestNumber<float>("-001", 4, -1, "-1"));
  EXPECT_TRUE(TestNumber<float>("001234", 6, 1234, "1234"));
  EXPECT_TRUE(TestNumber<float>("-001234567", 10, -1234567, "-1234567"));

  EXPECT_TRUE(
      TestNumber<float>("0016777215", 10, kFloatSignificandMax, "16777215"));

  EXPECT_TRUE(
      TestNumber<float>("-0016777215", 11, -kFloatSignificandMax, "-16777215"));

  // Decimal point.
  EXPECT_TRUE(TestNumber<float>(".", 1, 0, "0"));
  EXPECT_TRUE(TestNumber<float>(".1", 2, .1, "0.1"));
  EXPECT_TRUE(TestNumber<float>("-1.", 3, -1, "-1"));
  EXPECT_TRUE(TestNumber<float>("12.34", 5, 12.34, "12.34"));
  EXPECT_TRUE(TestNumber<float>("-1234.567", 9, -1234.567, "-1234.567"));

  // DoubleToBuffer is not precise on the last digit; this test case works
  // on OS X, let's see if that holds for other architectures.
  EXPECT_TRUE(TestNumber<float>("1677.7215", 9, 1677.7215f, "1677.7216"));

  // Works on OS X.
  EXPECT_TRUE(TestNumber<float>("-1677.7215", 10, -1677.7215f, "-1677.7216"));

  EXPECT_TRUE(TestNumber<float>("0e1", 3, 0, "0"));
  EXPECT_TRUE(TestNumber<float>("1e+1", 4, 10, "10"));
  EXPECT_TRUE(TestNumber<float>("-1e-1", 5, -0.1, "-0.1"));
  EXPECT_TRUE(TestNumber<float>("1234E0", 6, 1234, "1234"));

  // Works on OS X.
  EXPECT_TRUE(
      TestNumber<float>("-1234567E+30", 12, -1234567e30, "-1.234567e+36"));

  EXPECT_TRUE(
      TestNumber<float>("16777215E-30", 12, 16777215E-30f, "1.6777214e-23"));

  EXPECT_TRUE(
      TestNumber<float>("-16777215E30", 12, -16777215E30f, "-1.6777215e+37"));

  // Positive and negative infinity.
  EXPECT_TRUE(TestNumber<float>("inf", 3,
                                std::numeric_limits<float>::infinity(), "inf"));

  EXPECT_TRUE(TestNumber<float>(
      "-inf", 4, -std::numeric_limits<float>::infinity(), "-inf"));

  EXPECT_TRUE(TestNumber<float>("InF", 3,
                                std::numeric_limits<float>::infinity(), "inf"));

  EXPECT_TRUE(TestNumber<float>(
      "-iNf", 4, -std::numeric_limits<float>::infinity(), "-inf"));

  EXPECT_TRUE(TestNumber<float>("INFINITY", 8,
                                std::numeric_limits<float>::infinity(), "inf"));

  EXPECT_TRUE(TestNumber<float>(
      "-INFINITY", 9, -std::numeric_limits<float>::infinity(), "-inf"));

  EXPECT_TRUE(TestNumber<float>("InFINItY", 8,
                                std::numeric_limits<float>::infinity(), "inf"));

  EXPECT_TRUE(TestNumber<float>(
      "-INfINiTY", 9, -std::numeric_limits<float>::infinity(), "-inf"));

  EXPECT_TRUE(TestNumber<float>("infin", 3,
                                std::numeric_limits<float>::infinity(), "inf"));

  EXPECT_TRUE(TestNumber<float>(
      "-infinit", 4, -std::numeric_limits<float>::infinity(), "-inf"));

  EXPECT_TRUE(TestNumber<float>("infinityx", 8,
                                std::numeric_limits<float>::infinity(), "inf"));

  EXPECT_TRUE(TestNumber<float>(
      "-infinity0", 9, -std::numeric_limits<float>::infinity(), "-inf"));

  // Explicit NaN.
  EXPECT_TRUE(TestNumber<float>(
      "nan", 3, std::numeric_limits<float>::quiet_NaN(), "nan"));

  EXPECT_TRUE(TestNumber<float>(
      "NaN", 3, std::numeric_limits<float>::quiet_NaN(), "nan"));

  EXPECT_TRUE(TestNumber<float>(
      "nanx", 3, std::numeric_limits<float>::quiet_NaN(), "nan"));

  // Implicit NaN and number-too-small.
  EXPECT_TRUE(TestNumber<float>("1e999", 5,
                                std::numeric_limits<float>::infinity(), "inf"));

  EXPECT_TRUE(TestNumber<float>("1e-999", 6, 0, "0"));

  // Invalid characters.
  EXPECT_TRUE(TestNumber<float>(""));
  EXPECT_TRUE(TestNumber<float>("+1"));
  EXPECT_TRUE(TestNumber<float>("\0"));
  EXPECT_TRUE(TestNumber<float>(" "));
  EXPECT_TRUE(TestNumber<float>("+"));
  EXPECT_TRUE(TestNumber<float>("A"));
}

}  // namespace pjcore
