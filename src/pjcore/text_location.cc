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

#include "pjcore/logging.h"

namespace pjcore {

TextLocation MakeTextLocation() { return MakeTextLocation(0, 1, 1); }

TextLocation MakeTextLocation(uint64_t offset, uint64_t line, uint64_t column) {
  TextLocation text_location;
  text_location.set_offset(offset);
  text_location.set_line(line);
  text_location.set_column(column);
  return text_location;
}

TextLocation MakeTextLocation(StringPiece data) {
  TextLocation text_location = MakeTextLocation();
  AdvanceTextLocation(&text_location, data);
  return text_location;
}

void AdvanceTextLocation(TextLocation* mutable_text_location, char ch) {
  PJCORE_CHECK(mutable_text_location);
  mutable_text_location->set_offset(mutable_text_location->offset() + 1);
  if (ch == '\n') {
    mutable_text_location->set_line(mutable_text_location->line() + 1);
    mutable_text_location->set_column(1);
  } else if (ch == '\t') {
    mutable_text_location->set_column(
        ((mutable_text_location->column() - 1) | 7) + 2);
  } else if ((static_cast<uint8_t>(ch) & 0xc0) != 0x80) {
    mutable_text_location->set_column(mutable_text_location->column() + 1);
  }
}

void AdvanceTextLocation(TextLocation* mutable_text_location,
                         StringPiece data) {
  PJCORE_CHECK(mutable_text_location);

  mutable_text_location->set_offset(mutable_text_location->offset() +
                                    data.size());

  uint64_t line = mutable_text_location->line();
  uint64_t column = mutable_text_location->column();

  while (const void* newline = memchr(data.data(), '\n', data.size())) {
    size_t newline_position = (const char*)newline - data.data();
    ++line;
    column = 1;
    data.remove_prefix(newline_position + 1);
  }

  for (int position = 0; position < data.size(); ++position) {
    if (data[position] == '\t') {
      column = ((column - 1) | 7) + 2;
    } else if ((static_cast<uint8_t>(data[position]) & 0xc0) != 0x80) {
      ++column;
    }
  }

  mutable_text_location->set_line(line);
  mutable_text_location->set_column(column);
}

}  // namespace pjcore
