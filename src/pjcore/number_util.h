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

#ifndef PJCORE_NUMBER_UTIL_H_
#define PJCORE_NUMBER_UTIL_H_

#include <limits>
#include <string>

#include "pjcore/third_party/chromium/string_piece.h"

#include "pjcore/logging.h"

namespace pjcore {

/**
 * The goal of this header file is to provide namespace-level functions
 * ReadNumberPrefix, WriteNumberToBuffer, ReadNumber and WriteNumber that would
 * work for all numeric types. To achieve that, first, explicit overloads are
 * provided for types int64_t, uint64_t, double and float, and then template
 * functions defined for the rest of signed and unsigned integer types.
 * For float, the same functions can be used for reading as for double with a
 * subsequent static_cast, but for writing, it's more appopriate to have
 * explicit specialization as static_cast'ing it to double and using double's
 * precision potentially introduces "noise" digits to the output.
 */

/**
 * Reads a numeric prefix of the given string and returns its length, or zero if
 * the string does not begin with a signed integer, or that number does not fit
 * in the specified value type.
 * @param str string with a prefix of a signed integer
 * @param value non-null pointer to the output value
 * @return length of the numeric prefix, or 0
 */
size_t ReadNumberPrefix(StringPiece str, int64_t* value);

size_t ReadNumberPrefix(StringPiece str, uint64_t* value);

size_t ReadNumberPrefix(StringPiece str, double* value);

size_t ReadNumberPrefix(StringPiece str, float* value);

size_t ReadNumberPrefix(StringPiece str, bool* value);

/**
 * Struct with a character buffer sufficient to write any signed or unsigned
 * integer or a floating-point number, followed by a null character.
 */
struct WriteNumberBuffer {
  /** Character buffer of sufficient size. */
  char buffer[33];
};

/**
 * Writes a number into a buffer and returns a StringPiece with the result in
 * the buffer. Buffer contains null characer right after the result.
 * @param value number to write
 * @param buffer buffer sufficient to write any number
 * @return StringPiece with the result in the buffer
 */
StringPiece WriteNumberToBuffer(int64_t value, WriteNumberBuffer* buffer);

StringPiece WriteNumberToBuffer(uint64_t value, WriteNumberBuffer* buffer,
                                size_t width = 0);

StringPiece WriteNumberToBuffer(double value, WriteNumberBuffer* buffer);

StringPiece WriteNumberToBuffer(float value, WriteNumberBuffer* buffer);

StringPiece WriteNumberToBuffer(bool value, WriteNumberBuffer* buffer);

/**
 * Template struct that automatically resolves whether the specified integer
 * type is signed or not, and provides appropriate ReadPrefix and WriteToNumber
 * static functions that use overloads for int64_t or uint64_t and check for
 * overflow.
 */
template <typename Integer,
          bool kIsSigned = std::numeric_limits<Integer>::is_signed>
struct IntegerTraits;

/** Specialization of IntegerTraits for signed integers like int32_t. */
template <typename Integer>
struct IntegerTraits<Integer, true> {
  static size_t ReadPrefix(StringPiece str, Integer* value) {
    PJCORE_CHECK(value);
    *value = 0;

    int64_t signed_value;

    size_t prefix_length = ReadNumberPrefix(str, &signed_value);

    if (!prefix_length) {
      return 0;
    }

    *value = static_cast<Integer>(signed_value);

    if (static_cast<int64_t>(*value) != signed_value) {
      // Overflow.
      *value = 0;
      return 0;
    }

    return prefix_length;
  }

  static StringPiece WriteToBuffer(Integer value, WriteNumberBuffer* buffer) {
    return WriteNumberToBuffer(static_cast<int64_t>(value), buffer);
  }
};

/** Specialization of IntegerTraits for unsigned integers like int32_t. */
template <typename Integer>
struct IntegerTraits<Integer, false> {
  static size_t ReadPrefix(StringPiece str, Integer* value) {
    PJCORE_CHECK(value);
    *value = 0;

    uint64_t unsigned_value;

    size_t prefix_length = ReadNumberPrefix(str, &unsigned_value);

    if (!prefix_length) {
      return 0;
    }

    *value = static_cast<Integer>(unsigned_value);

    if (static_cast<uint64_t>(*value) != unsigned_value) {
      // Overflow.
      *value = 0;
      return 0;
    }

    return prefix_length;
  }

  static StringPiece WriteToBuffer(Integer value, WriteNumberBuffer* buffer) {
    return WriteNumberToBuffer(static_cast<uint64_t>(value), buffer);
  }
};

template <typename Integer>
size_t ReadNumberPrefix(StringPiece str, Integer* value) {
  return IntegerTraits<Integer>::ReadPrefix(str, value);
}

template <typename Integer>
StringPiece WriteNumberToBuffer(Integer value, WriteNumberBuffer* buffer) {
  return IntegerTraits<Integer>::WriteToBuffer(value, buffer);
}

/**
 * Reads a number from the given string.
 * @param str string with a number
 * @param value non-null pointer to the output value
 * @return true on success
 */
template <typename Number>
bool ReadNumber(StringPiece str, Number* value) {
  return !str.empty() && (ReadNumberPrefix(str, value) == str.length());
}

/**
 * Returns a string with written number.
 * @param value number to write
 * @return string with the written number
 */
template <typename Number>
std::string WriteNumber(Number value) {
  WriteNumberBuffer buffer;
  return WriteNumberToBuffer(value, &buffer).as_string();
}

template <typename Number>
void AppendNumber(Number value, std::string* str) {
  PJCORE_CHECK(str);

  WriteNumberBuffer buffer;
  WriteNumberToBuffer(value, &buffer).AppendToString(str);
}

std::string WriteNumber(uint64_t value, size_t width);

void AppendNumber(uint64_t value, std::string* str, size_t width);

}  // namespace pjcore

#endif  // PJCORE_NUMBER_UTIL_H_
