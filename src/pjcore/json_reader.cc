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

#include "pjcore/json_reader.h"

#include <functional>
#include <limits>
#include <stack>
#include <string>

#include "pjcore/logging.h"
#include "pjcore/json_writer.h"
#include "pjcore/name_value_util.h"
#include "pjcore/number_util.h"
#include "pjcore/repeated_field_util.h"
#include "pjcore/text_location.h"
#include "pjcore/string_piece_util.h"
#include "pjcore/unicode.h"

#ifdef min
#undef min
#endif

#ifdef max
#undef max
#endif

namespace pjcore {

namespace {

class Context {
 public:
  Context(const JsonReaderConfig& config, StringPiece str, JsonValue* value)
      : config_(config),
        location_(MakeTextLocation(0, 1, 1)),
        remaining_(str),
        value_(value),
        error(NULL) {
    PJCORE_CHECK(value_);
    value->Clear();
  }

  bool Complete(Error* input_error) {
    PJCORE_CHECK(input_error);
    input_error->Clear();

    error = input_error;

    if (!InternalComplete()) {
      *error->mutable_text_location() = location_;
      PJCORE_FAIL_SILENT("Failed to complete reading");
    }

    return true;
  }

 private:
  void AdvanceOne() {
    assert(!remaining_.empty());
    AdvanceTextLocation(&location_, remaining_[0]);
    remaining_.remove_prefix(1);
  }

  void Advance(size_t count) {
    assert(count <= remaining_.length());
    AdvanceTextLocation(&location_, remaining_.substr(0, count));
    remaining_.remove_prefix(count);
  }

  bool ReadHexDigit(uint32_t* hex_digit) {
    assert(hex_digit);

    PJCORE_REQUIRE(!remaining_.empty(), "Hex digit expected");

    PJCORE_REQUIRE(IsHexDigit::eval(remaining_[0]), "Hex digit expected");

    *hex_digit = pjcore::ReadHexDigit(remaining_[0]);

    AdvanceOne();

    return true;
  }

  bool ReadHexDigits(size_t count, uint32_t* value) {
    assert(value);

    *value = 0;

    uint32_t result = 0;

    while (count--) {
      uint32_t hex_digit;
      PJCORE_REQUIRE_SILENT(ReadHexDigit(&hex_digit), "Invalid hex digit");
      result = (result << 4) | hex_digit;
    }

    *value = result;
    return true;
  }

  bool ReadChar(char expected) {
    PJCORE_REQUIRE(!remaining_.empty() && remaining_[0] == expected,
                   std::string("Character '") + expected + "' expected");
    AdvanceOne();
    return true;
  }

  void ReadWhitespace() {
    Advance(MatchingPrefixLength(remaining_, IsWhitespace()));
  }

  bool ReadComment() {
    if (remaining_[0] == '#' || remaining_.starts_with("//")) {
      // Read the rest of the line after # or //.
      size_t remaining_line_length =
          MatchingPrefixLength(remaining_, std::not1(IsNewline()));

      // Read the newline character, too.
      if (remaining_line_length < remaining_.length()) {
        ++remaining_line_length;
      }

      Advance(remaining_line_length);
      return true;
    }

    PJCORE_REQUIRE(remaining_.starts_with("/*"),
                   "Comment beginning with // or /* expected");

    size_t offset = 2;

    for (;;) {
      PJCORE_REQUIRE(offset < remaining_.length(),
                     "Unterminated multi-line comment");
      if (remaining_[offset++] == '*') {
        PJCORE_REQUIRE(offset < remaining_.length(),
                       "Unterminated multi-line comment");
        if (remaining_[offset] == '/') {
          Advance(offset + 1);
          return true;
        }
      }
    }
  }

  bool ReadWhitespaceAndComments() {
    ReadWhitespace();

    if (config_.disallow_comments()) {
      return true;
    }

    while (!remaining_.empty() &&
           (remaining_[0] == '/' || remaining_[0] == '#')) {
      PJCORE_REQUIRE(ReadComment(), "Invalid comment");
      ReadWhitespace();
    }

    return true;
  }

