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

#ifndef PJCORE_STRING_PIECE_UTIL_H_
#define PJCORE_STRING_PIECE_UTIL_H_

#include <assert.h>

#include <limits>
#include <string>

#include "pjcore/third_party/chromium/string_piece.h"

#include "pjcore/logging.h"

/**
 * Helper functions for StringPiece manipulation.
 */
namespace pjcore {

/**
 * memmove wrapper suitable for StringPiece's. Copies content of source
 * StringPiece to the memory pointed by destination pointer.
 * memmove is safer to use than memcpy as it doesn't assume that the
 * regions do not overlap.
 * @param destination pointer to the destination memory
 * @param source StringPiece with source memory
 * @return destination pointer
 */
StringPiece MemMove(void* destination, StringPiece source);

/**
 * Macro that defines a functor that checks if a given character meets a certain
 * condition. Functor's single argument type is char, return type is bool. That
 * struct could've inherited std::unary_function, but the latter has been
 * deprecated in C++11.
 * Still, the struct typedef's argument_type and result_type, to allow it being
 * chained with other functors, like std::not1.
 */
#define PJCORE_CHAR_PREDICATE(_NAME, _CONDITION)               \
  struct _NAME {                                               \
    typedef char argument_type;                                \
    typedef bool result_type;                                  \
    inline static bool eval(char ch) { return _CONDITION; }    \
    inline bool operator()(char ch) const { return eval(ch); } \
  }

/** Functor that tells if a character is a whitespace. */
PJCORE_CHAR_PREDICATE(IsWhitespace, ch == ' ' || ch == '\t' || ch == '\n' ||
                                        ch == '\v' || ch == '\f' || ch == '\r');

/** Functor that tells if a character is a decimal digit. */
PJCORE_CHAR_PREDICATE(IsDigit, ch >= '0' && ch <= '9');

/** Functor that tells if a character is a hex digit. */
PJCORE_CHAR_PREDICATE(IsHexDigit, (ch >= '0' && ch <= '9') ||
                                      (ch >= 'a' && ch <= 'f') ||
                                      (ch >= 'A' && ch <= 'F'));

PJCORE_CHAR_PREDICATE(IsBase64Digit,
                      ch == '+' || ch == '/' || (ch >= '0' && ch <= '9') ||
                          (ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z'));

/** Functor that tells if a character is a newline. */
PJCORE_CHAR_PREDICATE(IsNewline, ch == '\n');

/** Functor that tells if a single-byte UTF-8 Unicode character is control. */
PJCORE_CHAR_PREDICATE(IsUnicodeControl, static_cast<uint8_t>(ch) < 32);

/**
 * Functor that tells if a character is ASCII character but neither a space nor
 * a control character.
 */
PJCORE_CHAR_PREDICATE(IsAsciiNotSpaceNotControl, ch > 0x20 && ch < 0x7f);

PJCORE_CHAR_PREDICATE(IsUriMark, ch == '-' || ch == '_' || ch == '.' ||
                                     ch == '!' || ch == '~' || ch == '*' ||
                                     ch == '\'' || ch == '(' || ch == ')');

PJCORE_CHAR_PREDICATE(IsUriAlpha,
                      (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z'));

PJCORE_CHAR_PREDICATE(IsUriUnescaped, IsUriAlpha::eval(ch) ||
                                          IsDigit::eval(ch) ||
                                          IsUriMark::eval(ch));

PJCORE_CHAR_PREDICATE(IsUriReserved, ch == ';' || ch == '/' || ch == '?' ||
                                         ch == ':' || ch == '@' || ch == '&' ||
                                         ch == '=' || ch == '+' || ch == '$' ||
                                         ch == ',');

PJCORE_CHAR_PREDICATE(AnyChar, true);

PJCORE_CHAR_PREDICATE(NoChar, false);

/**
 * Tells the length of the longest prefix of a string such that its individual
 * characters match the given predicate.
 * @param str string to match
 * @param predicate predicate to use
 * @return length of the longest prefix with matching characters
 */
template <typename Predicate>
size_t MatchingPrefixLength(StringPiece str, Predicate predicate) {
  size_t prefix_length = 0;
  while (prefix_length < str.size() && predicate(str[prefix_length])) {
    ++prefix_length;
  }
  return prefix_length;
}

inline char WriteDigit(uint8_t digit) {
  assert(digit < 10);

  return '0' + static_cast<char>(digit);
}

inline uint8_t ReadDigit(char ch) {
  assert(IsDigit::eval(ch));

  return static_cast<uint8_t>(ch - '0');
}

inline char WriteHexDigit(uint8_t digit) {
  assert(digit < 16);

  static const char* kHexDigits = "0123456789abcdef";

  return kHexDigits[digit];
}

inline uint8_t ReadHexDigit(char ch) {
  assert(IsHexDigit::eval(ch));

  if (ch < 'A') {
    return static_cast<uint8_t>(ch - '0');
  } else {
    return (static_cast<uint8_t>(ch - 'A') & 0xf) + 10;
  }
}

inline char WriteBase64Digit(uint8_t digit) {
  assert(digit < 64);

  static const char* kBase64Digits =
      "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

  return kBase64Digits[digit];
}

inline uint8_t ReadBase64Digit(char ch) {
  assert(IsBase64Digit::eval(ch));

  if (ch == '+') {
    return 62;
  } else if (ch == '/') {
    return 63;
  } else if (ch <= '9') {
    return static_cast<uint8_t>(ch - '0') + 52;
  } else if (ch <= 'Z') {
    return static_cast<uint8_t>(ch - 'A');
  } else {
    return static_cast<uint8_t>(ch - 'a') + 26;
  }
}

bool WriteHexBlob(StringPiece binary, void* hex_blob, size_t hex_blob_size);

std::string WriteHexBlob(StringPiece binary);

bool ReadHexBlob(StringPiece hex_blob, void* binary, size_t binary_size,
                 Error* error);

bool ReadHexBlob(StringPiece base_64, std::string* binary, Error* error);

std::string WriteBase64(StringPiece binary);

bool ReadBase64(StringPiece base_64, std::string* binary, Error* error);

}  // namespace pjcore

#endif  // PJCORE_STRING_PIECE_UTIL_H_
