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

#ifndef PJCORE_HTTP_UTIL_H_
#define PJCORE_HTTP_UTIL_H_

#include <http_parser.h>

#include <string>

#include "pjcore/third_party/chromium/string_piece.h"

#include "pjcore/http.pb.h"
#include "pjcore/live_http.pb.h"

namespace pjcore {

const int32_t kDefaultHttpPort = 80;

std::string HttpVersionToString(const HttpVersion& http_version);

bool NodeAndServiceNameEmpty(const NodeAndServiceName& node_and_service_name);

const char* HttpReasonPhrase(HttpStatusCode status_code);

bool http_method_IsValid(http_method method);

HttpVersion DefaultHttpVersion();

std::string GetCurrentHttpDate();

std::string CreateHttpStatusLine(const HttpVersion& version,
                                 const HttpStatusCode status_code);

bool CreateHttpRequestLineOut(http_method method, StringPiece url,
                              const HttpVersion& version,
                              std::string* request_line, std::string* error);

bool AppendHttpHeaders(
    const google::protobuf::RepeatedPtrField<HttpHeader>& headers,
    std::string* str, std::string* error);

void CaptureLiveHttpParserOut(const http_parser* parser, LiveHttpParser* live);

void CaptureLiveHttpRequestOut(const HttpRequest* request,
                               LiveHttpRequest* live,
                               size_t content_truncate_size = 0);

void CaptureLiveHttpResponseOut(const HttpResponse* response,
                                LiveHttpResponse* live,
                                size_t content_truncate_size = 0);

}  // namespace pjcore

#endif  // PJCORE_HTTP_UTIL_H_
