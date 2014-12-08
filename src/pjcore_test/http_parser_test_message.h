// Modification of test.c
// From https://github.com/joyent/http-parser/blob/master/test.c
// Original contains the following copyright, notice and disclaimer:

/* Copyright Joyent, Inc. and other Node contributors. All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#ifndef PJCORE_TEST_HTTP_PARSER_TEST_MESSAGE_H_
#define PJCORE_TEST_HTTP_PARSER_TEST_MESSAGE_H_

#include <http_parser.h>

#include "pjcore/third_party/chromium/string_piece.h"

namespace pjcore {

struct HttpParserTestMessage {
  static const size_t kMaxHeaders = 13;

  const char *name;  // for debugging purposes
  const char *raw;
  const http_parser_type type;
  const http_method method;
  const int status_code;
  const char *response_status;
  const char *request_path;
  const char *request_url;
  const char *fragment;
  const char *query_string;
  const char *body;
  const size_t body_size;
  const char *host;
  const char *userinfo;
  const uint16_t port;
  const int num_headers;
  const char *headers[kMaxHeaders][2];
  const bool should_keep_alive;

  const char *upgrade;  // upgraded body

  const uint16_t http_major;
  const uint16_t http_minor;

  const bool message_complete_on_eof;

  static const http_method kMissingMethod;
  static const int kMissingStatusCode;
  static const char *kMissingResponseStatus;
  static const char *kMissingRequestPath;
  static const char *kMissingRequestUrl;
  static const char *kMissingFragment;
  static const char *kMissingQueryString;
  static const char *kMissingBody;
  static const size_t kMissingBodySize;
  static const char *kMissingHost;
  static const char *kMissingUserinfo;
  static const uint16_t kMissingPort;

  static const char *kMissingUpgrade;  // upgraded body

  static const uint16_t kMissingHttpMajor;
  static const uint16_t kMissingHttpMinor;

  // Requests
  static const HttpParserTestMessage kCurlGet;
  static const HttpParserTestMessage kFirefoxGet;
  static const HttpParserTestMessage kPlainyogurt;
  static const HttpParserTestMessage kFragmentInUri;
  static const HttpParserTestMessage kGetNoHeadersNoBody;
  static const HttpParserTestMessage kGetOneHeaderNoBody;
  static const HttpParserTestMessage kGetFunkyContentLength;
  static const HttpParserTestMessage kPostIdentityBodyWorld;
  static const HttpParserTestMessage kPostChunkedAllYourBase;
  static const HttpParserTestMessage kTwoChunksMultZeroEnd;
  static const HttpParserTestMessage kChunkedWTrailingHeaders;
  static const HttpParserTestMessage kChunkedWCupcakeAfterLength;
  static const HttpParserTestMessage kWithQuotes;
  static const HttpParserTestMessage kApachebenchGet;
  static const HttpParserTestMessage kQueryUrlWithQuestionMarkGet;
  static const HttpParserTestMessage kPrefixNewlineGet;
  static const HttpParserTestMessage kUpgradeRequest;
  static const HttpParserTestMessage kConnectRequest;
  static const HttpParserTestMessage kReportReq;
  static const HttpParserTestMessage kNoHttpVersion;
  static const HttpParserTestMessage kMsearchReq;
  static const HttpParserTestMessage kLineFoldingInHeader;
  static const HttpParserTestMessage kQueryTerminatedHost;
  static const HttpParserTestMessage kQueryTerminatedHostport;
  static const HttpParserTestMessage kSpaceTerminatedHostport;
  static const HttpParserTestMessage kPatchReq;
  static const HttpParserTestMessage kConnectCapsRequest;
  static const HttpParserTestMessage kUtf8PathReq;
  static const HttpParserTestMessage kHostnameUnderscore;
  static const HttpParserTestMessage kEatTrailingCrlfNoConnectionClose;
  static const HttpParserTestMessage kEatTrailingCrlfWithConnectionClose;
  static const HttpParserTestMessage kPurgeReq;
  static const HttpParserTestMessage kSearchReq;
  static const HttpParserTestMessage kProxyWithBasicAuth;
  static const HttpParserTestMessage kLineFoldingInHeaderWithLf;

  // Responses
  static const HttpParserTestMessage kGoogle301;
  static const HttpParserTestMessage kNoContentLengthResponse;
  static const HttpParserTestMessage kNoHeadersNoBody404;
  static const HttpParserTestMessage kNoReasonPhrase;
  static const HttpParserTestMessage kTrailingSpaceOnChunkedBody;
  static const HttpParserTestMessage kNoCarriageRet;
  static const HttpParserTestMessage kProxyConnection;
  static const HttpParserTestMessage kUnderstoreHeaderKey;
  static const HttpParserTestMessage kBonjourMadameFr;
  static const HttpParserTestMessage kResFieldUnderscore;
  static const HttpParserTestMessage kNonAsciiInStatusLine;
  static const HttpParserTestMessage kHttpVersion09;
  static const HttpParserTestMessage kNoContentLengthNoTransferEncodingResponse;
  static const HttpParserTestMessage kNoBodyHttp10Ka200;
  static const HttpParserTestMessage kNoBodyHttp10Ka204;
  static const HttpParserTestMessage kNoBodyHttp11Ka200;
  static const HttpParserTestMessage kNoBodyHttp11Ka204;
  static const HttpParserTestMessage kNoBodyHttp11Noka204;
  static const HttpParserTestMessage kNoBodyHttp11KaChunked200;
  static const HttpParserTestMessage kSpaceInFieldRes;
  static const HttpParserTestMessage kAmazonCom;
  static const HttpParserTestMessage kEmptyReasonPhraseAfterSpace;

  static StringPiece GetFirstHalfCurlGetRaw();
  static StringPiece GetSecondHalfCurlGetRaw();
};

}  // namespace pjcore

#endif  // PJCORE_TEST_HTTP_PARSER_TEST_MESSAGE_H_
