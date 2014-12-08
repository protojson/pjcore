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

#include "pjcore_test/url_parser_test_message.h"

#include <stddef.h>

namespace pjcore {

const UrlParserTestMessage
UrlParserTestMessage::kProxyRequest = {
  "proxy request",  // name
  "http://hostname/",  // url
  false,  // is_connect
  {
    (1 << UF_SCHEMA) | (1 << UF_HOST) | (1 << UF_PATH),  // field_set
    0,  // port
    {
      {  0,  4 },  // UF_SCHEMA
      {  7,  8 },  // UF_HOST
      {  0,  0 },  // UF_PORT
      { 15,  1 },  // UF_PATH
      {  0,  0 },  // UF_QUERY
      {  0,  0 },  // UF_FRAGMENT
      {  0,  0 },  // UF_USERINFO
    }  // field_data
  },  // u
  true,  // rv
};

const UrlParserTestMessage
UrlParserTestMessage::kProxyRequestWithPort = {
  "proxy request with port",  // name
  "http://hostname:444/",  // url
  false,  // is_connect
  {
    (1 << UF_SCHEMA) | (1 << UF_HOST) | (1 << UF_PORT) |
      (1 << UF_PATH),  // field_set
    444,  // port
    {
      {  0,  4 },  // UF_SCHEMA
      {  7,  8 },  // UF_HOST
      { 16,  3 },  // UF_PORT
      { 19,  1 },  // UF_PATH
      {  0,  0 },  // UF_QUERY
      {  0,  0 },  // UF_FRAGMENT
      {  0,  0 },  // UF_USERINFO
    }  // field_data
  },  // u
  true,  // rv
};

const UrlParserTestMessage
UrlParserTestMessage::kConnectRequest = {
  "CONNECT request",  // name
  "hostname:443",  // url
  true,  // is_connect
  {
    (1 << UF_HOST) | (1 << UF_PORT),  // field_set
    443,  // port
    {
      {  0,  0 },  // UF_SCHEMA
      {  0,  8 },  // UF_HOST
      {  9,  3 },  // UF_PORT
      {  0,  0 },  // UF_PATH
      {  0,  0 },  // UF_QUERY
      {  0,  0 },  // UF_FRAGMENT
      {  0,  0 },  // UF_USERINFO
    }  // field_data
  },  // u
  true,  // rv
};

const UrlParserTestMessage
UrlParserTestMessage::kConnectRequestButNotConnect = {
  "CONNECT request but not connect",  // name
  "hostname:443",  // url
  false,  // is_connect
  {},  // u
  false,  // rv
};

const UrlParserTestMessage
UrlParserTestMessage::kProxyIpv6Request = {
  "proxy ipv6 request",  // name
  "http://[1:2::3:4]/",  // url
  false,  // is_connect
  {
    (1 << UF_SCHEMA) | (1 << UF_HOST) | (1 << UF_PATH),  // field_set
    0,  // port
    {
      {  0,  4 },  // UF_SCHEMA
      {  8,  8 },  // UF_HOST
      {  0,  0 },  // UF_PORT
      { 17,  1 },  // UF_PATH
      {  0,  0 },  // UF_QUERY
      {  0,  0 },  // UF_FRAGMENT
      {  0,  0 },  // UF_USERINFO
    }  // field_data
  },  // u
  true,  // rv
};

const UrlParserTestMessage
UrlParserTestMessage::kProxyIpv6RequestWithPort = {
  "proxy ipv6 request with port",  // name
  "http://[1:2::3:4]:67/",  // url
  false,  // is_connect
  {
    (1 << UF_SCHEMA) | (1 << UF_HOST) | (1 << UF_PORT) |
      (1 << UF_PATH),  // field_set
    67,  // port
    {
      {  0,  4 },  // UF_SCHEMA
      {  8,  8 },  // UF_HOST
      { 18,  2 },  // UF_PORT
      { 20,  1 },  // UF_PATH
      {  0,  0 },  // UF_QUERY
      {  0,  0 },  // UF_FRAGMENT
      {  0,  0 },  // UF_USERINFO
    }  // field_data
  },  // u
  true,  // rv
};

const UrlParserTestMessage
UrlParserTestMessage::kConnectIpv6Request = {
  "CONNECT ipv6 address",  // name
  "[1:2::3:4]:443",  // url
  true,  // is_connect
  {
    (1 << UF_HOST) | (1 << UF_PORT),  // field_set
    443,  // port
    {
      {  0,  0 },  // UF_SCHEMA
      {  1,  8 },  // UF_HOST
      { 11,  3 },  // UF_PORT
      {  0,  0 },  // UF_PATH
      {  0,  0 },  // UF_QUERY
      {  0,  0 },  // UF_FRAGMENT
      {  0,  0 },  // UF_USERINFO
    }  // field_data
  },  // u
  true,  // rv
};

const UrlParserTestMessage
UrlParserTestMessage::kIpv4InIpv6Request = {
  "ipv4 in ipv6 address",  // name
  "http://[2001:0000:0000:0000:0000:0000:1.9.1.1]/",  // url
  false,  // is_connect
  {
    (1 << UF_SCHEMA) | (1 << UF_HOST) | (1 << UF_PATH),  // field_set
    0,  // port
    {
      {  0,  4 },  // UF_SCHEMA
      {  8, 37 },  // UF_HOST
      {  0,  0 },  // UF_PORT
      { 46,  1 },  // UF_PATH
      {  0,  0 },  // UF_QUERY
      {  0,  0 },  // UF_FRAGMENT
      {  0,  0 },  // UF_USERINFO
    }  // field_data
  },  // u
  true,  // rv
};

const UrlParserTestMessage
UrlParserTestMessage::kExtractQuestionMarkInQueryString = {
  "extra ? in query string",  // name
  "http://a.tbcdn.cn/p/fp/2010c/??fp-header-min.css,fp-base-min.css,"
    "fp-channel-min.css,fp-product-min.css,fp-mall-min.css,fp-category-min.css,"
    "fp-sub-min.css,fp-gdp4p-min.css,fp-css3-min.css,"
    "fp-misc-min.css?t=20101022.css",  // url
  false,  // is_connect
  {
    (1 << UF_SCHEMA) | (1 << UF_HOST) | (1 << UF_PATH) |
      (1 << UF_QUERY),  // field_set
    0,  // port
    {
      {  0,  4 },  // UF_SCHEMA
      {  7, 10 },  // UF_HOST
      {  0,  0 },  // UF_PORT
      { 17, 12 },  // UF_PATH
      { 30, 187 },  // UF_QUERY
      {  0,  0 },  // UF_FRAGMENT
      {  0,  0 },  // UF_USERINFO
    }  // field_data
  },  // u
  true,  // rv
};

const UrlParserTestMessage
UrlParserTestMessage::kSpaceUrlEncodedRequest = {
  "space URL encoded",  // name
  "/toto.html?toto=a%20b",  // url
  false,  // is_connect
  {
    (1 << UF_PATH) | (1 << UF_QUERY),  // field_set
    0,  // port
    {
      {  0,  0 },  // UF_SCHEMA
      {  0,  0 },  // UF_HOST
      {  0,  0 },  // UF_PORT
      {  0, 10 },  // UF_PATH
      { 11, 10 },  // UF_QUERY
      {  0,  0 },  // UF_FRAGMENT
      {  0,  0 },  // UF_USERINFO
    }  // field_data
  },  // u
  true,  // rv
};


const UrlParserTestMessage
UrlParserTestMessage::kUrlFragment = {
  "URL fragment",  // name
  "/toto.html#titi",  // url
  false,  // is_connect
  {
    (1 << UF_PATH) | (1 << UF_FRAGMENT),  // field_set
    0,  // port
    {
      {  0,  0 },  // UF_SCHEMA
      {  0,  0 },  // UF_HOST
      {  0,  0 },  // UF_PORT
      {  0, 10 },  // UF_PATH
      {  0,  0 },  // UF_QUERY
      { 11,  4 },  // UF_FRAGMENT
      {  0,  0 },  // UF_USERINFO
    }  // field_data
  },  // u
  true,  // rv
};

const UrlParserTestMessage
UrlParserTestMessage::kComplexUrlFragment = {
  "complex URL fragment",  // name
  "http://www.webmasterworld.com/r.cgi?f=21&d=8405&url="
    "http://www.example.com/index.html?foo=bar&hello=world#midpage",  // url
  false,  // is_connect
  {
    (1 << UF_SCHEMA) | (1 << UF_HOST) | (1 << UF_PATH) | (1 << UF_QUERY) |
      (1 << UF_FRAGMENT),  // field_set
    0,  // port
    {
      {  0,  4 },  // UF_SCHEMA
      {  7, 22 },  // UF_HOST
      {  0,  0 },  // UF_PORT
      { 29,  6 },  // UF_PATH
      { 36, 69 },  // UF_QUERY
      {106,  7 },  // UF_FRAGMENT
      {  0,  0 },  // UF_USERINFO
    }  // field_data
  },  // u
  true,  // rv
};

const UrlParserTestMessage
UrlParserTestMessage::kComplexUrlFromNodeJs = {
  "complex URL from node js url parser doc",  // name
  "http://host.com:8080/p/a/t/h?query=string#hash",  // url
  false,  // is_connect
  {
    (1 << UF_SCHEMA) | (1 << UF_HOST) | (1 << UF_PORT) | (1 << UF_PATH) |
       (1 << UF_QUERY) | (1 << UF_FRAGMENT),  // field_set
    8080,  // port
    {
      {  0,  4 },  // UF_SCHEMA
      {  7,  8 },  // UF_HOST
      { 16,  4 },  // UF_PORT
      { 20,  8 },  // UF_PATH
      { 29, 12 },  // UF_QUERY
      { 42,  4 },  // UF_FRAGMENT
      {  0,  0 },  // UF_USERINFO
    }  // field_data
  },  // u
  true,  // rv
};

const UrlParserTestMessage
UrlParserTestMessage::kComplexUrlWithBasicAuth = {
  "complex URL with basic auth from node js url parser doc",  // name
  "http://a:b@host.com:8080/p/a/t/h?query=string#hash",  // url
  false,  // is_connect
  {
    (1 << UF_SCHEMA) | (1 << UF_HOST) | (1 << UF_PORT) | (1 << UF_PATH) |
      (1 << UF_QUERY) | (1 << UF_FRAGMENT) | (1 << UF_USERINFO),  // field_set
    8080,  // port
    {
      {  0,  4 },  // UF_SCHEMA
      { 11,  8 },  // UF_HOST
      { 20,  4 },  // UF_PORT
      { 24,  8 },  // UF_PATH
      { 33, 12 },  // UF_QUERY
      { 46,  4 },  // UF_FRAGMENT
      {  7,  3 },  // UF_USERINFO
    }  // field_data
  },  // u
  true,  // rv
};

const UrlParserTestMessage
UrlParserTestMessage::kDoubleAtSign= {
  "double @",  // name
  "http://a:b@@hostname:443/",  // url
  false,  // is_connect
  {},  // u
  false,  // rv
};

const UrlParserTestMessage
UrlParserTestMessage::kProxyEmptyHost = {
  "proxy empty host",  // name
  "http://:443/",  // url
  false,  // is_connect
  {},  // u
  false,  // rv
};

const UrlParserTestMessage
UrlParserTestMessage::kProxyEmptyPort = {
  "proxy empty port",  // name
  "http://hostname:/",  // url
  false,  // is_connect
  {},  // u
  false,  // rv
};

const UrlParserTestMessage
UrlParserTestMessage::kConnectWithBasicAuth = {
  "CONNECT with basic auth",  // name
  "a:b@hostname:443",  // url
  true,  // is_connect
  {},  // u
  false,  // rv
};

const UrlParserTestMessage
UrlParserTestMessage::kConnectEmptyHost = {
  "CONNECT empty host",  // name
  ":443",  // url
  true,  // is_connect
  {},  // u
  false,  // rv
};

const UrlParserTestMessage
UrlParserTestMessage::kConnectEmptyPort = {
  "CONNECT empty port",  // name
  "hostname:",  // url
  true,  // is_connect
  {},  // u
  false,  // rv
};

const UrlParserTestMessage
UrlParserTestMessage::kConnectWithExtraBits = {
  "CONNECT with extra bits",  // name
  "hostname:443/",  // url
  true,  // is_connect
  {},  // u
  false,  // rv
};

const UrlParserTestMessage
UrlParserTestMessage::kSpaceInUrl = {
  "space in URL",  // name
  "/foo bar/",  // url
  false,  // is_connect
  {},  // u
  false /* s_dead */
};

const UrlParserTestMessage
UrlParserTestMessage::kProxyBasicAuthWithSpaceUrlEncoded = {
  "proxy basic auth with space url encoded",  // name
  "http://a%20:b@host.com/",  // url
  false,  // is_connect
  {
    (1 << UF_SCHEMA) | (1 << UF_HOST) | (1 << UF_PATH) |
      (1 << UF_USERINFO),  // field_set
    0,  // port
    {
      {  0,  4 },  // UF_SCHEMA
      { 14,  8 },  // UF_HOST
      {  0,  0 },  // UF_PORT
      { 22,  1 },  // UF_PATH
      {  0,  0 },  // UF_QUERY
      {  0,  0 },  // UF_FRAGMENT
      {  7,  6 },  // UF_USERINFO
    }  // field_data
  },  // u
  true,  // rv
};

const UrlParserTestMessage
UrlParserTestMessage::kCarriageReturnInUrl = {
  "carriage return in URL",  // name
  "/foo\rbar/",  // url
  false,  // is_connect
  {},  // u
  false /* s_dead */
};

const UrlParserTestMessage
UrlParserTestMessage::kProxyDoubleColonInUrl = {
  "proxy double : in URL",  // name
  "http://hostname::443/",  // url
  false,  // is_connect
  {},  // u
  false /* s_dead */
};

const UrlParserTestMessage
UrlParserTestMessage::kProxyBasicAuthWithDoubleColon = {
  "proxy basic auth with double :",  // name
  "http://a::b@host.com/",  // url
  false,  // is_connect
  {
    (1 << UF_SCHEMA) | (1 << UF_HOST) | (1 << UF_PATH) |
      (1 << UF_USERINFO),  // field_set
    0,  // port
    {
      {  0,  4 },  // UF_SCHEMA
      { 12,  8 },  // UF_HOST
      {  0,  0 },  // UF_PORT
      { 20,  1 },  // UF_PATH
      {  0,  0 },  // UF_QUERY
      {  0,  0 },  // UF_FRAGMENT
      {  7,  4 },  // UF_USERINFO
    }  // field_data
  },  // u
  true,  // rv
};

const UrlParserTestMessage
UrlParserTestMessage::kLineFeedInUrl = {
  "line feed in URL",  // name
  "/foo\nbar/",  // url
  false,  // is_connect
  {},  // u
  false /* s_dead */
};

const UrlParserTestMessage
UrlParserTestMessage::kProxyEmptyBasicAuth = {
  "proxy empty basic auth",  // name
  "http://@hostname/fo",  // url
  false,  // is_connect
  {
    (1 << UF_SCHEMA) | (1 << UF_HOST) | (1 << UF_PATH),  // field_set
    0,  // port
    {
      {  0,  4 },  // UF_SCHEMA
      {  8,  8 },  // UF_HOST
      {  0,  0 },  // UF_PORT
      { 16,  3 },  // UF_PATH
      {  0,  0 },  // UF_QUERY
      {  0,  0 },  // UF_FRAGMENT
      {  0,  0 },  // UF_USERINFO
    }  // field_data
  },  // u
  true,  // rv
};

const UrlParserTestMessage
UrlParserTestMessage::kProxyLineFeedInHostname = {
  "proxy line feed in hostname",  // name
  "http://host\name/fo",  // url
  false,  // is_connect
  {},  // u
  false /* s_dead */
};

const UrlParserTestMessage
UrlParserTestMessage::kProxyPercentInHostname = {
  "proxy % in hostname",  // name
  "http://host%name/fo",  // url
  false,  // is_connect
  {},  // u
  false /* s_dead */
};

const UrlParserTestMessage
UrlParserTestMessage::kProxySemicolonInHostname = {
  "proxy ; in hostname",  // name
  "http://host;ame/fo",  // url
  false,  // is_connect
  {},  // u
  false /* s_dead */
};

const UrlParserTestMessage
UrlParserTestMessage::kProxyBasicAuthWithUnreservedChars= {
  "proxy basic auth with unreservedchars",  // name
  "http://a!;-_!=+$@host.com/",  // url
  false,  // is_connect
  {
    (1 << UF_SCHEMA) | (1 << UF_HOST) | (1 << UF_PATH) |
      (1 << UF_USERINFO),  // field_set
    0,  // port
    {
      {  0,  4 },  // UF_SCHEMA
      { 17,  8 },  // UF_HOST
      {  0,  0 },  // UF_PORT
      { 25,  1 },  // UF_PATH
      {  0,  0 },  // UF_QUERY
      {  0,  0 },  // UF_FRAGMENT
      {  7,  9 },  // UF_USERINFO
    }  // field_data
  },  // u
  true,  // rv
};

const UrlParserTestMessage
UrlParserTestMessage::kProxyOnlyEmptyBasicAuth = {
  "proxy only empty basic auth",  // name
  "http://@/fo",  // url
  false,  // is_connect
  {},  // u
  false /* s_dead */
};

const UrlParserTestMessage
UrlParserTestMessage::kProxyOnlyBasicAuth = {
  "proxy only basic auth",  // name
  "http://toto@/fo",  // url
  false,  // is_connect
  {},  // u
  false /* s_dead */
};

const UrlParserTestMessage
UrlParserTestMessage::kProxyEmptyHostname = {
  "proxy emtpy hostname",  // name
  "http:///fo",  // url
  false,  // is_connect
  {},  // u
  false /* s_dead */
};

const UrlParserTestMessage
UrlParserTestMessage::kProxyEqualInUrl = {
  "proxy = in URL",  // name
  "http://host=ame/fo",  // url
  false,  // is_connect
  {},  // u
  false /* s_dead */
};

#if HTTP_PARSER_STRICT

const UrlParserTestMessage
UrlParserTestMessage::kTabInUrl = {
  "tab in URL",  // name
  "/foo\tbar/",  // url
  false,  // is_connect
  {},  // u
  false /* s_dead */
};

const UrlParserTestMessage
UrlParserTestMessage::kFormFeedInUrl = {
  "form feed in URL",  // name
  "/foo\fbar/",  // url
  false,  // is_connect
  {},  // u
  false /* s_dead */
};

#else /* !HTTP_PARSER_STRICT */

const UrlParserTestMessage
UrlParserTestMessage::kTabInUrl = {
  "tab in URL",  // name
  "/foo\tbar/",  // url
  false,  // is_connect
  {
    (1 << UF_PATH),  // field_set
    0,  // port
    {
      {  0,  0 },  // UF_SCHEMA
      {  0,  0 },  // UF_HOST
      {  0,  0 },  // UF_PORT
      {  0,  9 },  // UF_PATH
      {  0,  0 },  // UF_QUERY
      {  0,  0 },  // UF_FRAGMENT
      {  0,  0 },  // UF_USERINFO
    }  // field_data
  },  // u
  true,  // rv
};

const UrlParserTestMessage
UrlParserTestMessage::kFormFeedInUrl = {
  "form feed in URL",  // name
  "/foo\fbar/",  // url
  false,  // is_connect
  {
    (1 << UF_PATH),  // field_set
    0,  // port
    {
      {  0,  0 },  // UF_SCHEMA
      {  0,  0 },  // UF_HOST
      {  0,  0 },  // UF_PORT
      {  0,  9 },  // UF_PATH
      {  0,  0 },  // UF_QUERY
      {  0,  0 },  // UF_FRAGMENT
      {  0,  0 },  // UF_USERINFO
    }  // field_data
  },  // u
  true,  // rv
};
#endif

}  // namespace pjcore
