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

#include "pjcore/http_parser_plus_plus.h"

#include <gtest/gtest.h>

#include "pjcore_test/http_parser_test_message.h"

#include "pjcore/error_util.h"
#include "pjcore/json_writer.h"
#include "pjcore/logging.h"

namespace pjcore {

using ::testing::AssertionResult;
using ::testing::AssertionSuccess;
using ::testing::AssertionFailure;

namespace {

#define VERIFY_MESSAGE_FIELD(_MESSAGE_FIELD, _DEFAULT, _ACTUAL_OBJECT,         \
                             _ACTUAL_FIELD)                                    \
  do {                                                                         \
    if (message._MESSAGE_FIELD == HttpParserTestMessage::_DEFAULT) {           \
      if (_ACTUAL_OBJECT.has_##_ACTUAL_FIELD()) {                              \
        return AssertionFailure() << "Unexpected " #_MESSAGE_FIELD ": "        \
                                  << _ACTUAL_OBJECT._ACTUAL_FIELD();           \
      }                                                                        \
    } else {                                                                   \
      if (!_ACTUAL_OBJECT.has_##_ACTUAL_FIELD()) {                             \
        return AssertionFailure() << "Expected " #_MESSAGE_FIELD;              \
      } else {                                                                 \
        if (_ACTUAL_OBJECT._ACTUAL_FIELD() != message._MESSAGE_FIELD) {        \
          return AssertionFailure()                                            \
                 << "Expected " #_MESSAGE_FIELD ": " << message._MESSAGE_FIELD \
                 << ", actual: " << _ACTUAL_OBJECT._ACTUAL_FIELD();            \
        }                                                                      \
      }                                                                        \
    }                                                                          \
  } while (false)

AssertionResult VerifyHeaders(
    const HttpParserTestMessage& message,
    const google::protobuf::RepeatedPtrField<HttpHeader>& headers) {
  if (headers.size() != message.num_headers) {
    return AssertionFailure() << "Expected num_headers: " << message.num_headers
                              << ", actual: " << headers.size();
  }

  for (int index = 0; index < message.num_headers; ++index) {
    if (headers.Get(index).name() != message.headers[index][0]) {
      return AssertionFailure() << "Expected headers[" << index
                                << "].name: " << message.headers[index][0]
                                << ", actual: " << headers.Get(index).name();
    }

    if (headers.Get(index).value() != message.headers[index][1]) {
      return AssertionFailure() << "Expected headers[" << index
                                << "].value: " << message.headers[index][1]
                                << ", actual: " << headers.Get(index).value();
    }
  }

  return AssertionSuccess();
}

AssertionResult VerifyRequest(const HttpParserTestMessage& message,
                              const HttpRequest& request) {
  VERIFY_MESSAGE_FIELD(method, kMissingMethod, request, method);

  VERIFY_MESSAGE_FIELD(request_url, kMissingRequestUrl, request, url);

  VERIFY_MESSAGE_FIELD(userinfo, kMissingUserinfo, request.parsed_url(),
                       userinfo);
  VERIFY_MESSAGE_FIELD(host, kMissingHost, request.parsed_url(), host);
  VERIFY_MESSAGE_FIELD(port, kMissingPort, request.parsed_url(), port);
  VERIFY_MESSAGE_FIELD(request_path, kMissingRequestPath, request.parsed_url(),
                       path);
  VERIFY_MESSAGE_FIELD(query_string, kMissingQueryString, request.parsed_url(),
                       query);
  VERIFY_MESSAGE_FIELD(fragment, kMissingFragment, request.parsed_url(),
                       fragment);

  VERIFY_MESSAGE_FIELD(http_major, kMissingHttpMajor, request.http_version(),
                       major);
  VERIFY_MESSAGE_FIELD(http_minor, kMissingHttpMinor, request.http_version(),
                       minor);

  VERIFY_MESSAGE_FIELD(body, kMissingBody, request, content);

  if (!request.has_should_keep_alive()) {
    return AssertionFailure()
           << "Expected should_keep_alive: " << message.should_keep_alive;
  }
  if (request.should_keep_alive() != message.should_keep_alive) {
    return AssertionFailure()
           << "Expected should_keep_alive: " << message.should_keep_alive
           << ", actual: " << request.should_keep_alive();
  }

  return VerifyHeaders(message, request.headers());
}

AssertionResult VerifyResponse(const HttpParserTestMessage& message,
                               const HttpResponse& response) {
  VERIFY_MESSAGE_FIELD(http_major, kMissingHttpMajor, response.http_version(),
                       major);
  VERIFY_MESSAGE_FIELD(http_minor, kMissingHttpMinor, response.http_version(),
                       minor);

  VERIFY_MESSAGE_FIELD(status_code, kMissingStatusCode, response, status_code);

  if (!response.has_should_keep_alive()) {
    return AssertionFailure()
           << "Expected should_keep_alive: " << message.should_keep_alive;
  }
  if (response.should_keep_alive() != message.should_keep_alive) {
    return AssertionFailure()
           << "Expected should_keep_alive: " << message.should_keep_alive
           << ", actual: " << response.should_keep_alive();
  }

  return VerifyHeaders(message, response.headers());
}

AssertionResult TestMessage(const HttpParserTestMessage& message) {
  PJCORE_CHECK(message.type == HTTP_REQUEST || message.type == HTTP_RESPONSE);

  StringPiece raw(message.raw);

  for (size_t split = 0; split < raw.length(); ++split) {
    HttpParserPlusPlus parser(NULL, message.type == HTTP_REQUEST
                                        ? HTTP_PARSER_REQUEST
                                        : HTTP_PARSER_RESPONSE);

    size_t offset;
    Error error;

    if (split) {
      if (!parser.Read(raw.substr(0, split), &offset, &error)) {
        return AssertionFailure() << "Failed to read to split offset " << split
                                  << ": " << ErrorToString(error);
      }

      if (offset != split) {
        return AssertionFailure()
               << "Unexpected offset for read to split offset " << split
               << ", actual: " << offset << ", expected: " << split;
      }
    }

    if (!parser.Read(raw.substr(split), &offset, &error)) {
      return AssertionFailure() << "Failed to read from split offset " << split
                                << ": " << ErrorToString(error);
    }

    if (offset != raw.length() - split) {
      return AssertionFailure()
             << "Unexpected offset for read from split offset " << split
             << ", actual: " << offset
             << ", expected: " << raw.length() - split;
    }

    if (message.message_complete_on_eof) {
      if (!parser.Read(StringPiece(), &offset, &error)) {
        return AssertionFailure()
               << "Failed to handle EOF: " << ErrorToString(error);
      }

      if (offset != 0) {
        return AssertionFailure()
               << "Unexpected offset for EOF read: " << offset;
      }
    }

    if (message.type == HTTP_REQUEST) {
      scoped_ptr<HttpRequest> request(parser.NextRequest());
      if (!request) {
        return AssertionFailure() << "Expected to read one HTTP request";
      }
      if (parser.NextRequest()) {
        return AssertionFailure() << "More than one HTTP request read";
      }
      AssertionResult result = VerifyRequest(message, *request.get());
      if (!result) {
        return result;
      }
    } else {
      scoped_ptr<HttpResponse> response(parser.NextResponse());
      if (!response) {
        return AssertionFailure() << "Expected to read one HTTP response";
      }
      if (parser.NextResponse()) {
        return AssertionFailure() << "More than one HTTP response read";
      }
      AssertionResult result = VerifyResponse(message, *response.get());
      if (!result) {
        return result;
      }
    }
  }

  return AssertionSuccess();
}

}  // unnamed namespace

