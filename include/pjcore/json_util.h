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

#ifndef PJCORE_JSON_UTIL_H_
#define PJCORE_JSON_UTIL_H_

#include <string>

#include "pjcore/third_party/chromium/string_piece.h"

#include "pjcore/error.pb.h"
#include "pjcore/json.pb.h"
#include "pjcore/make_json_value.h"

namespace pjcore {

bool VerifyJsonType(const JsonValue& value, std::string* error);

bool IsJsonNumber(const JsonValue& value);

bool AreJsonValuesEqual(const JsonValue& left, const JsonValue& right,
                        std::string* optional_diff_path = NULL);

template <typename LeftValue, typename RightValue>
bool AreJsonValuesEqual(const LeftValue& left, const RightValue& right,
                        std::string* optional_diff_path = NULL) {
  return AreJsonValuesEqual(MakeJsonValue(left), MakeJsonValue(right),
                            optional_diff_path);
}

const std::string& GetRootJsonPath();
void AppendJsonPathProperty(StringPiece property_name, std::string* json_path);
void AppendJsonPathElement(int64_t element_index, std::string* json_path);

std::string StripQuotesUnescapeTabsAndSlashes(StringPiece str);

}  // namespace pjcore

#endif  // PJCORE_JSON_UTIL_H_