  bool ReadString(std::string* str) {
    assert(str);

    str->clear();

    TextLocation begin_location = location_;

    PJCORE_REQUIRE(ReadChar('"'), "Invalid string");

    for (;;) {
      PJCORE_REQUIRE(!remaining_.empty(), "Unterminated string");

      switch (remaining_[0]) {
        case '"': {
          AdvanceOne();

          if (!Unicode::IsStructurallyValidUtf8(*str)) {
            location_ = begin_location;
            PJCORE_FAIL("Structurally invalid Unicode string");
          }

          return true;
        }

        case '\n':
          PJCORE_REQUIRE(config_.allow_control_characters(),
                         "String ending with double quotes expected");
          str->push_back(remaining_[0]);
          AdvanceOne();
          break;

        case '\\':
          AdvanceOne();

          PJCORE_REQUIRE(!remaining_.empty(), "Unterminated escape sequence");

          switch (remaining_[0]) {
            case '"':
            case '\\':
            case '/':
              str->push_back(remaining_[0]);
              AdvanceOne();
              break;

            case 'b':
              str->push_back('\b');
              AdvanceOne();
              break;

            case 'f':
              str->push_back('\f');
              AdvanceOne();
              break;

            case 'n':
              str->push_back('\n');
              AdvanceOne();
              break;

            case 'r':
              str->push_back('\r');
              AdvanceOne();
              break;

            case 't':
              str->push_back('\t');
              AdvanceOne();
              break;

            case 'u': {
              AdvanceOne();
              uint32_t code_point;
              PJCORE_REQUIRE_SILENT(ReadHexDigits(4, &code_point),
                                    "Invalid hex sequence");

              if (Unicode::IsHighSurrogate(code_point)) {
                uint32_t high_surrogate = code_point;

                uint32_t low_surrogate;
                PJCORE_REQUIRE(ReadChar('\\') && ReadChar('u') &&
                                   ReadHexDigits(4, &low_surrogate),
                               "Invalid Unicode escape sequence");

                PJCORE_REQUIRE(Unicode::IsLowSurrogate(low_surrogate),
                               "Low-surrogate code point expected");

                PJCORE_REQUIRE(Unicode::DecodeSurrogatePair(
                                   high_surrogate, low_surrogate, &code_point),
                               "Invalid surrogate pair");
              }

              Unicode::WriteCodePointBuffer buffer;
              StringPiece code_point_str =
                  Unicode::WriteCodePointToBuffer(code_point, &buffer);

              PJCORE_REQUIRE(!code_point_str.empty(),
                             std::string("Invalid Unicode code point ") +
                                 WriteNumber(code_point));

              code_point_str.AppendToString(str);
              break;
            }

            default:
              PJCORE_FAIL("Invalid escape sequence");
          }
          break;

        default:
          PJCORE_REQUIRE(
              config_.allow_control_characters() ||
                  !IsUnicodeControl::eval(remaining_[0]),
              std::string("Invalid control character \\x") +
                  WriteNumber(static_cast<uint8_t>(remaining_[0]), 2));

          str->push_back(remaining_[0]);
          AdvanceOne();
          break;
      }
    }
  }

  JsonValue* target() { return targetStack_.top(); }

  bool InternalComplete();

  const JsonReaderConfig& config_;

  TextLocation location_;

  StringPiece remaining_;

  JsonValue* value_;

  Error* error;

