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

#ifndef PJCORE_URL_UTIL_H_
#define PJCORE_URL_UTIL_H_

#include <string>

#include "pjcore/third_party/chromium/string_piece.h"

#include "pjcore/error.pb.h"
#include "pjcore/http.pb.h"

namespace pjcore {

bool ParseUrl(StringPiece url, bool is_connect, ParsedUrl* parsed_url,
              Error* error);

bool EncodeUri(StringPiece uri, std::string* encoded_uri);
bool DecodeUri(StringPiece encoded_uri, std::string* uri);

bool EncodeUriComponent(StringPiece uri_component,
                        std::string* encoded_uri_component);
bool DecodeUriComponent(StringPiece encoded_uri_component,
                        std::string* uri_component);

bool GetNodeAndServiceName(const ParsedUrl& parsed_url,
                           NodeAndServiceName* node_and_service_name,
                           std::string* error);

}  // namespace pjcore

#endif  // PJCORE_URL_UTIL_H_
