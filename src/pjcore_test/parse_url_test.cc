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

#include <gtest/gtest.h>

#include "pjcore_test/url_parser_test_message.h"

#include "pjcore/url_util.h"
#include "pjcore/error_util.h"
#include "pjcore/json_util.h"
#include "pjcore/json_writer.h"
#include "pjcore/logging.h"
#include "pjcore/make_json_value.h"

namespace pjcore {

using ::testing::AssertionResult;
using ::testing::AssertionSuccess;
using ::testing::AssertionFailure;

namespace {

AssertionResult TestMessage(const UrlParserTestMessage& message) {
  StringPiece url(message.url);

  if (!message.rv) {
    GlobalLogOverride global_log_override;
    ParsedUrl parsed_url;
    Error error;
    if (ParseUrl(url, message.is_connect, &parsed_url, &error)) {
      return AssertionFailure() << "Unexpected success";
    }
  } else {
    ParsedUrl parsed_url;
    Error error;
    if (!ParseUrl(url, message.is_connect, &parsed_url, &error)) {
      return AssertionFailure() << ErrorToString(error);
    }

    if (message.u.port == 0) {
      if (parsed_url.has_port()) {
        return AssertionFailure() << "Unexpected port: " << parsed_url.port();
      }
    } else {
      if (!parsed_url.has_port()) {
        return AssertionFailure() << "Port expected";
      }
      if (parsed_url.port() != message.u.port) {
        return AssertionFailure() << "Expected port " << message.u.port
                                  << ", actual: " << parsed_url.port();
      }
    }
#define CONSIDER_FIELD(_FIELD_ENUM, _FIELD_NAME)                               \
  do {                                                                         \
    if ((message.u.field_set & (1 << UF_##_FIELD_ENUM)) == 0) {                \
      if (parsed_url.has_##_FIELD_NAME()) {                                    \
        return AssertionFailure() << "Unexpected " #_FIELD_NAME ": "           \
                                  << parsed_url._FIELD_NAME();                 \
      }                                                                        \
    } else {                                                                   \
      StringPiece expected_field =                                             \
          url.substr(message.u.field_data[UF_##_FIELD_ENUM].off,               \
                     message.u.field_data[UF_##_FIELD_ENUM].len);              \
      if (!parsed_url.has_##_FIELD_NAME()) {                                   \
        return AssertionFailure() << "Expected " #_FIELD_NAME;                 \
      }                                                                        \
      if (parsed_url._FIELD_NAME() != expected_field) {                        \
        return AssertionFailure() << "Expected " #_FIELD_NAME " "              \
                                  << expected_field.as_string()                \
                                  << ", actual: " << parsed_url._FIELD_NAME(); \
      }                                                                        \
    }                                                                          \
  } while (false)

    CONSIDER_FIELD(SCHEMA, scheme);
    CONSIDER_FIELD(HOST, host);
    CONSIDER_FIELD(PATH, path);
    CONSIDER_FIELD(QUERY, query);
    CONSIDER_FIELD(FRAGMENT, fragment);
    CONSIDER_FIELD(USERINFO, userinfo);

#undef CONSIDER_FIELD
  }

  return AssertionSuccess();
}

}  // unnamed namespace

TEST(ParseUrl, ProxyRequest) {
  EXPECT_TRUE(TestMessage(UrlParserTestMessage::kProxyRequest));
}

TEST(ParseUrl, ProxyRequestWithPort) {
  EXPECT_TRUE(TestMessage(UrlParserTestMessage::kProxyRequestWithPort));
}

TEST(ParseUrl, ConnectRequest) {
  EXPECT_TRUE(TestMessage(UrlParserTestMessage::kConnectRequest));
}

TEST(ParseUrl, ConnectRequestButNotConnect) {
  EXPECT_TRUE(TestMessage(UrlParserTestMessage::kConnectRequestButNotConnect));
}

TEST(ParseUrl, ProxyIpv6Request) {
  EXPECT_TRUE(TestMessage(UrlParserTestMessage::kProxyIpv6Request));
}

TEST(ParseUrl, ProxyIpv6RequestWithPort) {
  EXPECT_TRUE(TestMessage(UrlParserTestMessage::kProxyIpv6RequestWithPort));
}

TEST(ParseUrl, ConnectIpv6Request) {
  EXPECT_TRUE(TestMessage(UrlParserTestMessage::kConnectIpv6Request));
}

TEST(ParseUrl, Ipv4InIpv6Request) {
  EXPECT_TRUE(TestMessage(UrlParserTestMessage::kIpv4InIpv6Request));
}

TEST(ParseUrl, ExtractQuestionMarkInQueryString) {
  EXPECT_TRUE(
      TestMessage(UrlParserTestMessage::kExtractQuestionMarkInQueryString));
}

TEST(ParseUrl, SpaceUrlEncodedRequest) {
  EXPECT_TRUE(TestMessage(UrlParserTestMessage::kSpaceUrlEncodedRequest));
}

TEST(ParseUrl, UrlFragment) {
  EXPECT_TRUE(TestMessage(UrlParserTestMessage::kUrlFragment));
}

TEST(ParseUrl, ComplexUrlFragment) {
  EXPECT_TRUE(TestMessage(UrlParserTestMessage::kComplexUrlFragment));
}

TEST(ParseUrl, ComplexUrlFromNodeJs) {
  EXPECT_TRUE(TestMessage(UrlParserTestMessage::kComplexUrlFromNodeJs));
}

TEST(ParseUrl, ComplexUrlWithBasicAuth) {
  EXPECT_TRUE(TestMessage(UrlParserTestMessage::kComplexUrlWithBasicAuth));
}

TEST(ParseUrl, DoubleAtSign) {
  EXPECT_TRUE(TestMessage(UrlParserTestMessage::kDoubleAtSign));
}

TEST(ParseUrl, ProxyEmptyHost) {
  EXPECT_TRUE(TestMessage(UrlParserTestMessage::kProxyEmptyHost));
}

TEST(ParseUrl, ProxyEmptyPort) {
  EXPECT_TRUE(TestMessage(UrlParserTestMessage::kProxyEmptyPort));
}

TEST(ParseUrl, ConnectWithBasicAuth) {
  EXPECT_TRUE(TestMessage(UrlParserTestMessage::kConnectWithBasicAuth));
}

TEST(ParseUrl, ConnectEmptyHost) {
  EXPECT_TRUE(TestMessage(UrlParserTestMessage::kConnectEmptyHost));
}

TEST(ParseUrl, ConnectEmptyPort) {
  EXPECT_TRUE(TestMessage(UrlParserTestMessage::kConnectEmptyPort));
}

TEST(ParseUrl, ConnectWithExtraBits) {
  EXPECT_TRUE(TestMessage(UrlParserTestMessage::kConnectWithExtraBits));
}

TEST(ParseUrl, SpaceInUrl) {
  EXPECT_TRUE(TestMessage(UrlParserTestMessage::kSpaceInUrl));
}

TEST(ParseUrl, ProxyBasicAuthWithSpaceUrlEncoded) {
  EXPECT_TRUE(
      TestMessage(UrlParserTestMessage::kProxyBasicAuthWithSpaceUrlEncoded));
}

TEST(ParseUrl, CarriageReturnInUrl) {
  EXPECT_TRUE(TestMessage(UrlParserTestMessage::kCarriageReturnInUrl));
}

TEST(ParseUrl, ProxyDoubleColonInUrl) {
  EXPECT_TRUE(TestMessage(UrlParserTestMessage::kProxyDoubleColonInUrl));
}

TEST(ParseUrl, ProxyBasicAuthWithDoubleColon) {
  EXPECT_TRUE(
      TestMessage(UrlParserTestMessage::kProxyBasicAuthWithDoubleColon));
}

TEST(ParseUrl, LineFeedInUrl) {
  EXPECT_TRUE(TestMessage(UrlParserTestMessage::kLineFeedInUrl));
}

TEST(ParseUrl, ProxyEmptyBasicAuth) {
  EXPECT_TRUE(TestMessage(UrlParserTestMessage::kProxyEmptyBasicAuth));
}

TEST(ParseUrl, ProxyLineFeedInHostname) {
  EXPECT_TRUE(TestMessage(UrlParserTestMessage::kProxyLineFeedInHostname));
}

TEST(ParseUrl, ProxyPercentInHostname) {
  EXPECT_TRUE(TestMessage(UrlParserTestMessage::kProxyPercentInHostname));
}

TEST(ParseUrl, ProxySemicolonInHostname) {
  EXPECT_TRUE(TestMessage(UrlParserTestMessage::kProxySemicolonInHostname));
}

TEST(ParseUrl, ProxyBasicAuthWithUnreservedChars) {
  EXPECT_TRUE(
      TestMessage(UrlParserTestMessage::kProxyBasicAuthWithUnreservedChars));
}

TEST(ParseUrl, ProxyOnlyEmptyBasicAuth) {
  EXPECT_TRUE(TestMessage(UrlParserTestMessage::kProxyOnlyEmptyBasicAuth));
}

TEST(ParseUrl, ProxyOnlyBasicAuth) {
  EXPECT_TRUE(TestMessage(UrlParserTestMessage::kProxyOnlyBasicAuth));
}

TEST(ParseUrl, ProxyEmptyHostname) {
  EXPECT_TRUE(TestMessage(UrlParserTestMessage::kProxyEmptyHostname));
}

TEST(ParseUrl, ProxyEqualInUrl) {
  EXPECT_TRUE(TestMessage(UrlParserTestMessage::kProxyEqualInUrl));
}

TEST(ParseUrl, TabInUrl) {
  EXPECT_TRUE(TestMessage(UrlParserTestMessage::kTabInUrl));
}

TEST(ParseUrl, FormFeedInUrl) {
  EXPECT_TRUE(TestMessage(UrlParserTestMessage::kFormFeedInUrl));
}

}  // namespace pjcore