// Requests
TEST(HttpParserPlusPlusTest, CurlGet) {
  EXPECT_TRUE(TestMessage(HttpParserTestMessage::kCurlGet));
}

TEST(HttpParserPlusPlusTest, FirefoxGet) {
  EXPECT_TRUE(TestMessage(HttpParserTestMessage::kFirefoxGet));
}

TEST(HttpParserPlusPlusTest, Plainyogurt) {
  EXPECT_TRUE(TestMessage(HttpParserTestMessage::kPlainyogurt));
}

TEST(HttpParserPlusPlusTest, FragmentInUri) {
  EXPECT_TRUE(TestMessage(HttpParserTestMessage::kFragmentInUri));
}

TEST(HttpParserPlusPlusTest, GetNoHeadersNoBody) {
  EXPECT_TRUE(TestMessage(HttpParserTestMessage::kGetNoHeadersNoBody));
}

TEST(HttpParserPlusPlusTest, GetOneHeaderNoBody) {
  EXPECT_TRUE(TestMessage(HttpParserTestMessage::kGetOneHeaderNoBody));
}

TEST(HttpParserPlusPlusTest, GetFunkyContentLength) {
  EXPECT_TRUE(TestMessage(HttpParserTestMessage::kGetFunkyContentLength));
}

TEST(HttpParserPlusPlusTest, PostIdentityBodyWorld) {
  EXPECT_TRUE(TestMessage(HttpParserTestMessage::kPostIdentityBodyWorld));
}

TEST(HttpParserPlusPlusTest, PostChunkedAllYourBase) {
  EXPECT_TRUE(TestMessage(HttpParserTestMessage::kPostChunkedAllYourBase));
}

TEST(HttpParserPlusPlusTest, TwoChunksMultZeroEnd) {
  EXPECT_TRUE(TestMessage(HttpParserTestMessage::kTwoChunksMultZeroEnd));
}

