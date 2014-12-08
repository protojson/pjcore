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

#include "pjcore/number_util.h"

#include <limits>
#include <string>

#include "google/protobuf/stubs/strutil.h"

#include "pjcore/string_piece_util.h"

namespace pjcore {

size_t ReadNumberPrefix(StringPiece str, int64_t* value) {
  PJCORE_CHECK(value);
  *value = 0;

  size_t offset = 0;

  if (offset >= str.length()) {
    return 0;
  }

  bool negative = false;
  if (str[offset] == '-') {
    negative = true;
    if (offset >= str.length()) {
      return 0;
    }
    ++offset;
  }

  uint64_t unsigned_value;

  size_t unsigned_prefix_length =
      ReadNumberPrefix(str.substr(offset), &unsigned_value);
  if (!unsigned_prefix_length) {
    return 0;
  }

  if (negative) {
    if (unsigned_value >
        static_cast<uint64_t>(std::numeric_limits<int64_t>::max()) + 1) {
      return 0;
    }
    *value = static_cast<int64_t>(-unsigned_value);
  } else {
    if (unsigned_value >
        static_cast<uint64_t>(std::numeric_limits<int64_t>::max())) {
      return 0;
    }
    *value = static_cast<uint64_t>(unsigned_value);
  }

  return offset + unsigned_prefix_length;
}

size_t ReadNumberPrefix(StringPiece str, uint64_t* value) {
  PJCORE_CHECK(value);
  *value = 0;

  uint64_t result = 0;

  size_t offset = 0;

  while (offset < str.length() && IsDigit::eval(str[offset])) {
    uint8_t digit = static_cast<uint8_t>(str[offset] - '0');
    if (result > (std::numeric_limits<uint64_t>::max() - digit) / 10) {
      return 0;
    }

    result = result * 10 + digit;
    ++offset;
  }

  *value = result;
  return offset;
}

size_t ReadNumberPrefix(StringPiece str, double* value) {
  PJCORE_CHECK(value);
  *value = 0;

  size_t offset = 0;

  if (str.length() - offset >= 3 &&
      (str[offset] == 'N' || str[offset] == 'n') &&
      (str[offset + 1] == 'A' || str[offset + 1] == 'a') &&
      (str[offset + 2] == 'N' || str[offset + 2] == 'n')) {
    *value = std::numeric_limits<double>::quiet_NaN();
    offset += 3;
  } else {
    bool negative = false;
    if (offset < str.length() && str[offset] == '-') {
      negative = true;
      ++offset;
    }

    if (str.length() - offset >= 3 &&
        (str[offset] == 'I' || str[offset] == 'i') &&
        (str[offset + 1] == 'N' || str[offset + 1] == 'n') &&
        (str[offset + 2] == 'F' || str[offset + 2] == 'f')) {
      *value = negative ? -std::numeric_limits<double>::infinity()
                        : std::numeric_limits<double>::infinity();
      offset += 3;

      if (str.length() - offset >= 5 &&
          (str[offset] == 'I' || str[offset] == 'i') &&
          (str[offset + 1] == 'N' || str[offset + 1] == 'n') &&
          (str[offset + 2] == 'I' || str[offset + 2] == 'i') &&
          (str[offset + 3] == 'T' || str[offset + 3] == 't') &&
          (str[offset + 4] == 'Y' || str[offset + 4] == 'y')) {
        offset += 5;
      }
    } else {
      // Parse signifcand and exponent as integers, then preprocess the matching
      // prefix into a form with
      //   a) known limited length - so that it can be re-allocated with a null
      //        character at the end,
      //   b) without decimal point - so that strtod doesn't look for
      //        locale-specific one,
      // and then pass it to stdlib's strtod.
      uint64_t significand = 0;
      int64_t exponent = 0;

      size_t digits_begin = offset;

      while (offset < str.length() && IsDigit::eval(str[offset])) {
        uint8_t digit = static_cast<uint8_t>(str[offset] - '0');
        if (significand > (std::numeric_limits<uint64_t>::max() - digit) / 10) {
          // Digits do not fit, and therefore do not matter for
          // significand, but still matter for exponent.
          size_t extra_digits_for_exponent =
              MatchingPrefixLength(str.substr(offset + 1), IsDigit());

          offset += extra_digits_for_exponent + 1;
          exponent += extra_digits_for_exponent + 1;

          break;
        }
        significand = significand * 10 + digit;
        ++offset;
      }

      if (offset < str.length() && str[offset] == '.') {
        ++offset;

        while (offset < str.length() && IsDigit::eval(str[offset])) {
          uint8_t digit = static_cast<uint8_t>(str[offset] - '0');
          if (significand >
              (std::numeric_limits<uint64_t>::max() - digit) / 10) {
            // Digits do not fit, and therefore do not matter.
            offset +=
                MatchingPrefixLength(str.substr(offset + 1), IsDigit()) + 1;
            break;
          }
          significand = significand * 10 + digit;
          ++offset;
          --exponent;
        }
      }

      if (digits_begin == offset) {
        // No digits or period.
        return 0;
      }

      if (offset < str.length() && (str[offset] == 'E' || str[offset] == 'e')) {
        ++offset;

        bool negative_exponent = false;
        if (offset < str.length() &&
            (str[offset] == '-' || str[offset] == '+')) {
          negative_exponent = (str[offset] == '-');
          ++offset;
        }

        int64_t explicit_exponent = 0;

        if (offset >= str.length() || !IsDigit::eval(str[offset])) {
          // Exponent expected.
          return 0;
        }

        do {
          uint8_t digit = static_cast<uint8_t>(str[offset] - '0');
          if (explicit_exponent >
              (std::numeric_limits<int64_t>::max() - digit) / 10) {
            // Exponent too large already.
            offset +=
                MatchingPrefixLength(str.substr(offset + 1), IsDigit()) + 1;

            break;
          }
          explicit_exponent = explicit_exponent * 10 + digit;

          ++offset;
        } while (offset < str.length() && IsDigit::eval(str[offset]));

        if (negative_exponent) {
          if (exponent >= 0 ||
              explicit_exponent <=
                  std::numeric_limits<int64_t>::max() + exponent + 1) {
            exponent -= explicit_exponent;
          } else {
            exponent = std::numeric_limits<int64_t>::min();
          }
        } else {
          if (exponent <= 0 ||
              explicit_exponent <=
                  std::numeric_limits<int64_t>::max() - exponent) {
            exponent += explicit_exponent;
          } else {
            exponent = std::numeric_limits<int64_t>::max();
          }
        }
      }

      char preprocessed_buffer[64] = {0};

      size_t preprocessed_offset = 0;

      if (negative) {
        preprocessed_buffer[preprocessed_offset++] = '-';
      }

      WriteNumberBuffer significand_buffer;
      StringPiece significand_str =
          WriteNumberToBuffer(significand, &significand_buffer);

      MemMove(preprocessed_buffer + preprocessed_offset, significand_str);

      preprocessed_offset += significand_str.length();

      preprocessed_buffer[preprocessed_offset++] = 'e';

      WriteNumberBuffer exponent_buffer;
      StringPiece exponent_str =
          WriteNumberToBuffer(exponent, &exponent_buffer);

      MemMove(preprocessed_buffer + preprocessed_offset, exponent_str);

      preprocessed_offset += exponent_str.length();

      preprocessed_buffer[preprocessed_offset] = '\0';

      char* endptr;
      *value = strtod(preprocessed_buffer, &endptr);

      if (endptr != preprocessed_buffer + preprocessed_offset) {
        // strtod got confused.
        return 0;
      }
    }
  }

  return offset;
}

size_t ReadNumberPrefix(StringPiece str, float* value) {
  PJCORE_CHECK(value);
  *value = 0;

  double double_value;
  size_t prefix_length = ReadNumberPrefix(str, &double_value);

  if (prefix_length) {
    *value = static_cast<float>(double_value);
  }

  return prefix_length;
}

StringPiece WriteNumberToBuffer(int64_t value, WriteNumberBuffer* buffer) {
  PJCORE_CHECK(buffer);
  if (value >= 0) {
    return WriteNumberToBuffer(static_cast<uint64_t>(value), buffer);
  } else {
    WriteNumberBuffer unsigned_buffer;
    StringPiece unsigned_str =
        WriteNumberToBuffer(-static_cast<uint64_t>(value), &unsigned_buffer);
    buffer->buffer[0] = '-';
    MemMove(buffer->buffer + 1, unsigned_str);
    buffer->buffer[unsigned_str.length() + 1] = '\0';
    return StringPiece(buffer->buffer, unsigned_str.length() + 1);
  }
}

size_t ReadNumberPrefix(StringPiece str, bool* value) {
  PJCORE_CHECK(value);
  *value = false;

  if (str.size() >= 4 && (str[0] == 'T' || str[0] == 't') &&
      (str[1] == 'R' || str[1] == 'r') && (str[2] == 'U' || str[2] == 'u') &&
      (str[3] == 'E' || str[3] == 'e')) {
    *value = true;
    return 4;
  }

  if (str.size() >= 5 && (str[0] == 'F' || str[0] == 'f') &&
      (str[1] == 'A' || str[1] == 'a') && (str[2] == 'L' || str[2] == 'l') &&
      (str[3] == 'S' || str[3] == 's') && (str[4] == 'E' || str[4] == 'e')) {
    *value = false;
    return 5;
  }

  return 0;
}

StringPiece WriteNumberToBuffer(uint64_t value, WriteNumberBuffer* buffer,
                                size_t width) {
  PJCORE_CHECK(buffer);

  size_t digit_count = 1;
  uint64_t copy = value;
  while (copy >= 10) {
    copy /= 10;
    ++digit_count;
  }

  if (digit_count < width) {
    digit_count = width;
  }

  if (digit_count >= sizeof(buffer->buffer)) {
    digit_count = sizeof(buffer->buffer) - 1;
  }

  copy = value;
  for (size_t digit = digit_count; digit > 0; --digit) {
    buffer->buffer[digit - 1] = '0' + static_cast<char>(value % 10);
    value /= 10;
  }

  buffer->buffer[digit_count] = '\0';

  return StringPiece(buffer->buffer, digit_count);
}

StringPiece WriteNumberToBuffer(double value, WriteNumberBuffer* buffer) {
  PJCORE_CHECK(buffer);

  char* result = google::protobuf::DoubleToBuffer(value, buffer->buffer);

  PJCORE_CHECK_EQ(result, buffer->buffer);

  return StringPiece(result);
}

StringPiece WriteNumberToBuffer(float value, WriteNumberBuffer* buffer) {
  PJCORE_CHECK(buffer);

  char* result = google::protobuf::FloatToBuffer(value, buffer->buffer);

  PJCORE_CHECK_EQ(result, buffer->buffer);

#ifdef _WIN32
  // FloatToBuffer may prepend 0 to exponent on Windows
  char* exp = strchr(result, 'e');
  if (exp) {
    if (exp[1] == '-' || exp[1] == '+') {
      if (exp[2] == '0' && exp[3] != '\0') {
        char *p = exp + 2;
        do {
          *p = *(p + 1);
	  ++p;
	} while (*p);
      }
    }
  }
#endif

  return StringPiece(result);
}

StringPiece WriteNumberToBuffer(bool value, WriteNumberBuffer* buffer) {
  PJCORE_CHECK(buffer);

  if (value) {
    memcpy(buffer->buffer, "true", 5);
    return StringPiece(buffer->buffer, 4);
  } else {
    memcpy(buffer->buffer, "false", 6);
    return StringPiece(buffer->buffer, 5);
  }
}

std::string WriteNumber(uint64_t value, size_t width) {
  WriteNumberBuffer buffer;
  return WriteNumberToBuffer(value, &buffer, width).as_string();
}

void AppendNumber(uint64_t value, std::string* str, size_t width) {
  WriteNumberBuffer buffer;
  WriteNumberToBuffer(value, &buffer, width).AppendToString(str);
}

}  // namespace pjcore
