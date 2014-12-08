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

#include "pjcore/json_writer.h"

#include <limits>
#include <stack>
#include <string>

#include "pjcore/logging.h"
#include "pjcore/json_util.h"
#include "pjcore/number_util.h"
#include "pjcore/repeated_field_util.h"
#include "pjcore/string_piece_util.h"
#include "pjcore/unicode.h"

namespace pjcore {

namespace {

struct Source {
  explicit Source(const JsonValue* a_value = NULL)
      : value(a_value), index(-1) {}

  const JsonValue* value;

  int index;
};

class Context {
 public:
  Context(const JsonWriterConfig& config, const JsonValue& value,
          std::string* output)
      : config_(config), value_(value), output_(output) {
    PJCORE_CHECK(output_);
    output_->clear();
  }

  void Complete();

 private:
  Source& source() { return source_stack_.top(); }

  void Indent() {
    newline_indent_.resize(newline_indent_.size() + config_.indent(), ' ');
  }

  void Outdent() {
    newline_indent_.resize(newline_indent_.size() - config_.indent());
  }

  void WriteFourHexDigits(uint32_t value) {
    output_->push_back(WriteHexDigit(value >> 12));
    output_->push_back(WriteHexDigit((value >> 8) & 0xf));
    output_->push_back(WriteHexDigit((value >> 4) & 0xf));
    output_->push_back(WriteHexDigit(value & 0xf));
  }

  void WriteHumanStrings(StringPiece str) {}

  void WriteString(StringPiece str) {
    output_->push_back('"');

    size_t offset = 0;

    while (offset < str.length()) {
      switch (str[offset]) {
        case '"':
          output_->append("\\\"");
          ++offset;
          break;

        case '\\':
          output_->append("\\\\");
          ++offset;
          break;

        case '/':
          output_->append("\\/");
          ++offset;
          break;

        case '\b':
          output_->append("\\b");
          ++offset;
          break;

        case '\n':
          output_->append("\\n");
          ++offset;
          break;

        case '\f':
          output_->append("\\f");
          ++offset;
          break;

        case '\r':
          output_->append("\\r");
          ++offset;
          break;

        case '\t':
          output_->append("\\t");
          ++offset;
          break;

        default:
          if (!config_.escape_unicode() ||
              static_cast<uint8_t>(str[offset]) < 0x80) {
            output_->push_back(str[offset]);
            ++offset;
          } else {
            Unicode::CodePoint code_point;
            size_t code_point_length =
                Unicode::ReadCodePointPrefix(str.substr(offset), &code_point);

            if (!code_point_length) {
              output_->push_back(str[offset]);
              ++offset;
            } else {
              if (!Unicode::IsSurrogatePair(code_point)) {
                output_->append("\\u");
                WriteFourHexDigits(code_point);
              } else {
                Unicode::CodePoint high_surrogate;
                Unicode::CodePoint low_surrogate;

                if (!Unicode::EncodeSurrogatePair(code_point, &high_surrogate,
                                                  &low_surrogate)) {
                  output_->append(str.data() + offset, code_point_length);
                } else {
                  output_->append("\\u");
                  WriteFourHexDigits(high_surrogate);

                  output_->append("\\u");
                  WriteFourHexDigits(low_surrogate);
                }
              }

              offset += code_point_length;
            }
          }
          break;
      }
    }

    output_->push_back('"');
  }

  const JsonWriterConfig& config_;

  const JsonValue& value_;

  std::string* output_;

  std::stack<Source> source_stack_;