TEST(HttpParserPlusPlusTest, ChunkedWTrailingHeaders) {
  EXPECT_TRUE(TestMessage(HttpParserTestMessage::kChunkedWTrailingHeaders));
}

TEST(HttpParserPlusPlusTest, ChunkedWCupcakeAfterLength) {
  EXPECT_TRUE(TestMessage(HttpParserTestMessage::kChunkedWCupcakeAfterLength));
}

TEST(HttpParserPlusPlusTest, WithQuotes) {
  EXPECT_TRUE(TestMessage(HttpParserTestMessage::kWithQuotes));
}

TEST(HttpParserPlusPlusTest, ApachebenchGet) {
  EXPECT_TRUE(TestMessage(HttpParserTestMessage::kApachebenchGet));
}

TEST(HttpParserPlusPlusTest, QueryUrlWithQuestionMarkGet) {
  EXPECT_TRUE(TestMessage(HttpParserTestMessage::kQueryUrlWithQuestionMarkGet));
}

TEST(HttpParserPlusPlusTest, PrefixNewlineGet) {
  EXPECT_TRUE(TestMessage(HttpParserTestMessage::kPrefixNewlineGet));
}

// TODO(pjcore): support Upgrade header
// http://en.wikipedia.org/wiki/HTTP/1.1_Upgrade_header
#if 0
TEST(HttpParserPlusPlusTest, UpgradeRequest) {
  EXPECT_TRUE(TestMessage(HttpParserTestMessage::kUpgradeRequest));
}
#endif

// TODO(pjcore): support CONNECT method
// http://en.wikipedia.org/wiki/HTTP_tunnel#HTTP_CONNECT_tunneling
#if 0
TEST(HttpParserPlusPlusTest, ConnectRequest) {
  EXPECT_TRUE(TestMessage(HttpParserTestMessage::kConnectRequest));
}
#endif

TEST(HttpParserPlusPlusTest, ReportReq) {
  EXPECT_TRUE(TestMessage(HttpParserTestMessage::kReportReq));
}

TEST(HttpParserPlusPlusTest, NoHttpVersion) {
  EXPECT_TRUE(TestMessage(HttpParserTestMessage::kNoHttpVersion));
}

TEST(HttpParserPlusPlusTest, MsearchReq) {
  EXPECT_TRUE(TestMessage(HttpParserTestMessage::kMsearchReq));
}

TEST(HttpParserPlusPlusTest, LineFoldingInHeader) {
  EXPECT_TRUE(TestMessage(HttpParserTestMessage::kLineFoldingInHeader));
}

TEST(HttpParserPlusPlusTest, QueryTerminatedHost) {
  EXPECT_TRUE(TestMessage(HttpParserTestMessage::kQueryTerminatedHost));
}

TEST(HttpParserPlusPlusTest, QueryTerminatedHostport) {
  EXPECT_TRUE(TestMessage(HttpParserTestMessage::kQueryTerminatedHostport));
}

TEST(HttpParserPlusPlusTest, SpaceTerminatedHostport) {
  EXPECT_TRUE(TestMessage(HttpParserTestMessage::kSpaceTerminatedHostport));
}

TEST(HttpParserPlusPlusTest, PatchReq) {
  EXPECT_TRUE(TestMessage(HttpParserTestMessage::kPatchReq));
}

// TODO(pjcore): support CONNECT method
// http://en.wikipedia.org/wiki/HTTP_tunnel#HTTP_CONNECT_tunneling
#if 0
TEST(HttpParserPlusPlusTest, ConnectCapsRequest) {
  EXPECT_TRUE(TestMessage(HttpParserTestMessage::kConnectCapsRequest));
}

TEST(HttpParserPlusPlusTest, HostnameUnderscore) {
  EXPECT_TRUE(TestMessage(HttpParserTestMessage::kHostnameUnderscore));
}
#endif

#if !HTTP_PARSER_STRICT
TEST(HttpParserPlusPlusTest, Utf8PathReq) {
  EXPECT_TRUE(TestMessage(HttpParserTestMessage::kUtf8PathReq));
}

#endif

TEST(HttpParserPlusPlusTest, EatTrailingCrlfNoConnectionClose) {
  EXPECT_TRUE(
      TestMessage(HttpParserTestMessage::kEatTrailingCrlfNoConnectionClose));
}

TEST(HttpParserPlusPlusTest, EatTrailingCrlfWithConnectionClose) {
  EXPECT_TRUE(
      TestMessage(HttpParserTestMessage::kEatTrailingCrlfWithConnectionClose));
}

TEST(HttpParserPlusPlusTest, PurgeReq) {
  EXPECT_TRUE(TestMessage(HttpParserTestMessage::kPurgeReq));
}

TEST(HttpParserPlusPlusTest, SearchReq) {
  EXPECT_TRUE(TestMessage(HttpParserTestMessage::kSearchReq));
}

