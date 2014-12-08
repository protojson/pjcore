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

#ifndef PJCORE_UNBOX_JSON_VALUE_H_
#define PJCORE_UNBOX_JSON_VALUE_H_

#include <string>

#include "pjcore/error.pb.h"
#include "pjcore/json.pb.h"

namespace pjcore {

bool UnboxJsonValue(const JsonValue& json_value, bool* bool_value,
                    Error* error);

bool UnboxJsonValue(const JsonValue& json_value, int32_t* signed_value,
                    Error* error);

bool UnboxJsonValue(const JsonValue& json_value, int64_t* signed_value,
                    Error* error);

bool UnboxJsonValue(const JsonValue& json_value,
                    long long int* signed_value,  // NOLINT(runtime/int)
                    Error* error);

bool UnboxJsonValue(const JsonValue& json_value, uint32_t* unsigned_value,
                    Error* error);

bool UnboxJsonValue(const JsonValue& json_value, uint64_t* unsigned_value,
                    Error* error);

bool UnboxJsonValue(const JsonValue& json_value,
                    long long unsigned* unsigned_value,  // NOLINT(runtime/int)
                    Error* error);

bool UnboxJsonValue(const JsonValue& json_value, float* float_value,
                    Error* error);

bool UnboxJsonValue(const JsonValue& json_value, double* double_value,
                    Error* error);

bool UnboxJsonValue(const JsonValue& json_value, std::string* string_value,
                    Error* error);

bool UnboxJsonValue(const JsonValue& json_value,
                    google::protobuf::Message* message, Error* error,
                    google::protobuf::RepeatedPtrField<JsonValue::Property>*
                        unknown_object_properties = NULL);

bool UnboxJsonValue(const JsonValue& json_value, JsonValue* json_value_copy,
                    Error* error);

}  // namespace pjcore

#endif  // PJCORE_UNBOX_JSON_VALUE_H_
