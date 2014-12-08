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

#ifndef PJCORE_UNICODE_H_
#define PJCORE_UNICODE_H_

#include <string>

#include "pjcore/third_party/chromium/string_piece.h"

namespace pjcore {

namespace Unicode {

typedef uint32_t CodePoint;

bool IsHighSurrogate(CodePoint code_point);

bool IsLowSurrogate(CodePoint code_point);

bool DecodeSurrogatePair(CodePoint high_surrogate, CodePoint low_surrogate,
                         CodePoint* surrogate_pair);

bool IsSurrogatePair(CodePoint code_point);

bool EncodeSurrogatePair(CodePoint surrogate_pair, CodePoint* high_surrogate,
                         CodePoint* low_surrogate);

int ReadCodePointPrefix(StringPiece str, CodePoint* code_point);

struct WriteCodePointBuffer {
  char buffer[5];
};

StringPiece WriteCodePointToBuffer(CodePoint code_point,
                                   WriteCodePointBuffer* buffer);

bool IsStructurallyValidUtf8(StringPiece str);

StringPiece ByteOrderMarkUtf8();

StringPiece ByteOrderMarkUtf16BigEndian();

StringPiece ByteOrderMarkUtf16LittleEndian();

StringPiece ByteOrderMarkUtf32BigEndian();

StringPiece ByteOrderMarkUtf32LittleEndian();

}  // namespace Unicode

}  // namespace pjcore

#endif  // PJCORE_UNICODE_H_