TEST(HttpParserPlusPlusTest, ProxyWithBasicAuth) {
  EXPECT_TRUE(TestMessage(HttpParserTestMessage::kProxyWithBasicAuth));
}

TEST(HttpParserPlusPlusTest, LineFoldingInHeaderWithLf) {
  EXPECT_TRUE(TestMessage(HttpParserTestMessage::kLineFoldingInHeaderWithLf));
}

// Responses
TEST(HttpParserPlusPlusTest, Google301) {
  EXPECT_TRUE(TestMessage(HttpParserTestMessage::kGoogle301));
}

TEST(HttpParserPlusPlusTest, NoContentLengthResponse) {
  EXPECT_TRUE(TestMessage(HttpParserTestMessage::kNoContentLengthResponse));
}

TEST(HttpParserPlusPlusTest, NoHeadersNoBody404) {
  EXPECT_TRUE(TestMessage(HttpParserTestMessage::kNoHeadersNoBody404));
}

TEST(HttpParserPlusPlusTest, NoReasonPhrase) {
  EXPECT_TRUE(TestMessage(HttpParserTestMessage::kNoReasonPhrase));
}

TEST(HttpParserPlusPlusTest, TrailingSpaceOnChunkedBody) {
  EXPECT_TRUE(TestMessage(HttpParserTestMessage::kTrailingSpaceOnChunkedBody));
}

TEST(HttpParserPlusPlusTest, NoCarriageRet) {
  EXPECT_TRUE(TestMessage(HttpParserTestMessage::kNoCarriageRet));
}

TEST(HttpParserPlusPlusTest, ProxyConnection) {
  EXPECT_TRUE(TestMessage(HttpParserTestMessage::kProxyConnection));
}

TEST(HttpParserPlusPlusTest, UnderstoreHeaderKey) {
  EXPECT_TRUE(TestMessage(HttpParserTestMessage::kUnderstoreHeaderKey));
}

TEST(HttpParserPlusPlusTest, BonjourMadameFr) {
  EXPECT_TRUE(TestMessage(HttpParserTestMessage::kBonjourMadameFr));
}

TEST(HttpParserPlusPlusTest, ResFieldUnderscore) {
  EXPECT_TRUE(TestMessage(HttpParserTestMessage::kResFieldUnderscore));
}

TEST(HttpParserPlusPlusTest, NonAsciiInStatusLine) {
  EXPECT_TRUE(TestMessage(HttpParserTestMessage::kNonAsciiInStatusLine));
}

TEST(HttpParserPlusPlusTest, HttpVersion09) {
  EXPECT_TRUE(TestMessage(HttpParserTestMessage::kHttpVersion09));
}

TEST(HttpParserPlusPlusTest, NoContentLengthNoTransferEncodingResponse) {
  EXPECT_TRUE(TestMessage(
      HttpParserTestMessage::kNoContentLengthNoTransferEncodingResponse));
}

TEST(HttpParserPlusPlusTest, NoBodyHttp10Ka200) {
  EXPECT_TRUE(TestMessage(HttpParserTestMessage::kNoBodyHttp10Ka200));
}

TEST(HttpParserPlusPlusTest, NoBodyHttp10Ka204) {
  EXPECT_TRUE(TestMessage(HttpParserTestMessage::kNoBodyHttp10Ka204));
}

TEST(HttpParserPlusPlusTest, NoBodyHttp11Ka200) {
  EXPECT_TRUE(TestMessage(HttpParserTestMessage::kNoBodyHttp11Ka200));
}

TEST(HttpParserPlusPlusTest, NoBodyHttp11Ka204) {
  EXPECT_TRUE(TestMessage(HttpParserTestMessage::kNoBodyHttp11Ka204));
}

TEST(HttpParserPlusPlusTest, NoBodyHttp11Noka204) {
  EXPECT_TRUE(TestMessage(HttpParserTestMessage::kNoBodyHttp11Noka204));
}

TEST(HttpParserPlusPlusTest, NoBodyHttp11KaChunked200) {
  EXPECT_TRUE(TestMessage(HttpParserTestMessage::kNoBodyHttp11KaChunked200));
}

#if !HTTP_PARSER_STRICT
TEST(HttpParserPlusPlusTest, SpaceInFieldRes) {
  EXPECT_TRUE(TestMessage(HttpParserTestMessage::kSpaceInFieldRes));
}
#endif

TEST(HttpParserPlusPlusTest, AmazonCom) {
  EXPECT_TRUE(TestMessage(HttpParserTestMessage::kAmazonCom));
}

TEST(HttpParserPlusPlusTest, EmptyReasonPhraseAfterSpace) {
  EXPECT_TRUE(TestMessage(HttpParserTestMessage::kEmptyReasonPhraseAfterSpace));
}

}  // namespace pjcore