  std::string newline_indent_;
};

void Context::Complete() {
  output_->clear();

  if (config_.include_byte_order_mark()) {
    Unicode::ByteOrderMarkUtf8().AppendToString(output_);
  }

  source_stack_.push(Source(&value_));
  if (config_.indent()) {
    newline_indent_ = "\n";
  }

  for (;;) {
    std::string type_error;
    PJCORE_CHECK(VerifyJsonType(*source().value, &type_error));  // type_error

    switch (source().value->type()) {
      case JsonValue::TYPE_NULL:
        output_->append("null", 4);
        break;

      case JsonValue::TYPE_STRING:
        WriteString(source().value->string_value());
        break;

      case JsonValue::TYPE_SIGNED:
        output_->append(WriteNumber(source().value->signed_value()));
        break;

      case JsonValue::TYPE_UNSIGNED:
        output_->append(WriteNumber(source().value->unsigned_value()));
        break;

      case JsonValue::TYPE_DOUBLE:
        if (source().value->double_value() != source().value->double_value()) {
          if (config_.null_for_nan_and_infinity()) {
            output_->append("null", 4);
          } else {
            output_->append("NaN", 3);
          }
        } else if (source().value->double_value() ==
                   std::numeric_limits<double>::infinity()) {
          if (config_.null_for_nan_and_infinity()) {
            output_->append("null", 4);
          } else {
            output_->append("Infinity", 8);
          }
        } else if (source().value->double_value() ==
                   -std::numeric_limits<double>::infinity()) {
          if (config_.null_for_nan_and_infinity()) {
            output_->append("null", 4);
          } else {
            output_->append("-Infinity", 9);
          }
        } else {
          output_->append(WriteNumber(source().value->double_value()));
        }
        break;

      case JsonValue::TYPE_OBJECT:
        if (Empty(source().value->object_properties())) {
          output_->append("{}");
        } else {
          output_->push_back('{');
          source_stack_.push(Source());
          if (config_.indent()) {
            Indent();
          }
        }
        break;

      case JsonValue::TYPE_ARRAY:
        if (Empty(source().value->array_elements())) {
          output_->append("[]");
        } else {
          output_->push_back('[');
          source_stack_.push(Source());
          if (config_.indent()) {
            Indent();
          }
        }
        break;

      case JsonValue::TYPE_BOOL:
        output_->append(WriteNumber(source().value->bool_value()));
        break;

      default:
        PJCORE_CHECK(false);  // source().value->type()
    }

    for (;;) {
      source_stack_.pop();

      if (source_stack_.empty()) {
        return;
      }

      if (source().value->type() == JsonValue::TYPE_OBJECT) {
        if (++source().index >= source().value->object_properties_size()) {
          if (config_.indent()) {
            Outdent();
            output_->append(newline_indent_);
          }
          output_->push_back('}');
        } else {
          if (source().index > 0) {
            output_->push_back(',');
          }
          if (config_.indent()) {
            output_->append(newline_indent_);
          } else if (source().index != 0 && config_.space()) {
            output_->push_back(' ');
          }
          WriteString(source().value->object_properties(source().index).name());
          output_->push_back(':');
          if (config_.space()) {
            output_->push_back(' ');
          }
          source_stack_.push(Source(
              &source().value->object_properties(source().index).value()));
          break;
        }
      } else {
        PJCORE_CHECK_EQ(JsonValue::TYPE_ARRAY, source().value->type());
        if (++source().index >= source().value->array_elements_size()) {
          if (config_.indent()) {
            Outdent();
            output_->append(newline_indent_);
          }
          output_->push_back(']');
        } else {
          if (source().index > 0) {
            output_->push_back(',');
          }
          if (config_.indent()) {
            output_->append(newline_indent_);
          } else if (source().index != 0 && config_.space()) {
            output_->push_back(' ');
          }
          source_stack_.push(
              Source(&source().value->array_elements(source().index)));
          break;
        }
      }
    }
  }
}

}  // unnamed namespace

std::string WriteJson(const JsonValue& value, const JsonWriterConfig& config) {
  std::string str;

  Context context(config, value, &str);

  context.Complete();

  return str;
}

std::string WritePrettyJson(const JsonValue& value) {
  JsonWriterConfig config;
  config.set_indent(kJsonPrettyIndent);
  config.set_space(true);
  return WriteJson(value, config);
}

}  // namespace pjcore
