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

#include "pjcore/unicode.h"

#include <google/protobuf/stubs/common.h>

#include "pjcore/logging.h"

namespace pjcore {

namespace Unicode {

namespace {

const CodePoint kHighSurrogateBegin = 0xd800;
const CodePoint kHighSurrogateEnd = 0xdc00;

const CodePoint kLowSurrogateBegin = 0xdc00;
const CodePoint kLowSurrogateEnd = 0xe000;

const CodePoint kSurrogatePairBegin = 0x10000;
const CodePoint kSurrogatePairEnd = 0x110000;

}  // unnamed namespace

bool IsHighSurrogate(CodePoint code_point) {
  return code_point >= kHighSurrogateBegin && code_point < kHighSurrogateEnd;
}

bool IsLowSurrogate(CodePoint code_point) {
  return code_point >= kLowSurrogateBegin && code_point < kLowSurrogateEnd;
}

bool DecodeSurrogatePair(CodePoint high_surrogate, CodePoint low_surrogate,
                         CodePoint* code_point) {
  PJCORE_CHECK(code_point);

  if (!IsHighSurrogate(high_surrogate)) {
    PJCORE_FAIL_SILENT("Invalid high surrogate");
  }

  if (!IsLowSurrogate(low_surrogate)) {
    PJCORE_FAIL_SILENT("Invalid low surrogate");
  }

  *code_point =
      kSurrogatePairBegin + (((high_surrogate - kHighSurrogateBegin) << 10) |
                             (low_surrogate - kLowSurrogateBegin));

  return true;
}

bool IsSurrogatePair(CodePoint code_point) {
  return code_point >= kSurrogatePairBegin && code_point < kSurrogatePairEnd;
}

bool EncodeSurrogatePair(CodePoint surrogate_pair, CodePoint* high_surrogate,
                         CodePoint* low_surrogate) {
  PJCORE_CHECK(high_surrogate);
  PJCORE_CHECK(low_surrogate);

  if (!IsSurrogatePair(surrogate_pair)) {
    PJCORE_FAIL_SILENT("Code point not surrogate");
  }

  surrogate_pair -= kSurrogatePairBegin;

  *high_surrogate = (surrogate_pair >> 10) + kHighSurrogateBegin;
  *low_surrogate = (surrogate_pair & 0x3ff) + kLowSurrogateBegin;

  return true;
}

int ReadCodePointPrefix(StringPiece str, CodePoint* code_point) {
  PJCORE_CHECK(code_point);
  *code_point = 0;

  const uint8_t* data = reinterpret_cast<const uint8_t*>(str.data());

  if (str.size() < 1) {
    return 0;
  }

  if ((data[0] & 0x80) == 0) {
    *code_point = data[0] & 0x7f;
    return 1;
  }

  if ((data[0] & 0xc0) == 0x80) {
    return 0;
  }

  if ((data[0] & 0xe0) == 0xc0) {
    if (str.size() < 2 || (data[1] & 0xc0) != 0x80) {
      return 0;
    }
    *code_point = ((data[0] & 0x1f) << 6) | (data[1] & 0x3f);
    return 2;
  }

  if ((data[0] & 0xf0) == 0xe0) {
    if (str.size() < 2 || (data[1] & 0xc0) != 0x80 ||
        (data[2] & 0xc0) != 0x80) {
      return 0;
    }
    *code_point =
        ((data[0] & 0x0f) << 12) | ((data[1] & 0x3f) << 6) | (data[2] & 0x3f);
    return 3;
  }

  if ((data[0] & 0xf8) == 0xf0) {
    if (str.size() < 2 || (data[1] & 0xc0) != 0x80 ||
        (data[2] & 0xc0) != 0x80 || (data[3] & 0xc0) != 0x80) {
      return 0;
    }
    *code_point = ((data[0] & 0x0f) << 18) | ((data[1] & 0x3f) << 12) |
                  ((data[2] & 0x3f) << 6) | (data[3] & 0x3f);
    return 4;
  }

  return 0;
}

StringPiece WriteCodePointToBuffer(CodePoint code_point,
                                   WriteCodePointBuffer* buffer) {
  PJCORE_CHECK(buffer);

  uint8_t* data = reinterpret_cast<uint8_t*>(buffer->buffer);

  if (code_point < 0x80) {
    data[0] = static_cast<uint8_t>(code_point);
    data[1] = 0;
    return StringPiece(buffer->buffer, 1);
  }

  if (code_point < 0x800) {
    data[0] = static_cast<uint8_t>(code_point >> 6) | 0xc0;
    data[1] = (static_cast<uint8_t>(code_point) & 0x3f) | 0x80;
    data[2] = 0;
    return StringPiece(buffer->buffer, 2);
  }

  if (code_point < 0x10000) {
    data[0] = static_cast<uint8_t>(code_point >> 12) | 0xe0;
    data[1] = (static_cast<uint8_t>(code_point >> 6) & 0x3f) | 0x80;
    data[2] = (static_cast<uint8_t>(code_point) & 0x3f) | 0x80;
    data[3] = 0;
    return StringPiece(buffer->buffer, 3);
  }

  if (code_point < 0x200000) {
    data[0] = static_cast<uint8_t>(code_point >> 18) | 0xf0;
    data[1] = (static_cast<uint8_t>(code_point >> 12) & 0x3f) | 0x80;
    data[2] = (static_cast<uint8_t>(code_point >> 6) & 0x3f) | 0x80;
    data[3] = (static_cast<uint8_t>(code_point) & 0x3f) | 0x80;
    data[4] = 0;
    return StringPiece(buffer->buffer, 4);
  }

  data[0] = 0;
  return StringPiece(buffer->buffer, 0);
}

bool IsStructurallyValidUtf8(StringPiece str) {
  return google::protobuf::internal::IsStructurallyValidUTF8(
      str.data(), static_cast<int>(str.size()));
}

StringPiece ByteOrderMarkUtf8() {
  static const char byte_order_mark[] = {0xef, 0xbb, 0xbf};
  return StringPiece(byte_order_mark, sizeof(byte_order_mark));
}

StringPiece ByteOrderMarkUtf16BigEndian() {
  static const char byte_order_mark[] = {0xfe, 0xff};
  return StringPiece(byte_order_mark, sizeof(byte_order_mark));
}

StringPiece ByteOrderMarkUtf16LittleEndian() {
  static const char byte_order_mark[] = {0xff, 0xfe};
  return StringPiece(byte_order_mark, sizeof(byte_order_mark));
}

StringPiece ByteOrderMarkUtf32BigEndian() {
  static const char byte_order_mark[] = {0x00, 0x00, 0xfe, 0xff};
  return StringPiece(byte_order_mark, sizeof(byte_order_mark));
}

StringPiece ByteOrderMarkUtf32LittleEndian() {
  static const char byte_order_mark[] = {0xff, 0xfe, 0x00, 0x00};
  return StringPiece(byte_order_mark, sizeof(byte_order_mark));
}

}  // namespace Unicode

}  // namespace pjcore
