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

#include "pjcore/http_util.h"

#include <stdio.h>
#include <time.h>

#include <uv.h>

#include <http_parser.h>

#include "pjcore/logging.h"
#include "pjcore/json_util.h"
#include "pjcore/live_util.h"
#include "pjcore/json_writer.h"
#include "pjcore/name_value_util.h"
#include "pjcore/number_util.h"
#include "pjcore/string_piece_util.h"

namespace pjcore {

std::string HttpVersionToString(const HttpVersion& http_version) {
  return std::string("HTTP/") +
         WriteNumber(http_version.has_major() ? http_version.major() : 1) +
         "." +
         WriteNumber((http_version.has_major() && http_version.has_minor())
                         ? http_version.minor()
                         : 1);
}

bool NodeAndServiceNameEmpty(const NodeAndServiceName& node_and_service_name) {
  return node_and_service_name.node_name().empty() &&
         node_and_service_name.service_name().empty();
}

const char* HttpReasonPhrase(HttpStatusCode status_code) {
  switch (status_code) {
    case HTTP_STATUS_CODE_CONTINUE:
      return "Continue";

    case HTTP_STATUS_CODE_SWITCHING_PROTOCOLS:
      return "Switching Protocols";

    case HTTP_STATUS_CODE_OK:
      return "OK";

    case HTTP_STATUS_CODE_CREATED:
      return "Created";

    case HTTP_STATUS_CODE_ACCEPTED:
      return "Accepted";

    case HTTP_STATUS_CODE_NON_AUTHORITATIVE_INFORMATION:
      return "Non-Authoritative Information";

    case HTTP_STATUS_CODE_NO_CONTENT:
      return "No Content";

    case HTTP_STATUS_CODE_RESET_CONTENT:
      return "Reset Content";

    case HTTP_STATUS_CODE_PARTIAL_CONTENT:
      return "Partial Content";

    case HTTP_STATUS_CODE_MULTIPLE_CHOICES:
      return "Multiple Choices";

    case HTTP_STATUS_CODE_MOVED_PERMANENTLY:
      return "Moved Permanently";

    case HTTP_STATUS_CODE_FOUND:
      return "Found";

    case HTTP_STATUS_CODE_SEE_OTHER:
      return "See Other";

    case HTTP_STATUS_CODE_NOT_MODIFIED:
      return "Not Modified";

    case HTTP_STATUS_CODE_USE_PROXY:
      return "Use Proxy";

    case HTTP_STATUS_CODE__UNUSED_:
      return "(Unused)";

    case HTTP_STATUS_CODE_TEMPORARY_REDIRECT:
      return "Temporary Redirect";

    case HTTP_STATUS_CODE_BAD_REQUEST:
      return "Bad Request";

    case HTTP_STATUS_CODE_UNAUTHORIZED:
      return "Unauthorized";

    case HTTP_STATUS_CODE_PAYMENT_REQUIRED:
      return "Payment Required";

    case HTTP_STATUS_CODE_FORBIDDEN:
      return "Forbidden";

    case HTTP_STATUS_CODE_NOT_FOUND:
      return "Not Found";

    case HTTP_STATUS_CODE_METHOD_NOT_ALLOWED:
      return "Method Not Allowed";

    case HTTP_STATUS_CODE_NOT_ACCEPTABLE:
      return "Not Acceptable";

    case HTTP_STATUS_CODE_PROXY_AUTHENTICATION_REQUIRED:
      return "Proxy Authentication Required";

    case HTTP_STATUS_CODE_REQUEST_TIMEOUT:
      return "Request Timeout";

    case HTTP_STATUS_CODE_CONFLICT:
      return "Conflict";

    case HTTP_STATUS_CODE_GONE:
      return "Gone";

    case HTTP_STATUS_CODE_LENGTH_REQUIRED:
      return "Length Required";

    case HTTP_STATUS_CODE_PRECONDITION_FAILED:
      return "Precondition Failed";

    case HTTP_STATUS_CODE_REQUEST_ENTITY_TOO_LARGE:
      return "Request Entity Too Large";

    case HTTP_STATUS_CODE_REQUEST_URI_TOO_LONG:
      return "Request-URI Too Long";

    case HTTP_STATUS_CODE_UNSUPPORTED_MEDIA_TYPE:
      return "Unsupported Media Type";

    case HTTP_STATUS_CODE_REQUESTED_RANGE_NOT_SATISFIABLE:
      return "Requested Range Not Satisfiable";

    case HTTP_STATUS_CODE_EXPECTATION_FAILED:
      return "Expectation Failed";

    case HTTP_STATUS_CODE_INTERNAL_SERVER_ERROR:
      return "Internal Server Error";

    case HTTP_STATUS_CODE_NOT_IMPLEMENTED:
      return "Not Implemented";

    case HTTP_STATUS_CODE_BAD_GATEWAY:
      return "Bad Gateway";

    case HTTP_STATUS_CODE_SERVICE_UNAVAILABLE:
      return "Service Unavailable";

    case HTTP_STATUS_CODE_GATEWAY_TIMEOUT:
      return "Gateway Timeout";

    case HTTP_STATUS_CODE_HTTP_VERSION_NOT_SUPPORTED:
      return "HTTP Version Not Supported";

    default:
      return "Unknown HTTP status code";
  }
}

bool http_method_IsValid(http_method method) {
  switch (method) {
#define XX(num, name, string) \
  case HTTP_##name:           \
    return true;
    HTTP_METHOD_MAP(XX)
#undef XX

    default:
      return false;
  }
}

HttpVersion DefaultHttpVersion() {
  HttpVersion default_http_version;
  default_http_version.set_major(1);
  default_http_version.set_minor(1);
  return default_http_version;
}

std::string GetCurrentHttpDate() {
  time_t current_time = time(0);
  tm current_time_detail;
#ifdef _WIN32
  if (gmtime_s(&current_time_detail, &current_time) != 0) {
    return std::string();
  }
#else
  if (!gmtime_r(&current_time, &current_time_detail)) {
    return std::string();
  }
#endif
  char buffer[1024];
  size_t length = strftime(buffer, sizeof(buffer), "%a, %d %b %Y %H:%M:%S GMT",
                           &current_time_detail);
  return std::string(buffer, length);
}

std::string CreateHttpStatusLine(const HttpVersion& version,
                                 const HttpStatusCode status_code) {
  return HttpVersionToString(version) + " " +
         WriteNumber(static_cast<int32_t>(status_code)) + " " +
         HttpReasonPhrase(status_code) + "\r\n";
}

bool CreateHttpRequestLineOut(http_method method, StringPiece url,
                              const HttpVersion& version,
                              std::string* request_line, std::string* error) {
  PJCORE_CHECK(request_line);
  request_line->clear();
  PJCORE_CHECK(error);
  error->clear();

  PJCORE_REQUIRE_STRING(
      http_method_IsValid(method),
      std::string("Invalid method: ") + WriteNumber(static_cast<int>(method)));

  size_t valid_url_length =
      MatchingPrefixLength(url, IsAsciiNotSpaceNotControl());

  PJCORE_REQUIRE_STRING(
      valid_url_length == url.length(),
      std::string("Invalid character code in URL: ") +
          WriteNumber(static_cast<uint8_t>(url[valid_url_length])));

  request_line->assign(http_method_str(method));
  request_line->push_back(' ');
  url.AppendToString(request_line);
  request_line->push_back(' ');
  request_line->append(HttpVersionToString(version));
  request_line->append("\r\n", 2);
  return true;
}

bool AppendHttpHeaders(
    const google::protobuf::RepeatedPtrField<HttpHeader>& headers,
    std::string* str, std::string* error) {
  PJCORE_CHECK(str);
  PJCORE_CHECK(error);
  error->clear();

  // TODO(pjcore): verify according to
  // http://www.w3.org/Protocols/rfc2616/rfc2616-sec4.html#sec4.2

  for (google::protobuf::RepeatedPtrField<HttpHeader>::const_iterator it =
           headers.begin();
       it != headers.end(); ++it) {
    str->append(it->name());
    str->append(": ", 2);
    str->append(it->value());
    str->append("\r\n", 2);
  }

  return true;
}

void CaptureLiveHttpParserOut(const http_parser* parser, LiveHttpParser* live) {
  PJCORE_CHECK(parser);

  PJCORE_CHECK(live);
  live->Clear();

  live->set_ptr(reinterpret_cast<uint64_t>(parser));

  switch (static_cast<http_parser_type>(parser->type)) {
    case HTTP_REQUEST:
      live->set_type(HTTP_PARSER_REQUEST);
      break;

    case HTTP_RESPONSE:
      live->set_type(HTTP_PARSER_RESPONSE);
      break;

    case HTTP_BOTH:
      live->set_type(HTTP_PARSER_BOTH);
      break;

    default:
      break;
  }

  if (parser->flags) {
    live->set_flags(parser->flags);

#define CONSIDER_FLAG(_FLAG_ENUM, _FLAG_NAME)    \
  do {                                           \
    if ((parser->flags & F_##_FLAG_ENUM) != 0) { \
      live->set_flag_##_FLAG_NAME(true);         \
    }                                            \
  } while (false)

    CONSIDER_FLAG(CHUNKED, chunked);
    CONSIDER_FLAG(CONNECTION_KEEP_ALIVE, connection_keep_alive);
    CONSIDER_FLAG(CONNECTION_CLOSE, connection_close);
    CONSIDER_FLAG(TRAILING, trailing);
    CONSIDER_FLAG(UPGRADE, upgrade);
    CONSIDER_FLAG(SKIPBODY, skipbody);

#undef CONSIDER_FLAG
  }

  if (parser->state) {
    live->set_state(parser->state);
  }

  if (parser->header_state) {
    live->set_header_state(parser->header_state);
  }

  if (parser->index) {
    live->set_index(parser->index);
  }

  if (parser->nread) {
    live->set_nread(parser->nread);
  }

  if (parser->content_length) {
    live->set_content_length(parser->content_length);
  }

  if (parser->http_major) {
    live->set_http_major(parser->http_major);
  }

  if (parser->http_minor) {
    live->set_http_minor(parser->http_minor);
  }

  if (HttpStatusCode_IsValid(parser->status_code)) {
    live->set_status_code(static_cast<HttpStatusCode>(parser->status_code));
  }

  live->set_method(parser->method);
  live->set_method_str(
      http_method_str(static_cast<http_method>(parser->method)));

  if (parser->http_errno) {
    live->set_http_errno(parser->http_errno);
  }

  if (parser->upgrade) {
    live->set_upgrade(parser->upgrade);
  }

  if (parser->data) {
    live->set_data_ptr(reinterpret_cast<uint64_t>(parser->data));
  }
}

void CaptureLiveHttpRequestOut(const HttpRequest* request,
                               LiveHttpRequest* live,
                               size_t content_truncate_size) {
  PJCORE_CHECK(request);

  PJCORE_CHECK(live);
  live->Clear();

  live->set_ptr(reinterpret_cast<int64_t>(request));

  if (request->has_method()) {
    live->set_method(request->method());
    live->set_method_str(
        http_method_str(static_cast<http_method>(request->method())));
  }

  if (!request->url().empty()) {
    live->set_url(request->url());
  }

  if (request->has_parsed_url()) {
    live->mutable_parsed_url()->CopyFrom(request->parsed_url());
  }

  if (request->has_http_version()) {
    live->mutable_http_version()->CopyFrom(request->http_version());
  }

  live->mutable_headers()->CopyFrom(request->headers());

  if (!request->content().empty()) {
    CaptureLiveContentOut(request->content(), live->mutable_content(),
                          content_truncate_size);
  }
}

void CaptureLiveHttpResponseOut(const HttpResponse* response,
                                LiveHttpResponse* live,
                                size_t content_truncate_size) {
  PJCORE_CHECK(response);

  PJCORE_CHECK(live);
  live->Clear();

  live->set_ptr(reinterpret_cast<int64_t>(response));

  if (response->has_http_version()) {
    live->mutable_http_version()->CopyFrom(response->http_version());
  }

  if (response->has_status_code()) {
    live->set_status_code(response->status_code());
  }

  live->mutable_headers()->CopyFrom(response->headers());

  if (!response->content().empty()) {
    CaptureLiveContentOut(response->content(), live->mutable_content(),
                          content_truncate_size);
  }
}

}  // namespace pjcore