  std::stack<JsonValue*> targetStack_;
};

bool Context::InternalComplete() {
  value_->Clear();

  targetStack_.push(value_);

  if (remaining_.starts_with(Unicode::ByteOrderMarkUtf8())) {
    Advance(Unicode::ByteOrderMarkUtf8().length());
  } else {
    PJCORE_REQUIRE(
        !remaining_.starts_with(Unicode::ByteOrderMarkUtf32BigEndian()),
        "Big-Endian UTF-32 not supported, only UTF-8");
    PJCORE_REQUIRE(
        !remaining_.starts_with(Unicode::ByteOrderMarkUtf32LittleEndian()),
        "Little-Endian UTF-32 not supported, only UTF-8");
    PJCORE_REQUIRE(
        !remaining_.starts_with(Unicode::ByteOrderMarkUtf16BigEndian()),
        "Big-Endian UTF-16 not supported, only UTF-8");
    PJCORE_REQUIRE(
        !remaining_.starts_with(Unicode::ByteOrderMarkUtf16LittleEndian()),
        "Little-Endian UTF-16 not supported, only UTF-8");
  }

  bool read_property_name = false;

  for (;;) {
    PJCORE_CHECK(!targetStack_.empty());

    PJCORE_REQUIRE_SILENT(ReadWhitespaceAndComments(),
                          "Failed to read whitespace and/or comments");

    PJCORE_REQUIRE(!remaining_.empty(), read_property_name
                                            ? "Property name expected"
                                            : "Value expected");

    if (read_property_name) {
      PJCORE_CHECK_EQ(JsonValue::TYPE_OBJECT, target()->type());

      read_property_name = false;

      if (remaining_[0] == '}') {
        if (!Empty(target()->object_properties())) {
          PJCORE_REQUIRE(!config_.disallow_trailing_commas(),
                         "Trailing commas disallowed");
        }
        AdvanceOne();
        if (!config_.properties_as_is()) {
          StableSortAndRemoveDuplicatesByName(
              target()->mutable_object_properties());
        }
      } else {
        JsonValue::Property* property = target()->add_object_properties();

        PJCORE_REQUIRE_SILENT(ReadString(property->mutable_name()),
                              "Failed to read property name");

        PJCORE_REQUIRE_SILENT(ReadWhitespaceAndComments(),
                              "Failed to read whitespace and/or comments");
        PJCORE_REQUIRE(ReadChar(':'), "Colon expected");

        targetStack_.push(property->mutable_value());
        continue;
      }
    } else if (remaining_[0] == ']') {
      targetStack_.pop();
      PJCORE_REQUIRE(
          !targetStack_.empty() && target()->type() == JsonValue::TYPE_ARRAY,
          "Not in a list of array items, unexpected bracket");
      PJCORE_CHECK(!Empty(target()->array_elements()));
      target()->mutable_array_elements()->RemoveLast();

      if (!Empty(target()->array_elements())) {
        PJCORE_REQUIRE(!config_.disallow_trailing_commas(),
                       "Trailing commas disallowed");
      }
      AdvanceOne();
    } else {
      if (remaining_[0] == '{') {
        AdvanceOne();
        target()->set_type(JsonValue::TYPE_OBJECT);
        read_property_name = true;
        continue;
      }

      if (remaining_[0] == '[') {
        AdvanceOne();
        target()->set_type(JsonValue::TYPE_ARRAY);
        targetStack_.push(target()->add_array_elements());
        continue;
      }

      if (remaining_[0] == '"') {
        target()->set_type(JsonValue::TYPE_STRING);
        PJCORE_REQUIRE_SILENT(ReadString(target()->mutable_string_value()),
                              "Failed to read string");
      } else if (remaining_[0] == 'n' && remaining_.starts_with("null")) {
        target()->set_type(JsonValue::TYPE_NULL);
        Advance(4);
      } else if (remaining_[0] == 't' && remaining_.starts_with("true")) {
        target()->set_type(JsonValue::TYPE_BOOL);
        target()->set_bool_value(true);
        Advance(4);
      } else if (remaining_[0] == 'f' && remaining_.starts_with("false")) {
        target()->set_type(JsonValue::TYPE_BOOL);
        target()->set_bool_value(false);
        Advance(5);
      } else if (remaining_.length() >= 3 &&
                 (remaining_[0] == 'N' || remaining_[0] == 'n') &&
                 (remaining_[1] == 'A' || remaining_[1] == 'a') &&
                 (remaining_[2] == 'N' || remaining_[2] == 'n')) {
        PJCORE_REQUIRE(!config_.disallow_nan_and_infinity(), "NaN disallowed");
        target()->set_type(JsonValue::TYPE_DOUBLE);
        target()->set_double_value(std::numeric_limits<double>::quiet_NaN());
        Advance(3);
      } else {
        size_t optional_minus = (remaining_[0] == '-') ? 1 : 0;
        if (remaining_.length() >= optional_minus + 3 &&
            (remaining_[optional_minus + 0] == 'I' ||
             remaining_[optional_minus + 0] == 'i') &&
            (remaining_[optional_minus + 1] == 'N' ||
             remaining_[optional_minus + 1] == 'n') &&
            (remaining_[optional_minus + 2] == 'F' ||
             remaining_[optional_minus + 2] == 'f')) {
          PJCORE_REQUIRE(!config_.disallow_nan_and_infinity(),
                         "Infinity disallowed");
          target()->set_type(JsonValue::TYPE_DOUBLE);
          target()->set_double_value(
              optional_minus ? -std::numeric_limits<double>::infinity()
                             : std::numeric_limits<double>::infinity());
          Advance(optional_minus + 3);
          if (remaining_.length() >= 5 &&
              (remaining_[0] == 'I' || remaining_[0] == 'i') &&
              (remaining_[1] == 'N' || remaining_[1] == 'n') &&
              (remaining_[2] == 'I' || remaining_[2] == 'i') &&
              (remaining_[3] == 'T' || remaining_[3] == 't') &&
              (remaining_[4] == 'Y' || remaining_[4] == 'y')) {
            Advance(5);
          }
        } else {
          size_t number_length = 0;
          if (optional_minus) {
            PJCORE_REQUIRE(remaining_.length() < 3 || remaining_[1] != '0' ||
                               !IsDigit::eval(remaining_[2]),
                           "Invalid number with leading zeroes");
            int64_t signed_value;
            number_length = ReadNumberPrefix(remaining_, &signed_value);
            if (number_length) {
              target()->set_type(JsonValue::TYPE_SIGNED);
              target()->set_signed_value(signed_value);
            }
          } else if (IsDigit::eval(remaining_[0])) {
            PJCORE_REQUIRE(remaining_.length() < 2 || remaining_[0] != '0' ||
                               !IsDigit::eval(remaining_[1]),
                           "Invalid number with leading zeroes");
            uint64_t unsigned_value;
            number_length = ReadNumberPrefix(remaining_, &unsigned_value);
            if (number_length) {
              if (unsigned_value <=
                  static_cast<uint64_t>(std::numeric_limits<int64_t>::max())) {
                target()->set_type(JsonValue::TYPE_SIGNED);
                target()->set_signed_value(
                    static_cast<int64_t>(unsigned_value));
              } else {
                target()->set_type(JsonValue::TYPE_UNSIGNED);
                target()->set_unsigned_value(unsigned_value);
              }
            }
          }

          if (!number_length || (number_length < remaining_.length() &&
                                 (remaining_[number_length] == 'e' ||
                                  remaining_[number_length] == 'E' ||
                                  remaining_[number_length] == '.'))) {
            target()->clear_type();
            target()->clear_signed_value();
            target()->clear_unsigned_value();
            PJCORE_REQUIRE(remaining_[0] == '-' || IsDigit::eval(remaining_[0]),
                           "Invalid value");
            double double_value;
            number_length = ReadNumberPrefix(remaining_, &double_value);
            PJCORE_REQUIRE(number_length, "Invalid number");
            target()->set_type(JsonValue::TYPE_DOUBLE);
            target()->set_double_value(double_value);
          }

          Advance(number_length);
        }
      }
    }

    for (;;) {
      targetStack_.pop();

      if (targetStack_.empty()) {
        PJCORE_REQUIRE_SILENT(ReadWhitespaceAndComments(),
                              "Failed to read whitespace and/or comments");
        PJCORE_REQUIRE(remaining_.empty(), "End expected");
        return true;
      }

      if (target()->type() == JsonValue::TYPE_OBJECT) {
        PJCORE_REQUIRE_SILENT(ReadWhitespaceAndComments(),
                              "Failed to read whitespace and/or comments");
        if (!remaining_.empty() && remaining_[0] == ',') {
          AdvanceOne();
          read_property_name = true;
          break;
        } else {
          PJCORE_REQUIRE(ReadChar('}'), "Close bracket or comma expected");
          if (!config_.properties_as_is()) {
            StableSortAndRemoveDuplicatesByName(
                target()->mutable_object_properties());
          }
        }
      } else {
        PJCORE_CHECK_EQ(JsonValue::TYPE_ARRAY, target()->type());
        PJCORE_REQUIRE_SILENT(ReadWhitespaceAndComments(),
                              "Failed to read whitespace and/or comments");
        if (!remaining_.empty() && remaining_[0] == ',') {
          AdvanceOne();
          targetStack_.push(target()->add_array_elements());
          break;
        } else {
          PJCORE_REQUIRE(ReadChar(']'), "Close brace or comma expected");
        }
      }
    }
  }
}

}  // unnamed namespace

bool ReadJson(StringPiece str, JsonValue* value, Error* error,
              const JsonReaderConfig& config) {
  PJCORE_CHECK(value);
  value->Clear();
  PJCORE_CHECK(error);
  error->Clear();

  Context context(config, str, value);

  PJCORE_REQUIRE_CAUSE(context.Complete(error), "Failed to parse JSON string");

  return true;
}

}  // namespace pjcore
