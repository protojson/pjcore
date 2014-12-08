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

#include "pjcore/url_util.h"

#include <http_parser.h>

#include <algorithm>

#include "pjcore/logging.h"
#include "pjcore/number_util.h"
#include "pjcore/repeated_field_util.h"
#include "pjcore/string_piece_util.h"
#include "pjcore/unicode.h"

namespace pjcore {

bool ParseUrl(StringPiece url, bool is_connect, ParsedUrl* parsed_url,
              Error* error) {
  PJCORE_CHECK(parsed_url);
  parsed_url->Clear();
  PJCORE_CHECK(error);
  error->Clear();

  http_parser_url result;
  memset(&result, 0, sizeof(result));

  PJCORE_REQUIRE(http_parser_parse_url(url.data(), url.size(),
                                       is_connect ? 1 : 0, &result) == 0,
                 "Failed to parse URL");

#define CONSIDER_FIELD(_FIELD_ENUM, _FIELD_NAME)              \
  do {                                                        \
    if ((result.field_set & (1 << UF_##_FIELD_ENUM)) != 0) {  \
      url.substr(result.field_data[UF_##_FIELD_ENUM].off,     \
                 result.field_data[UF_##_FIELD_ENUM].len)     \
          .CopyToString(parsed_url->mutable_##_FIELD_NAME()); \
    }                                                         \
  } while (false)

  CONSIDER_FIELD(SCHEMA, scheme);
  CONSIDER_FIELD(HOST, host);
  CONSIDER_FIELD(PATH, path);
  CONSIDER_FIELD(QUERY, query);
  CONSIDER_FIELD(FRAGMENT, fragment);
  CONSIDER_FIELD(USERINFO, userinfo);

#undef CONSIDER_FIELD

  if ((result.field_set & (1 << UF_PORT)) != 0) {
    parsed_url->set_port(result.port);
  }

  if (!parsed_url->query().empty()) {
    std::string query = parsed_url->query();
    std::replace(query.begin(), query.end(), '+', ' ');
    StringPiece remaining(query);

    for (;;) {
      size_t pair_delim = remaining.find('&');
      StringPiece pair = (pair_delim == StringPiece::npos)
                             ? remaining
                             : remaining.substr(0, pair_delim);

      size_t delim = pair.find('=');

      if (delim != StringPiece::npos) {
        UrlParameter parameter;
        if (DecodeUriComponent(pair.substr(0, delim),
                               parameter.mutable_name()) &&
            !parameter.name().empty() &&
            DecodeUriComponent(pair.substr(delim + 1),
                               parameter.mutable_value())) {
          *parsed_url->add_parameters() = parameter;
        }
      }

      if (pair_delim == StringPiece::npos) {
        break;
      }

      remaining.remove_prefix(pair_delim + 1);
    }
  }

  return true;
}

namespace {

// http://www.ecma-international.org/ecma-262/5.1/#sec-15.1.3
// URI Handling Function Properties

template <typename Predicate>
bool Encode(StringPiece str, const Predicate& is_unescaped,
            std::string* encoded) {
  PJCORE_CHECK(encoded);
  encoded->clear();

  PJCORE_REQUIRE_SILENT(Unicode::IsStructurallyValidUtf8(str), "Invalid UTF-8");

  for (size_t offset = 0; offset < str.length(); ++offset) {
    if (is_unescaped(str[offset])) {
      encoded->push_back(str[offset]);
    } else {
      uint8_t value = static_cast<uint8_t>(str[offset]);
      encoded->push_back('%');
      encoded->push_back(WriteHexDigit(value >> 4));
      encoded->push_back(WriteHexDigit(value & 0xf));
    }
  }

  return true;
}

PJCORE_CHAR_PREDICATE(IsUnescapedForEncodeUri, IsUriReserved::eval(ch) ||
                                                   IsUriUnescaped::eval(ch) ||
                                                   ch == '#');
PJCORE_CHAR_PREDICATE(IsUnescapedForEncodeUriComponent,
                      IsUriUnescaped::eval(ch));

template <typename Predicate>
bool Decode(StringPiece str, const Predicate& is_reserved,
            std::string* unencoded) {
  PJCORE_CHECK(unencoded);
  unencoded->clear();

  size_t offset = 0;

  while (offset < str.length()) {
    if (str[offset] != '%') {
      unencoded->push_back(str[offset]);
      ++offset;
    } else {
      PJCORE_REQUIRE_SILENT(str.length() - offset >= 3,
                            "Unterminated percent escape");
      PJCORE_REQUIRE_SILENT(IsHexDigit::eval(str[offset + 1]) &&
                                IsHexDigit::eval(str[offset + 2]),
                            "Invalid percent sequence");
      unencoded->push_back(static_cast<char>(
          ReadHexDigit(str[offset + 1]) << 4 | ReadHexDigit(str[offset + 2])));
      offset += 3;
    }
  }

  PJCORE_REQUIRE_SILENT(Unicode::IsStructurallyValidUtf8(*unencoded),
                        "Invalid unencoded UTF-8");

  return true;
}

PJCORE_CHAR_PREDICATE(IsReservedForDecodeUri,
                      IsUriReserved::eval(ch) || ch == '#');
PJCORE_CHAR_PREDICATE(IsReservedForDecodeUriComponent, false);

}  // unnamed namespace

bool EncodeUri(StringPiece uri, std::string* encoded_uri) {
  return Encode(uri, IsUnescapedForEncodeUri(), encoded_uri);
}

bool DecodeUri(StringPiece encoded_uri, std::string* uri) {
  return Decode(encoded_uri, IsReservedForDecodeUri(), uri);
}

bool EncodeUriComponent(StringPiece uri_component,
                        std::string* encoded_uri_component) {
  return Encode(uri_component, IsUnescapedForEncodeUriComponent(),
                encoded_uri_component);
}

bool DecodeUriComponent(StringPiece encoded_uri_component,
                        std::string* uri_component) {
  return Decode(encoded_uri_component, IsReservedForDecodeUriComponent(),
                uri_component);
}

bool GetNodeAndServiceName(const ParsedUrl& parsed_url,
                           NodeAndServiceName* host_and_service_name,
                           std::string* error) {
  PJCORE_CHECK(host_and_service_name);
  host_and_service_name->Clear();

  PJCORE_CHECK(error);
  error->clear();

  if (!parsed_url.host().empty()) {
    host_and_service_name->set_node_name(parsed_url.host());
  }

  if (parsed_url.has_port()) {
    host_and_service_name->set_service_name(WriteNumber(parsed_url.port()));
  } else if (!parsed_url.scheme().empty()) {
    host_and_service_name->set_service_name(parsed_url.scheme());
  }

  PJCORE_REQUIRE_STRING(
      !host_and_service_name->node_name().empty() ||
          !host_and_service_name->service_name().empty(),
      "At least one of node name and service must be specified");

  return true;
}

}  // namespace pjcore
