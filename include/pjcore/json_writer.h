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

#ifndef PJCORE_JSON_WRITER_H_
#define PJCORE_JSON_WRITER_H_

#include <string>

#include "pjcore/json.pb.h"
#include "pjcore/make_json_value.h"

namespace pjcore {

const uint32_t kJsonPrettyIndent = 2;

std::string WriteJson(
    const JsonValue& value,
    const JsonWriterConfig& config = JsonWriterConfig::default_instance());

template <typename Value>
std::string WriteJson(
    const Value& value,
    const JsonWriterConfig& config = JsonWriterConfig::default_instance()) {
  return WriteJson(MakeJsonValue(value), config);
}

std::string WritePrettyJson(const JsonValue& value);

template <typename Value>
std::string WritePrettyJson(const Value& value) {
  return WritePrettyJson(MakeJsonValue(value));
}

}  // namespace pjcore

#endif  // PJCORE_JSON_WRITER_H_
