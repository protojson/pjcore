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

#ifndef PJCORE_TEST_URL_PARSER_TEST_MESSAGE_H_
#define PJCORE_TEST_URL_PARSER_TEST_MESSAGE_H_

#include <http_parser.h>

namespace pjcore {

struct UrlParserTestMessage {
  const char* name;
  const char* url;
  bool is_connect;
  http_parser_url u;
  bool rv;

  static const UrlParserTestMessage kProxyRequest;
  static const UrlParserTestMessage kProxyRequestWithPort;
  static const UrlParserTestMessage kConnectRequest;
  static const UrlParserTestMessage kConnectRequestButNotConnect;
  static const UrlParserTestMessage kProxyIpv6Request;
  static const UrlParserTestMessage kProxyIpv6RequestWithPort;
  static const UrlParserTestMessage kConnectIpv6Request;
  static const UrlParserTestMessage kIpv4InIpv6Request;
  static const UrlParserTestMessage kExtractQuestionMarkInQueryString;
  static const UrlParserTestMessage kSpaceUrlEncodedRequest;
  static const UrlParserTestMessage kUrlFragment;
  static const UrlParserTestMessage kComplexUrlFragment;
  static const UrlParserTestMessage kComplexUrlFromNodeJs;
  static const UrlParserTestMessage kComplexUrlWithBasicAuth;
  static const UrlParserTestMessage kDoubleAtSign;
  static const UrlParserTestMessage kProxyEmptyHost;
  static const UrlParserTestMessage kProxyEmptyPort;
  static const UrlParserTestMessage kConnectWithBasicAuth;
  static const UrlParserTestMessage kConnectEmptyHost;
  static const UrlParserTestMessage kConnectEmptyPort;
  static const UrlParserTestMessage kConnectWithExtraBits;
  static const UrlParserTestMessage kSpaceInUrl;
  static const UrlParserTestMessage kProxyBasicAuthWithSpaceUrlEncoded;
  static const UrlParserTestMessage kCarriageReturnInUrl;
  static const UrlParserTestMessage kProxyDoubleColonInUrl;
  static const UrlParserTestMessage kProxyBasicAuthWithDoubleColon;
  static const UrlParserTestMessage kLineFeedInUrl;
  static const UrlParserTestMessage kProxyEmptyBasicAuth;
  static const UrlParserTestMessage kProxyLineFeedInHostname;
  static const UrlParserTestMessage kProxyPercentInHostname;
  static const UrlParserTestMessage kProxySemicolonInHostname;
  static const UrlParserTestMessage kProxyBasicAuthWithUnreservedChars;
  static const UrlParserTestMessage kProxyOnlyEmptyBasicAuth;
  static const UrlParserTestMessage kProxyOnlyBasicAuth;
  static const UrlParserTestMessage kProxyEmptyHostname;
  static const UrlParserTestMessage kProxyEqualInUrl;
  static const UrlParserTestMessage kTabInUrl;
  static const UrlParserTestMessage kFormFeedInUrl;
};

}  // namespace pjcore

#endif  // PJCORE_TEST_URL_PARSER_TEST_MESSAGE_H_
