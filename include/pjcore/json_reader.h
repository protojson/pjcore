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

#ifndef PJCORE_JSON_READER_H_
#define PJCORE_JSON_READER_H_

#include "pjcore/third_party/chromium/string_piece.h"

#include "pjcore/error.pb.h"
#include "pjcore/json.pb.h"

namespace pjcore {

bool ReadJson(
    StringPiece str, JsonValue* value, Error* error,
    const JsonReaderConfig& config = JsonReaderConfig::default_instance());

}  // namespace pjcore

#endif  // PJCORE_JSON_READER_H_
