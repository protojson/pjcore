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

#include "pjcore_test/http_parser_test_message.h"

#include <stddef.h>

namespace pjcore {

const size_t HttpParserTestMessage::kMaxHeaders;

const http_method HttpParserTestMessage::kMissingMethod =
    static_cast<http_method>(-1);
const int HttpParserTestMessage::kMissingStatusCode = -1;
const char *HttpParserTestMessage::kMissingResponseStatus =
    "HttpParserTestMessage::kMissingResponseStatus";
const char *HttpParserTestMessage::kMissingRequestPath =
    "HttpParserTestMessage::kMissingRequestPath";
const char *HttpParserTestMessage::kMissingRequestUrl =
    "HttpParserTestMessage::kMissingRequestUrl";
const char *HttpParserTestMessage::kMissingFragment =
    "HttpParserTestMessage::kMissingFragment";
const char *HttpParserTestMessage::kMissingQueryString =
    "HttpParserTestMessage::kMissingQueryString";
const char *HttpParserTestMessage::kMissingBody =
    "HttpParserTestMessage::kMissingBody";
const size_t HttpParserTestMessage::kMissingBodySize = 123;
const char *HttpParserTestMessage::kMissingHost =
  "HttpParserTestMessage::kMissingHost";
const char *HttpParserTestMessage::kMissingUserinfo =
  "HttpParserTestMessage::kMissingUserinfo";
const uint16_t HttpParserTestMessage::kMissingPort = 456;

const char *HttpParserTestMessage::kMissingUpgrade =
    "HttpParserTestMessage::kMissingUpgrade";

const uint16_t HttpParserTestMessage::kMissingHttpMajor = 123;
const uint16_t HttpParserTestMessage::kMissingHttpMinor = 456;

/* * R E Q U E S T S * */
const HttpParserTestMessage
HttpParserTestMessage::kCurlGet = {
  "curl get",  // name
  "GET /test HTTP/1.1\r\n"
    "User-Agent: curl/7.18.0 (i486-pc-linux-gnu) libcurl/7.18.0 OpenSSL/0.9.8g"
    " zlib/1.2.3.3 libidn/1.1\r\n"
    "Host: 0.0.0.0=5000\r\n"
    "Accept: */*\r\n"
    "\r\n",  // raw
  HTTP_REQUEST,  // type
  HTTP_GET,  // method
  kMissingStatusCode,  // status_code
  kMissingResponseStatus,  // response_status
  "/test",  // request_path
  "/test",  // request_url
  kMissingFragment,  // fragment
  kMissingQueryString,  // query_string
  kMissingBody,  // body
  kMissingBodySize,  // body_size
  kMissingHost,  // host
  kMissingUserinfo,  // userinfo
  kMissingPort,  // port
  3,  // num_headers
  { {"User-Agent", "curl/7.18.0 (i486-pc-linux-gnu) libcurl/7.18.0"
      " OpenSSL/0.9.8g zlib/1.2.3.3 libidn/1.1"},
    {"Host", "0.0.0.0=5000"},
    {"Accept", "*/*"} },  // headers
  true,  // should_keep_alive
  kMissingUpgrade,  // upgrade
  1,  // http_major
  1,  // http_minor
  false,  // message_complete_on_eof
};

const HttpParserTestMessage
HttpParserTestMessage::kFirefoxGet = {
  "firefox get",  // name
  "GET /favicon.ico HTTP/1.1\r\n"
    "Host: 0.0.0.0=5000\r\n"
    "User-Agent: Mozilla/5.0 (X11; U; Linux i686; en-US; rv:1.9)"
    " Gecko/2008061015 Firefox/3.0\r\n"
    "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;"
    "q=0.8\r\n"
    "Accept-Language: en-us,en;q=0.5\r\n"
    "Accept-Encoding: gzip,deflate\r\n"
    "Accept-Charset: ISO-8859-1,utf-8;q=0.7,*;q=0.7\r\n"
    "Keep-Alive: 300\r\n"
    "Connection: keep-alive\r\n"
    "\r\n",  // raw
  HTTP_REQUEST,  // type
  HTTP_GET,  // method
  kMissingStatusCode,  // status_code
  kMissingResponseStatus,  // response_status
  "/favicon.ico",  // request_path
  "/favicon.ico",  // request_url
  kMissingFragment,  // fragment
  kMissingQueryString,  // query_string
  kMissingBody,  // body
  kMissingBodySize,  // body_size
  kMissingHost,  // host
  kMissingUserinfo,  // userinfo
  kMissingPort,  // port
  8,  // num_headers
  { {"Host", "0.0.0.0=5000"},
    {"User-Agent", "Mozilla/5.0 (X11; U; Linux i686; en-US; rv:1.9)"
     " Gecko/2008061015 Firefox/3.0"},
    {"Accept", "text/html,application/xhtml+xml,application/xml;q=0.9,*/*;"
     "q=0.8"},
    {"Accept-Language", "en-us,en;q=0.5"},
    {"Accept-Encoding", "gzip,deflate"},
    {"Accept-Charset", "ISO-8859-1,utf-8;q=0.7,*;q=0.7"},
    {"Keep-Alive", "300"},
    {"Connection", "keep-alive"} },  // headers
  true,  // should_keep_alive
  kMissingUpgrade,  // upgrade
  1,  // http_major
  1,  // http_minor
  false,  // message_complete_on_eof
};

const HttpParserTestMessage
HttpParserTestMessage::kPlainyogurt = {
  "plainyogurt",  // name
  "GET /plainyogurt HTTP/1.1\r\n"
    "aaaaaaaaaaaaa:++++++++++\r\n"
    "\r\n",  // raw
  HTTP_REQUEST,  // type
  HTTP_GET,  // method
  kMissingStatusCode,  // status_code
  kMissingResponseStatus,  // response_status
  "/plainyogurt",  // request_path
  "/plainyogurt",  // request_url
  kMissingFragment,  // fragment
  kMissingQueryString,  // query_string
  kMissingBody,  // body
  kMissingBodySize,  // body_size
  kMissingHost,  // host
  kMissingUserinfo,  // userinfo
  kMissingPort,  // port
  1,  // num_headers
  { {"aaaaaaaaaaaaa",  "++++++++++"} },  // headers
  true,  // should_keep_alive
  kMissingUpgrade,  // upgrade
  1,  // http_major
  1,  // http_minor
  false,  // message_complete_on_eof
};

  /* XXX request url does include fragment? */
const HttpParserTestMessage
HttpParserTestMessage::kFragmentInUri = {
  "fragment in url",  // name
  "GET /forums/1/topics/2375?page=1#posts-17408 HTTP/1.1\r\n"
    "\r\n",  // raw
  HTTP_REQUEST,  // type
  HTTP_GET,  // method
  kMissingStatusCode,  // status_code
  kMissingResponseStatus,  // response_status
  "/forums/1/topics/2375",  // request_path
  "/forums/1/topics/2375?page=1#posts-17408",  // request_url
  "posts-17408",  // fragment
  "page=1",  // query_string
  kMissingBody,  // body
  kMissingBodySize,  // body_size
  kMissingHost,  // host
  kMissingUserinfo,  // userinfo
  kMissingPort,  // port
  0,  // num_headers
  {/*default*/},  // headers
  true,  // should_keep_alive
  kMissingUpgrade,  // upgrade
  1,  // http_major
  1,  // http_minor
  false,  // message_complete_on_eof
};

const HttpParserTestMessage
HttpParserTestMessage::kGetNoHeadersNoBody = {
  "get no headers no body",  // name
  "GET /get_no_headers_no_body/world HTTP/1.1\r\n"
    "\r\n",  // raw
  HTTP_REQUEST,  // type
  HTTP_GET,  // method
  kMissingStatusCode,  // status_code
  kMissingResponseStatus,  // response_status
  "/get_no_headers_no_body/world",  // request_path
  "/get_no_headers_no_body/world",  // request_url
  kMissingFragment,  // fragment
  kMissingQueryString,  // query_string
  kMissingBody,  // body
  kMissingBodySize,  // body_size
  kMissingHost,  // host
  kMissingUserinfo,  // userinfo
  kMissingPort,  // port
  0,  // num_headers
  {/*default*/},  // headers
  true,  // should_keep_alive
  kMissingUpgrade,  // upgrade
  1,  // http_major
  1,  // http_minor
  false /* would need Connection: close */,  // message_complete_on_eof
};

const HttpParserTestMessage
HttpParserTestMessage::kGetOneHeaderNoBody = {
  "get one header no body",  // name
  "GET /get_one_header_no_body HTTP/1.1\r\n"
    "Accept: */*\r\n"
    "\r\n",  // raw
  HTTP_REQUEST,  // type
  HTTP_GET,  // method
  kMissingStatusCode,  // status_code
  kMissingResponseStatus,  // response_status
  "/get_one_header_no_body",  // request_path
  "/get_one_header_no_body",  // request_url
  kMissingFragment,  // fragment
  kMissingQueryString,  // query_string
  kMissingBody,  // body
  kMissingBodySize,  // body_size
  kMissingHost,  // host
  kMissingUserinfo,  // userinfo
  kMissingPort,  // port
  1,  // num_headers
  { {"Accept", "*/*"} },  // headers
  true,  // should_keep_alive
  kMissingUpgrade,  // upgrade
  1,  // http_major
  1,  // http_minor
  false /* would need Connection: close */,  // message_complete_on_eof
};

const HttpParserTestMessage
HttpParserTestMessage::kGetFunkyContentLength = {
  "get funky content length body hello",  // name
  "GET /get_funky_content_length_body_hello HTTP/1.0\r\n"
    "conTENT-Length: 5\r\n"
    "\r\n"
    "HELLO",  // raw
  HTTP_REQUEST,  // type
  HTTP_GET,  // method
  kMissingStatusCode,  // status_code
  kMissingResponseStatus,  // response_status
  "/get_funky_content_length_body_hello",  // request_path
  "/get_funky_content_length_body_hello",  // request_url
  kMissingFragment,  // fragment
  kMissingQueryString,  // query_string
  "HELLO",  // body
  kMissingBodySize,  // body_size
  kMissingHost,  // host
  kMissingUserinfo,  // userinfo
  kMissingPort,  // port
  1,  // num_headers
  { {"conTENT-Length", "5"} },  // headers
  false,  // should_keep_alive
  kMissingUpgrade,  // upgrade
  1,  // http_major
  0,  // http_minor
  false,  // message_complete_on_eof
};

const HttpParserTestMessage
HttpParserTestMessage::kPostIdentityBodyWorld = {
  "post identity body world",  // name
  "POST /post_identity_body_world?q=search#hey HTTP/1.1\r\n"
    "Accept: */*\r\n"
    "Transfer-Encoding: identity\r\n"
    "Content-Length: 5\r\n"
    "\r\n"
    "World",  // raw
  HTTP_REQUEST,  // type
  HTTP_POST,  // method
  kMissingStatusCode,  // status_code
  kMissingResponseStatus,  // response_status
  "/post_identity_body_world",  // request_path
  "/post_identity_body_world?q=search#hey",  // request_url
  "hey",  // fragment
  "q=search",  // query_string
  "World",  // body
  kMissingBodySize,  // body_size
  kMissingHost,  // host
  kMissingUserinfo,  // userinfo
  kMissingPort,  // port
  3,  // num_headers
  { {"Accept", "*/*"},
    {"Transfer-Encoding", "identity"},
    {"Content-Length", "5"} },  // headers
  true,  // should_keep_alive
  kMissingUpgrade,  // upgrade
  1,  // http_major
  1,  // http_minor
  false,  // message_complete_on_eof
};

const HttpParserTestMessage
HttpParserTestMessage::kPostChunkedAllYourBase = {
  "post - chunked body: all your base are belong to us",  // name
  "POST /post_chunked_all_your_base HTTP/1.1\r\n"
    "Transfer-Encoding: chunked\r\n"
    "\r\n"
    "1e\r\nall your base are belong to us\r\n"
    "0\r\n"
    "\r\n",  // raw
  HTTP_REQUEST,  // type
  HTTP_POST,  // method
  kMissingStatusCode,  // status_code
  kMissingResponseStatus,  // response_status
  "/post_chunked_all_your_base",  // request_path
  "/post_chunked_all_your_base",  // request_url
  kMissingFragment,  // fragment
  kMissingQueryString,  // query_string
  "all your base are belong to us",  // body
  kMissingBodySize,  // body_size
  kMissingHost,  // host
  kMissingUserinfo,  // userinfo
  kMissingPort,  // port
  1,  // num_headers
  { {"Transfer-Encoding", "chunked"} },  // headers
  true,  // should_keep_alive
  kMissingUpgrade,  // upgrade
  1,  // http_major
  1,  // http_minor
  false,  // message_complete_on_eof
};

const HttpParserTestMessage
HttpParserTestMessage::kTwoChunksMultZeroEnd = {
  "two chunks ; triple zero ending",  // name
  "POST /two_chunks_mult_zero_end HTTP/1.1\r\n"
    "Transfer-Encoding: chunked\r\n"
    "\r\n"
    "5\r\nhello\r\n"
    "6\r\n world\r\n"
    "000\r\n"
    "\r\n",  // raw
  HTTP_REQUEST,  // type
  HTTP_POST,  // method
  kMissingStatusCode,  // status_code
  kMissingResponseStatus,  // response_status
  "/two_chunks_mult_zero_end",  // request_path
  "/two_chunks_mult_zero_end",  // request_url
  kMissingFragment,  // fragment
  kMissingQueryString,  // query_string
  "hello world",  // body
  kMissingBodySize,  // body_size
  kMissingHost,  // host
  kMissingUserinfo,  // userinfo
  kMissingPort,  // port
  1,  // num_headers
  { {"Transfer-Encoding", "chunked"} },  // headers
  true,  // should_keep_alive
  kMissingUpgrade,  // upgrade
  1,  // http_major
  1,  // http_minor
  false,  // message_complete_on_eof
};

const HttpParserTestMessage
HttpParserTestMessage::kChunkedWTrailingHeaders = {
  "chunked with trailing headers. blech.",  // name
  "POST /chunked_w_trailing_headers HTTP/1.1\r\n"
    "Transfer-Encoding: chunked\r\n"
    "\r\n"
    "5\r\nhello\r\n"
    "6\r\n world\r\n"
    "0\r\n"
    "Vary: *\r\n"
    "Content-Type: text/plain\r\n"
    "\r\n",  // raw
  HTTP_REQUEST,  // type
  HTTP_POST,  // method
  kMissingStatusCode,  // status_code
  kMissingResponseStatus,  // response_status
  "/chunked_w_trailing_headers",  // request_path
  "/chunked_w_trailing_headers",  // request_url
  kMissingFragment,  // fragment
  kMissingQueryString,  // query_string
  "hello world",  // body
  kMissingBodySize,  // body_size
  kMissingHost,  // host
  kMissingUserinfo,  // userinfo
  kMissingPort,  // port
  3,  // num_headers
  { {"Transfer-Encoding",  "chunked"},
    {"Vary", "*"},
    {"Content-Type", "text/plain"} },  // headers
  true,  // should_keep_alive
  kMissingUpgrade,  // upgrade
  1,  // http_major
  1,  // http_minor
  false,  // message_complete_on_eof
};

const HttpParserTestMessage
HttpParserTestMessage::kChunkedWCupcakeAfterLength = {
  "with cupcake after the length",  // name
  "POST /chunked_w_cupcake_after_length HTTP/1.1\r\n"
    "Transfer-Encoding: chunked\r\n"
    "\r\n"
    "5; ihatew3;whattheyogurt=aretheseparametersfor\r\nhello\r\n"
    "6; blahblah; blah\r\n world\r\n"
    "0\r\n"
    "\r\n",  // raw
  HTTP_REQUEST,  // type
  HTTP_POST,  // method
  kMissingStatusCode,  // status_code
  kMissingResponseStatus,  // response_status
  "/chunked_w_cupcake_after_length",  // request_path
  "/chunked_w_cupcake_after_length",  // request_url
  kMissingFragment,  // fragment
  kMissingQueryString,  // query_string
  "hello world",  // body
  kMissingBodySize,  // body_size
  kMissingHost,  // host
  kMissingUserinfo,  // userinfo
  kMissingPort,  // port
  1,  // num_headers
  { {"Transfer-Encoding", "chunked"} },  // headers
  true,  // should_keep_alive
  kMissingUpgrade,  // upgrade
  1,  // http_major
  1,  // http_minor
  false,  // message_complete_on_eof
};

const HttpParserTestMessage
HttpParserTestMessage::kWithQuotes = {
  "with quotes",  // name
  "GET /with_\"stupid\"_quotes?foo=\"bar\" HTTP/1.1\r\n\r\n",  // raw
  HTTP_REQUEST,  // type
  HTTP_GET,  // method
  kMissingStatusCode,  // status_code
  kMissingResponseStatus,  // response_status
  "/with_\"stupid\"_quotes",  // request_path
  "/with_\"stupid\"_quotes?foo=\"bar\"",  // request_url
  kMissingFragment,  // fragment
  "foo=\"bar\"",  // query_string
  kMissingBody,  // body
  kMissingBodySize,  // body_size
  kMissingHost,  // host
  kMissingUserinfo,  // userinfo
  kMissingPort,  // port
  0,  // num_headers
  {},  // headers
  true,  // should_keep_alive
  kMissingUpgrade,  // upgrade
  1,  // http_major
  1,  // http_minor
  false,  // message_complete_on_eof
};

/* The server receiving this request SHOULD NOT wait for EOF
 * to know that content-length == 0.
 * How to represent this in a unit test? message_complete_on_eof
 * Compare with NO_CONTENT_LENGTH_RESPONSE.
 */
const HttpParserTestMessage
HttpParserTestMessage::kApachebenchGet = {
  "apachebench get",  // name
  "GET /test HTTP/1.0\r\n"
    "Host: 0.0.0.0:5000\r\n"
    "User-Agent: ApacheBench/2.3\r\n"
    "Accept: */*\r\n\r\n",  // raw
  HTTP_REQUEST,  // type
  HTTP_GET,  // method
  kMissingStatusCode,  // status_code
  kMissingResponseStatus,  // response_status
  "/test",  // request_path
  "/test",  // request_url
  kMissingFragment,  // fragment
  kMissingQueryString,  // query_string
  kMissingBody,  // body
  kMissingBodySize,  // body_size
  kMissingHost,  // host
  kMissingUserinfo,  // userinfo
  kMissingPort,  // port
  3,  // num_headers
  { {"Host", "0.0.0.0:5000"},
    {"User-Agent", "ApacheBench/2.3"},
    {"Accept", "*/*"} },  // headers
  false,  // should_keep_alive
  kMissingUpgrade,  // upgrade
  1,  // http_major
  0,  // http_minor
  false,  // message_complete_on_eof
};

/* Some clients include '?' characters in query strings.
 */
const HttpParserTestMessage
HttpParserTestMessage::kQueryUrlWithQuestionMarkGet = {
  "query url with question mark",  // name
  "GET /test.cgi?foo=bar?baz HTTP/1.1\r\n\r\n",  // raw
  HTTP_REQUEST,  // type
  HTTP_GET,  // method
  kMissingStatusCode,  // status_code
  kMissingResponseStatus,  // response_status
  "/test.cgi",  // request_path
  "/test.cgi?foo=bar?baz",  // request_url
  kMissingFragment,  // fragment
  "foo=bar?baz",  // query_string
  kMissingBody,  // body
  kMissingBodySize,  // body_size
  kMissingHost,  // host
  kMissingUserinfo,  // userinfo
  kMissingPort,  // port
  0,  // num_headers
  {},  // headers
  true,  // should_keep_alive
  kMissingUpgrade,  // upgrade
  1,  // http_major
  1,  // http_minor
  false,  // message_complete_on_eof
};

/* Some clients, especially after a POST in a keep-alive connection,
 * will send an extra CRLF before the next request
 */
const HttpParserTestMessage
HttpParserTestMessage::kPrefixNewlineGet = {
  "newline prefix get",  // name
  "\r\nGET /test HTTP/1.1\r\n\r\n",  // raw
  HTTP_REQUEST,  // type
  HTTP_GET,  // method
  kMissingStatusCode,  // status_code
  kMissingResponseStatus,  // response_status
  "/test",  // request_path
  "/test",  // request_url
  kMissingFragment,  // fragment
  kMissingQueryString,  // query_string
  kMissingBody,  // body
  kMissingBodySize,  // body_size
  kMissingHost,  // host
  kMissingUserinfo,  // userinfo
  kMissingPort,  // port
  0,  // num_headers
  {},  // headers
  true,  // should_keep_alive
  kMissingUpgrade,  // upgrade
  1,  // http_major
  1,  // http_minor
  false,  // message_complete_on_eof
};

const HttpParserTestMessage
HttpParserTestMessage::kUpgradeRequest = {
  "upgrade request",  // name
  "GET /demo HTTP/1.1\r\n"
    "Host: example.com\r\n"
    "Connection: Upgrade\r\n"
    "Sec-WebSocket-Key2: 12998 5 Y3 1  .P00\r\n"
    "Sec-WebSocket-Protocol: sample\r\n"
    "Upgrade: WebSocket\r\n"
    "Sec-WebSocket-Key1: 4 @1  46546xW%0l 1 5\r\n"
    "Origin: http://example.com\r\n"
    "\r\n"
    "Hot diggity dogg",  // raw
  HTTP_REQUEST,  // type
  HTTP_GET,  // method
  kMissingStatusCode,  // status_code
  kMissingResponseStatus,  // response_status
  "/demo",  // request_path
  "/demo",  // request_url
  kMissingFragment,  // fragment
  kMissingQueryString,  // query_string
  kMissingBody,  // body
  kMissingBodySize,  // body_size
  kMissingHost,  // host
  kMissingUserinfo,  // userinfo
  kMissingPort,  // port
  7,  // num_headers
  { {"Host", "example.com"},
    {"Connection", "Upgrade"},
    {"Sec-WebSocket-Key2", "12998 5 Y3 1  .P00"},
    {"Sec-WebSocket-Protocol", "sample"},
    {"Upgrade", "WebSocket"},
    {"Sec-WebSocket-Key1", "4 @1  46546xW%0l 1 5"},
    {"Origin", "http://example.com"} },  // headers
  true,  // should_keep_alive
  "Hot diggity dogg",  // upgrade
  1,  // http_major
  1,  // http_minor
  false,  // message_complete_on_eof
};

const HttpParserTestMessage
HttpParserTestMessage::kConnectRequest = {
  "connect request",  // name
  "CONNECT 0-home0.netscape.com:443 HTTP/1.0\r\n"
    "User-agent: Mozilla/1.1N\r\n"
    "Proxy-authorization: basic aGVsbG86d29ybGQ=\r\n"
    "\r\n"
    "some data\r\n"
    "and yet even more data",  // raw
  HTTP_REQUEST,  // type
  HTTP_CONNECT,  // method
  kMissingStatusCode,  // status_code
  kMissingResponseStatus,  // response_status
  kMissingRequestPath,  // request_path
  "0-home0.netscape.com:443",  // request_url
  kMissingFragment,  // fragment
  kMissingQueryString,  // query_string
  kMissingBody,  // body
  kMissingBodySize,  // body_size
  kMissingHost,  // host
  kMissingUserinfo,  // userinfo
  kMissingPort,  // port
  2,  // num_headers
  { {"User-agent", "Mozilla/1.1N"},
    {"Proxy-authorization", "basic aGVsbG86d29ybGQ="} },  // headers
  false,  // should_keep_alive
  "some data\r\nand yet even more data",  // upgrade
  1,  // http_major
  0,  // http_minor
  false,  // message_complete_on_eof
};

const HttpParserTestMessage
HttpParserTestMessage::kReportReq = {
  "report request",  // name
  "REPORT /test HTTP/1.1\r\n"
    "\r\n",  // raw
  HTTP_REQUEST,  // type
  HTTP_REPORT,  // method
  kMissingStatusCode,  // status_code
  kMissingResponseStatus,  // response_status
  "/test",  // request_path
  "/test",  // request_url
  kMissingFragment,  // fragment
  kMissingQueryString,  // query_string
  kMissingBody,  // body
  kMissingBodySize,  // body_size
  kMissingHost,  // host
  kMissingUserinfo,  // userinfo
  kMissingPort,  // port
  0,  // num_headers
  {},  // headers
  true,  // should_keep_alive
  kMissingUpgrade,  // upgrade
  1,  // http_major
  1,  // http_minor
  false,  // message_complete_on_eof
};

const HttpParserTestMessage
HttpParserTestMessage::kNoHttpVersion = {
  "request with no http version",  // name
  "GET /\r\n"
    "\r\n",  // raw
  HTTP_REQUEST,  // type
  HTTP_GET,  // method
  kMissingStatusCode,  // status_code
  kMissingResponseStatus,  // response_status
  "/",  // request_path
  "/",  // request_url
  kMissingFragment,  // fragment
  kMissingQueryString,  // query_string
  kMissingBody,  // body
  kMissingBodySize,  // body_size
  kMissingHost,  // host
  kMissingUserinfo,  // userinfo
  kMissingPort,  // port
  0,  // num_headers
  {},  // headers
  false,  // should_keep_alive
  kMissingUpgrade,  // upgrade
  0,  // http_major
  9,  // http_minor
  false,  // message_complete_on_eof
};

const HttpParserTestMessage
HttpParserTestMessage::kMsearchReq = {
  "m-search request",  // name
  "M-SEARCH * HTTP/1.1\r\n"
    "HOST: 239.255.255.250:1900\r\n"
    "MAN: \"ssdp:discover\"\r\n"
    "ST: \"ssdp:all\"\r\n"
    "\r\n",  // raw
  HTTP_REQUEST,  // type
  HTTP_MSEARCH,  // method
  kMissingStatusCode,  // status_code
  kMissingResponseStatus,  // response_status
  "*",  // request_path
  "*",  // request_url
  kMissingFragment,  // fragment
  kMissingQueryString,  // query_string
  kMissingBody,  // body
  kMissingBodySize,  // body_size
  kMissingHost,  // host
  kMissingUserinfo,  // userinfo
  kMissingPort,  // port
  3,  // num_headers
  { {"HOST", "239.255.255.250:1900"},
    {"MAN", "\"ssdp:discover\""},
    {"ST", "\"ssdp:all\""} },  // headers
  true,  // should_keep_alive
  kMissingUpgrade,  // upgrade
  1,  // http_major
  1,  // http_minor
  false,  // message_complete_on_eof
};

const HttpParserTestMessage
HttpParserTestMessage::kLineFoldingInHeader = {
  "line folding in header value",  // name
  "GET / HTTP/1.1\r\n"
    "Line1:   abc\r\n"
    "\tdef\r\n"
    " ghi\r\n"
    "\t\tjkl\r\n"
    "  mno \r\n"
    "\t \tqrs\r\n"
    "Line2: \t line2\t\r\n"
    "Line3:\r\n"
    " line3\r\n"
    "Line4: \r\n"
    " \r\n"
    "Connection:\r\n"
    " close\r\n"
    "\r\n",  // raw
  HTTP_REQUEST,  // type
  HTTP_GET,  // method
  kMissingStatusCode,  // status_code
  kMissingResponseStatus,  // response_status
  "/",  // request_path
  "/",  // request_url
  kMissingFragment,  // fragment
  kMissingQueryString,  // query_string
  kMissingBody,  // body
  kMissingBodySize,  // body_size
  kMissingHost,  // host
  kMissingUserinfo,  // userinfo
  kMissingPort,  // port
  5,  // num_headers
  { {"Line1", "abc\tdef ghi\t\tjkl  mno \t \tqrs"},
    {"Line2", "line2\t"},
    {"Line3", "line3"},
    {"Line4", ""},
    {"Connection", "close"} },  // headers
  false,  // should_keep_alive
  kMissingUpgrade,  // upgrade
  1,  // http_major
  1,  // http_minor
  false,  // message_complete_on_eof
};


const HttpParserTestMessage
HttpParserTestMessage::kQueryTerminatedHost = {
  "host terminated by a query string",  // name
  "GET http://hypnotoad.org?hail=all HTTP/1.1\r\n"
    "\r\n",  // raw
  HTTP_REQUEST,  // type
  HTTP_GET,  // method
  kMissingStatusCode,  // status_code
  kMissingResponseStatus,  // response_status
  kMissingRequestPath,  // request_path
  "http://hypnotoad.org?hail=all",  // request_url
  kMissingFragment,  // fragment
  "hail=all",  // query_string
  kMissingBody,  // body
  kMissingBodySize,  // body_size
  "hypnotoad.org",  // host
  kMissingUserinfo,  // userinfo
  kMissingPort,  // port
  0,  // num_headers
  {},  // headers
  true,  // should_keep_alive
  kMissingUpgrade,  // upgrade
  1,  // http_major
  1,  // http_minor
  false,  // message_complete_on_eof
};

const HttpParserTestMessage
HttpParserTestMessage::kQueryTerminatedHostport = {
  "host:port terminated by a query string",  // name
  "GET http://hypnotoad.org:1234?hail=all HTTP/1.1\r\n"
    "\r\n",  // raw
  HTTP_REQUEST,  // type
  HTTP_GET,  // method
  kMissingStatusCode,  // status_code
  kMissingResponseStatus,  // response_status
  kMissingRequestPath,  // request_path
  "http://hypnotoad.org:1234?hail=all",  // request_url
  kMissingFragment,  // fragment
  "hail=all",  // query_string
  kMissingBody,  // body
  kMissingBodySize,  // body_size
  "hypnotoad.org",  // host
  kMissingUserinfo,  // userinfo
  1234,  // port
  0,  // num_headers
  {},  // headers
  true,  // should_keep_alive
  kMissingUpgrade,  // upgrade
  1,  // http_major
  1,  // http_minor
  false,  // message_complete_on_eof
};

const HttpParserTestMessage
HttpParserTestMessage::kSpaceTerminatedHostport = {
  "host:port terminated by a space",  // name
  "GET http://hypnotoad.org:1234 HTTP/1.1\r\n"
    "\r\n",  // raw
  HTTP_REQUEST,  // type
  HTTP_GET,  // method
  kMissingStatusCode,  // status_code
  kMissingResponseStatus,  // response_status
  kMissingRequestPath,  // request_path
  "http://hypnotoad.org:1234",  // request_url
  kMissingFragment,  // fragment
  kMissingQueryString,  // query_string
  kMissingBody,  // body
  kMissingBodySize,  // body_size
  "hypnotoad.org",  // host
  kMissingUserinfo,  // userinfo
  1234,  // port
  0,  // num_headers
  {},  // headers
  true,  // should_keep_alive
  kMissingUpgrade,  // upgrade
  1,  // http_major
  1,  // http_minor
  false,  // message_complete_on_eof
};

const HttpParserTestMessage
HttpParserTestMessage::kPatchReq = {
  "PATCH request",  // name
  "PATCH /file.txt HTTP/1.1\r\n"
    "Host: www.example.com\r\n"
    "Content-Type: application/example\r\n"
    "If-Match: \"e0023aa4e\"\r\n"
    "Content-Length: 10\r\n"
    "\r\n"
    "cccccccccc",  // raw
  HTTP_REQUEST,  // type
  HTTP_PATCH,  // method
  kMissingStatusCode,  // status_code
  kMissingResponseStatus,  // response_status
  "/file.txt",  // request_path
  "/file.txt",  // request_url
  kMissingFragment,  // fragment
  kMissingQueryString,  // query_string
  "cccccccccc",  // body
  kMissingBodySize,  // body_size
  kMissingHost,  // host
  kMissingUserinfo,  // userinfo
  kMissingPort,  // port
  4,  // num_headers
  { {"Host", "www.example.com"},
    {"Content-Type", "application/example"},
    {"If-Match", "\"e0023aa4e\""},
    {"Content-Length", "10"} },  // headers
  true,  // should_keep_alive
  kMissingUpgrade,  // upgrade
  1,  // http_major
  1,  // http_minor
  false,  // message_complete_on_eof
};

const HttpParserTestMessage
HttpParserTestMessage::kConnectCapsRequest = {
  "connect caps request",  // name
  "CONNECT HOME0.NETSCAPE.COM:443 HTTP/1.0\r\n"
    "User-agent: Mozilla/1.1N\r\n"
    "Proxy-authorization: basic aGVsbG86d29ybGQ=\r\n"
    "\r\n",  // raw
  HTTP_REQUEST,  // type
  HTTP_CONNECT,  // method
  kMissingStatusCode,  // status_code
  kMissingResponseStatus,  // response_status
  kMissingRequestPath,  // request_path
  "HOME0.NETSCAPE.COM:443",  // request_url
  kMissingFragment,  // fragment
  kMissingQueryString,  // query_string
  kMissingBody,  // body
  kMissingBodySize,  // body_size
  kMissingHost,  // host
  kMissingUserinfo,  // userinfo
  kMissingPort,  // port
  2,  // num_headers
  { {"User-agent", "Mozilla/1.1N"},
    {"Proxy-authorization", "basic aGVsbG86d29ybGQ="} },  // headers
  false,  // should_keep_alive
  "",  // upgrade
  1,  // http_major
  0,  // http_minor
  false,  // message_complete_on_eof
};

const HttpParserTestMessage
HttpParserTestMessage::kUtf8PathReq = {
  "utf-8 path request",  // name
  "GET /δ¶/δt/pope?q=1#narf HTTP/1.1\r\n"
    "Host: github.com\r\n"
    "\r\n",  // raw
  HTTP_REQUEST,  // type
  HTTP_GET,  // method
  kMissingStatusCode,  // status_code
  kMissingResponseStatus,  // response_status
  "/δ¶/δt/pope",  // request_path
  "/δ¶/δt/pope?q=1#narf",  // request_url
  "narf",  // fragment
  "q=1",  // query_string
  kMissingBody,  // body
  kMissingBodySize,  // body_size
  kMissingHost,  // host
  kMissingUserinfo,  // userinfo
  kMissingPort,  // port
  1,  // num_headers
  { {"Host", "github.com"} },  // headers
  true,  // should_keep_alive
  kMissingUpgrade,  // upgrade
  1,  // http_major
  1,  // http_minor
  false,  // message_complete_on_eof
};

const HttpParserTestMessage
HttpParserTestMessage::kHostnameUnderscore = {
  "hostname underscore",  // name
  "CONNECT home_0.netscape.com:443 HTTP/1.0\r\n"
    "User-agent: Mozilla/1.1N\r\n"
    "Proxy-authorization: basic aGVsbG86d29ybGQ=\r\n"
    "\r\n",  // raw
  HTTP_REQUEST,  // type
  HTTP_CONNECT,  // method
  kMissingStatusCode,  // status_code
  kMissingResponseStatus,  // response_status
  kMissingRequestPath,  // request_path
  "home_0.netscape.com:443",  // request_url
  kMissingFragment,  // fragment
  kMissingQueryString,  // query_string
  kMissingBody,  // body
  kMissingBodySize,  // body_size
  kMissingHost,  // host
  kMissingUserinfo,  // userinfo
  kMissingPort,  // port
  2,  // num_headers
  { {"User-agent", "Mozilla/1.1N"},
    {"Proxy-authorization", "basic aGVsbG86d29ybGQ="} },  // headers
  false,  // should_keep_alive
  "",  // upgrade
  1,  // http_major
  0,  // http_minor
  false,  // message_complete_on_eof
};

// see https://github.com/ry/http-parser/issues/47
const HttpParserTestMessage
HttpParserTestMessage::kEatTrailingCrlfNoConnectionClose = {
  "eat CRLF between requests, no \"Connection: close\" header",  // name
  "POST / HTTP/1.1\r\n"
    "Host: www.example.com\r\n"
    "Content-Type: application/x-www-form-urlencoded\r\n"
    "Content-Length: 4\r\n"
    "\r\n"
    "q=42\r\n" /* note the trailing CRLF */,  // raw
  HTTP_REQUEST,  // type
  HTTP_POST,  // method
  kMissingStatusCode,  // status_code
  kMissingResponseStatus,  // response_status
  "/",  // request_path
  "/",  // request_url
  kMissingFragment,  // fragment
  kMissingQueryString,  // query_string
  "q=42",  // body
  kMissingBodySize,  // body_size
  kMissingHost,  // host
  kMissingUserinfo,  // userinfo
  kMissingPort,  // port
  3,  // num_headers
  { {"Host", "www.example.com"},
    {"Content-Type", "application/x-www-form-urlencoded"},
    {"Content-Length", "4"} },  // headers
  true,  // should_keep_alive
  0,  // upgrade
  1,  // http_major
  1,  // http_minor
  false,  // message_complete_on_eof
};

// see https://github.com/ry/http-parser/issues/47
const HttpParserTestMessage
HttpParserTestMessage::kEatTrailingCrlfWithConnectionClose = {
  "eat CRLF between requests even if \"Connection: close\" is set",  // name
  "POST / HTTP/1.1\r\n"
    "Host: www.example.com\r\n"
    "Content-Type: application/x-www-form-urlencoded\r\n"
    "Content-Length: 4\r\n"
    "Connection: close\r\n"
    "\r\n"
    "q=42\r\n" /* note the trailing CRLF */,  // raw
  HTTP_REQUEST,  // type
  HTTP_POST,  // method
  kMissingStatusCode,  // status_code
  kMissingResponseStatus,  // response_status
  "/",  // request_path
  "/",  // request_url
  kMissingFragment,  // fragment
  kMissingQueryString,  // query_string
  "q=42",  // body
  kMissingBodySize,  // body_size
  kMissingHost,  // host
  kMissingUserinfo,  // userinfo
  kMissingPort,  // port
  4,  // num_headers
  { {"Host", "www.example.com"},
    {"Content-Type", "application/x-www-form-urlencoded"},
    {"Content-Length", "4"},
    {"Connection", "close"} },  // headers
  false,  // should_keep_alive
  0,  // upgrade
  1,  // http_major
  1,  // http_minor
  /* input buffer isn't empty when on_message_complete is called */
  false,  // message_complete_on_eof
};

const HttpParserTestMessage
HttpParserTestMessage::kPurgeReq = {
  "PURGE request",  // name
  "PURGE /file.txt HTTP/1.1\r\n"
    "Host: www.example.com\r\n"
    "\r\n",  // raw
  HTTP_REQUEST,  // type
  HTTP_PURGE,  // method
  kMissingStatusCode,  // status_code
  kMissingResponseStatus,  // response_status
  "/file.txt",  // request_path
  "/file.txt",  // request_url
  kMissingFragment,  // fragment
  kMissingQueryString,  // query_string
  kMissingBody,  // body
  kMissingBodySize,  // body_size
  kMissingHost,  // host
  kMissingUserinfo,  // userinfo
  kMissingPort,  // port
  1,  // num_headers
  { {"Host", "www.example.com"} },  // headers
  true,  // should_keep_alive
  kMissingUpgrade,  // upgrade
  1,  // http_major
  1,  // http_minor
  false,  // message_complete_on_eof
};

const HttpParserTestMessage
HttpParserTestMessage::kSearchReq = {
  "SEARCH request",  // name
  "SEARCH / HTTP/1.1\r\n"
    "Host: www.example.com\r\n"
    "\r\n",  // raw
  HTTP_REQUEST,  // type
  HTTP_SEARCH,  // method
  kMissingStatusCode,  // status_code
  kMissingResponseStatus,  // response_status
  "/",  // request_path
  "/",  // request_url
  kMissingFragment,  // fragment
  kMissingQueryString,  // query_string
  kMissingBody,  // body
  kMissingBodySize,  // body_size
  kMissingHost,  // host
  kMissingUserinfo,  // userinfo
  kMissingPort,  // port
  1,  // num_headers
  { {"Host", "www.example.com"} },  // headers
  true,  // should_keep_alive
  kMissingUpgrade,  // upgrade
  1,  // http_major
  1,  // http_minor
  false,  // message_complete_on_eof
};

const HttpParserTestMessage
HttpParserTestMessage::kProxyWithBasicAuth = {
  "host:port and basic_auth",  // name
  "GET http://a%12:b!&*$@hypnotoad.org:1234/toto HTTP/1.1\r\n"
    "\r\n",  // raw
  HTTP_REQUEST,  // type
  HTTP_GET,  // method
  kMissingStatusCode,  // status_code
  kMissingResponseStatus,  // response_status
  "/toto",  // request_path
  "http://a%12:b!&*$@hypnotoad.org:1234/toto",  // request_url
  kMissingFragment,  // fragment
  kMissingQueryString,  // query_string
  kMissingBody,  // body
  kMissingBodySize,  // body_size
  "hypnotoad.org",  // host
  "a%12:b!&*$",  // userinfo
  1234,  // port
  0,  // num_headers
  {},  // headers
  true,  // should_keep_alive
  kMissingUpgrade,  // upgrade
  1,  // http_major
  1,  // http_minor
  false,  // message_complete_on_eof
};

const HttpParserTestMessage
HttpParserTestMessage::kLineFoldingInHeaderWithLf = {
  "line folding in header value",  // name
  "GET / HTTP/1.1\n"
    "Line1:   abc\n"
    "\tdef\n"
    " ghi\n"
    "\t\tjkl\n"
    "  mno \n"
    "\t \tqrs\n"
    "Line2: \t line2\t\n"
    "Line3:\n"
    " line3\n"
    "Line4: \n"
    " \n"
    "Connection:\n"
    " close\n"
    "\n",  // raw
  HTTP_REQUEST,  // type
  HTTP_GET,  // method
  kMissingStatusCode,  // status_code
  kMissingResponseStatus,  // response_status
  "/",  // request_path
  "/",  // request_url
  kMissingFragment,  // fragment
  kMissingQueryString,  // query_string
  kMissingBody,  // body
  kMissingBodySize,  // body_size
  kMissingHost,  // host
  kMissingUserinfo,  // userinfo
  kMissingPort,  // port
  5,  // num_headers
  { {"Line1", "abc\tdef ghi\t\tjkl  mno \t \tqrs"},
    {"Line2", "line2\t"},
    {"Line3", "line3"},
    {"Line4", ""},
    {"Connection", "close"} },  // headers
  false,  // should_keep_alive
  kMissingUpgrade,  // upgrade
  1,  // http_major
  1,  // http_minor
  false,  // message_complete_on_eof
};

/* * R E S P O N S E S * */
const HttpParserTestMessage
HttpParserTestMessage::kGoogle301 = {
  "google 301",  // name
  "HTTP/1.1 301 Moved Permanently\r\n"
    "Location: http://www.google.com/\r\n"
    "Content-Type: text/html; charset=UTF-8\r\n"
    "Date: Sun, 26 Apr 2009 11:11:49 GMT\r\n"
    "Expires: Tue, 26 May 2009 11:11:49 GMT\r\n"
    "X-$PrototypeBI-Version: 1.6.0.3\r\n" /* $ char in header field */
    "Cache-Control: public, max-age=2592000\r\n"
    "Server: gws\r\n"
    "Content-Length:  219  \r\n"
    "\r\n"
    "<HTML><HEAD><meta http-equiv=\"content-type\" content=\"text/html;"
    "charset=utf-8\">\n"
    "<TITLE>301 Moved</TITLE></HEAD><BODY>\n"
    "<H1>301 Moved</H1>\n"
    "The document has moved\n"
    "<A HREF=\"http://www.google.com/\">here</A>.\r\n"
    "</BODY></HTML>\r\n",  // raw
  HTTP_RESPONSE,  // type
  kMissingMethod,  // method
  301,  // status_code
  "Moved Permanently",  // response_status
  kMissingRequestPath,  // request_path
  kMissingRequestUrl,  // request_url
  kMissingFragment,  // fragment
  kMissingQueryString,  // query_string
  "<HTML><HEAD><meta http-equiv=\"content-type\" content=\"text/html;"
    "charset=utf-8\">\n"
    "<TITLE>301 Moved</TITLE></HEAD><BODY>\n"
    "<H1>301 Moved</H1>\n"
    "The document has moved\n"
    "<A HREF=\"http://www.google.com/\">here</A>.\r\n"
    "</BODY></HTML>\r\n",  // body
  kMissingBodySize,  // body_size
  kMissingHost,  // host
  kMissingUserinfo,  // userinfo
  kMissingPort,  // port
  8,  // num_headers
  { {"Location", "http://www.google.com/"},
    {"Content-Type", "text/html; charset=UTF-8"},
    {"Date", "Sun, 26 Apr 2009 11:11:49 GMT"},
    {"Expires", "Tue, 26 May 2009 11:11:49 GMT"},
    {"X-$PrototypeBI-Version", "1.6.0.3"},
    {"Cache-Control", "public, max-age=2592000"},
    {"Server", "gws"},
    {"Content-Length", "219  "} },  // headers
  true,  // should_keep_alive
  kMissingUpgrade,  // upgrade
  1,  // http_major
  1,  // http_minor
  false,  // message_complete_on_eof
};

/* The client should wait for the server's EOF. That is, when content-length
 * is not specified, and "Connection: close", the end of body is specified
 * by the EOF.
 * Compare with APACHEBENCH_GET
 */
const HttpParserTestMessage
HttpParserTestMessage::kNoContentLengthResponse = {
  "no content-length response",  // name
  "HTTP/1.1 200 OK\r\n"
    "Date: Tue, 04 Aug 2009 07:59:32 GMT\r\n"
    "Server: Apache\r\n"
    "X-Powered-By: Servlet/2.5 JSP/2.1\r\n"
    "Content-Type: text/xml; charset=utf-8\r\n"
    "Connection: close\r\n"
    "\r\n"
    "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
    "<SOAP-ENV:Envelope xmlns:SOAP-ENV=\"http://schemas.xmlsoap.org/soap"
    "/envelope/\">\n"
    "  <SOAP-ENV:Body>\n"
    "    <SOAP-ENV:Fault>\n"
    "       <faultcode>SOAP-ENV:Client</faultcode>\n"
    "       <faultstring>Client Error</faultstring>\n"
    "    </SOAP-ENV:Fault>\n"
    "  </SOAP-ENV:Body>\n"
    "</SOAP-ENV:Envelope>",  // raw
  HTTP_RESPONSE,  // type
  kMissingMethod,  // method
  200,  // status_code
  "OK",  // response_status
  kMissingRequestPath,  // request_path
  kMissingRequestUrl,  // request_url
  kMissingFragment,  // fragment
  kMissingQueryString,  // query_string
  "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
    "<SOAP-ENV:Envelope xmlns:SOAP-ENV=\"http://schemas.xmlsoap.org/soap"
    "/envelope/\">\n"
    "  <SOAP-ENV:Body>\n"
    "    <SOAP-ENV:Fault>\n"
    "       <faultcode>SOAP-ENV:Client</faultcode>\n"
    "       <faultstring>Client Error</faultstring>\n"
    "    </SOAP-ENV:Fault>\n"
    "  </SOAP-ENV:Body>\n"
    "</SOAP-ENV:Envelope>",  // body
  kMissingBodySize,  // body_size
  kMissingHost,  // host
  kMissingUserinfo,  // userinfo
  kMissingPort,  // port
  5,  // num_headers
  { {"Date", "Tue, 04 Aug 2009 07:59:32 GMT"},
    {"Server", "Apache"},
    {"X-Powered-By", "Servlet/2.5 JSP/2.1"},
    {"Content-Type", "text/xml; charset=utf-8"},
    {"Connection", "close"} },  // headers
  false,  // should_keep_alive
  kMissingUpgrade,  // upgrade
  1,  // http_major
  1,  // http_minor
  true,  // message_complete_on_eof
};

const HttpParserTestMessage
HttpParserTestMessage::kNoHeadersNoBody404 = {
  "404 no headers no body",  // name
  "HTTP/1.1 404 Not Found\r\n\r\n",  // raw
  HTTP_RESPONSE,  // type
  kMissingMethod,  // method
  404,  // status_code
  "Not Found",  // response_status
  kMissingRequestPath,  // request_path
  kMissingRequestUrl,  // request_url
  kMissingFragment,  // fragment
  kMissingQueryString,  // query_string
  "",  // body
  0,  // body_size
  kMissingHost,  // host
  kMissingUserinfo,  // userinfo
  kMissingPort,  // port
  0,  // num_headers
  {},  // headers
  false,  // should_keep_alive
  kMissingUpgrade,  // upgrade
  1,  // http_major
  1,  // http_minor
  true,  // message_complete_on_eof
};

const HttpParserTestMessage
HttpParserTestMessage::kNoReasonPhrase = {
  "301 no response phrase",  // name
  "HTTP/1.1 301\r\n\r\n",  // raw
  HTTP_RESPONSE,  // type
  kMissingMethod,  // method
  301,  // status_code
  "",  // response_status
  kMissingRequestPath,  // request_path
  kMissingRequestUrl,  // request_url
  kMissingFragment,  // fragment
  kMissingQueryString,  // query_string
  "",  // body
  kMissingBodySize,  // body_size
  kMissingHost,  // host
  kMissingUserinfo,  // userinfo
  kMissingPort,  // port
  0,  // num_headers
  {},  // headers
  false,  // should_keep_alive
  kMissingUpgrade,  // upgrade
  1,  // http_major
  1,  // http_minor
  true,  // message_complete_on_eof
};


const HttpParserTestMessage
HttpParserTestMessage::kTrailingSpaceOnChunkedBody = {
  "200 trailing space on chunked body",  // name
  "HTTP/1.1 200 OK\r\n"
    "Content-Type: text/plain\r\n"
    "Transfer-Encoding: chunked\r\n"
    "\r\n"
    "25  \r\n"
    "This is the data in the first chunk\r\n"
    "\r\n"
    "1C\r\n"
    "and this is the second one\r\n"
    "\r\n"
    "0  \r\n"
    "\r\n",  // raw
  HTTP_RESPONSE,  // type
  kMissingMethod,  // method
  200,  // status_code
  "OK",  // response_status
  kMissingRequestPath,  // request_path
  kMissingRequestUrl,  // request_url
  kMissingFragment,  // fragment
  kMissingQueryString,  // query_string
  "This is the data in the first chunk\r\n"
    "and this is the second one\r\n",  // body
  37+28,  // body_size
  kMissingHost,  // host
  kMissingUserinfo,  // userinfo
  kMissingPort,  // port
  2,  // num_headers
  { {"Content-Type", "text/plain"},
    {"Transfer-Encoding", "chunked"} },  // headers
  true,  // should_keep_alive
  kMissingUpgrade,  // upgrade
  1,  // http_major
  1,  // http_minor
  false,  // message_complete_on_eof
};

const HttpParserTestMessage
HttpParserTestMessage::kNoCarriageRet = {
  "no carriage ret",  // name
  "HTTP/1.1 200 OK\n"
    "Content-Type: text/html; charset=utf-8\n"
    "Connection: close\n"
    "\n"
    "these headers are from http://news.ycombinator.com/",  // raw
  HTTP_RESPONSE,  // type
  kMissingMethod,  // method
  200,  // status_code
  "OK",  // response_status
  kMissingRequestPath,  // request_path
  kMissingRequestUrl,  // request_url
  kMissingFragment,  // fragment
  kMissingQueryString,  // query_string
  "these headers are from http://news.ycombinator.com/",  // body
  kMissingBodySize,  // body_size
  kMissingHost,  // host
  kMissingUserinfo,  // userinfo
  kMissingPort,  // port
  2,  // num_headers
  { {"Content-Type", "text/html; charset=utf-8"},
    {"Connection", "close"} },  // headers
  false,  // should_keep_alive
  kMissingUpgrade,  // upgrade
  1,  // http_major
  1,  // http_minor
  true,  // message_complete_on_eof
};

const HttpParserTestMessage
HttpParserTestMessage::kProxyConnection = {
  "proxy connection",  // name
  "HTTP/1.1 200 OK\r\n"
    "Content-Type: text/html; charset=UTF-8\r\n"
    "Content-Length: 11\r\n"
    "Proxy-Connection: close\r\n"
    "Date: Thu, 31 Dec 2009 20:55:48 +0000\r\n"
    "\r\n"
    "hello world",  // raw
  HTTP_RESPONSE,  // type
  kMissingMethod,  // method
  200,  // status_code
  "OK",  // response_status
  kMissingRequestPath,  // request_path
  kMissingRequestUrl,  // request_url
  kMissingFragment,  // fragment
  kMissingQueryString,  // query_string
  "hello world",  // body
  kMissingBodySize,  // body_size
  kMissingHost,  // host
  kMissingUserinfo,  // userinfo
  kMissingPort,  // port
  4,  // num_headers
  { {"Content-Type", "text/html; charset=UTF-8"},
    {"Content-Length", "11"},
    {"Proxy-Connection", "close"},
    {"Date", "Thu, 31 Dec 2009 20:55:48 +0000"} },  // headers
  false,  // should_keep_alive
  kMissingUpgrade,  // upgrade
  1,  // http_major
  1,  // http_minor
  false,  // message_complete_on_eof
};

// shown by curl -o /dev/null
//   -v "http://ad.doubleclick.net/pfadx/DARTSHELLCONFIGXML;dcmt=text/xml;"
const HttpParserTestMessage
HttpParserTestMessage::kUnderstoreHeaderKey = {
  "underscore header key",  // name
  "HTTP/1.1 200 OK\r\n"
    "Server: DCLK-AdSvr\r\n"
    "Content-Type: text/xml\r\n"
    "Content-Length: 0\r\n"
    "DCLK_imp: v7;x;114750856;0-0;0;17820020;0/0;21603567/21621457/1;;~okv=;"
    "dcmt=text/xml;;~cs=o\r\n\r\n",  // raw
  HTTP_RESPONSE,  // type
  kMissingMethod,  // method
  200,  // status_code
  "OK",  // response_status
  kMissingRequestPath,  // request_path
  kMissingRequestUrl,  // request_url
  kMissingFragment,  // fragment
  kMissingQueryString,  // query_string
  "",  // body
  kMissingBodySize,  // body_size
  kMissingHost,  // host
  kMissingUserinfo,  // userinfo
  kMissingPort,  // port
  4,  // num_headers
  { {"Server", "DCLK-AdSvr"},
    {"Content-Type", "text/xml"},
    {"Content-Length", "0"},
    {"DCLK_imp", "v7;x;114750856;0-0;0;17820020;0/0;21603567/21621457/1;;~okv=;"
     "dcmt=text/xml;;~cs=o"} },  // headers
  true,  // should_keep_alive
  kMissingUpgrade,  // upgrade
  1,  // http_major
  1,  // http_minor
  false,  // message_complete_on_eof
};

/* The client should not merge two headers fields when the first one doesn't
 * have a value.
 */
const HttpParserTestMessage
HttpParserTestMessage::kBonjourMadameFr = {
  "bonjourmadame.fr",  // name
  "HTTP/1.0 301 Moved Permanently\r\n"
    "Date: Thu, 03 Jun 2010 09:56:32 GMT\r\n"
    "Server: Apache/2.2.3 (Red Hat)\r\n"
    "Cache-Control: public\r\n"
    "Pragma: \r\n"
    "Location: http://www.bonjourmadame.fr/\r\n"
    "Vary: Accept-Encoding\r\n"
    "Content-Length: 0\r\n"
    "Content-Type: text/html; charset=UTF-8\r\n"
    "Connection: keep-alive\r\n"
    "\r\n",  // raw
  HTTP_RESPONSE,  // type
  kMissingMethod,  // method
  301,  // status_code
  "Moved Permanently",  // response_status
  kMissingRequestPath,  // request_path
  kMissingRequestUrl,  // request_url
  kMissingFragment,  // fragment
  kMissingQueryString,  // query_string
  "",  // body
  kMissingBodySize,  // body_size
  kMissingHost,  // host
  kMissingUserinfo,  // userinfo
  kMissingPort,  // port
  9,  // num_headers
  { {"Date", "Thu, 03 Jun 2010 09:56:32 GMT"},
    {"Server", "Apache/2.2.3 (Red Hat)"},
    {"Cache-Control", "public"},
    {"Pragma", ""},
    {"Location", "http://www.bonjourmadame.fr/"},
    {"Vary",  "Accept-Encoding"},
    {"Content-Length", "0"},
    {"Content-Type", "text/html; charset=UTF-8"},
    {"Connection", "keep-alive"} },  // headers
  true,  // should_keep_alive
  kMissingUpgrade,  // upgrade
  1,  // http_major
  0,  // http_minor
  false,  // message_complete_on_eof
};

/* Should handle spaces in header fields */
const HttpParserTestMessage
HttpParserTestMessage::kResFieldUnderscore = {
  "field underscore",  // name
  "HTTP/1.1 200 OK\r\n"
    "Date: Tue, 28 Sep 2010 01:14:13 GMT\r\n"
    "Server: Apache\r\n"
    "Cache-Control: no-cache, must-revalidate\r\n"
    "Expires: Mon, 26 Jul 1997 05:00:00 GMT\r\n"
    ".et-Cookie: PlaxoCS=1274804622353690521; path=/; domain=.plaxo.com\r\n"
    "Vary: Accept-Encoding\r\n"
    "_eep-Alive: timeout=45\r\n" /* semantic value ignored */
    "_onnection: Keep-Alive\r\n" /* semantic value ignored */
    "Transfer-Encoding: chunked\r\n"
    "Content-Type: text/html\r\n"
    "Connection: close\r\n"
    "\r\n"
    "0\r\n\r\n",  // raw
  HTTP_RESPONSE,  // type
  kMissingMethod,  // method
  200,  // status_code
  "OK",  // response_status
  kMissingRequestPath,  // request_path
  kMissingRequestUrl,  // request_url
  kMissingFragment,  // fragment
  kMissingQueryString,  // query_string
  "",  // body
  kMissingBodySize,  // body_size
  kMissingHost,  // host
  kMissingUserinfo,  // userinfo
  kMissingPort,  // port
  11,  // num_headers
  { {"Date", "Tue, 28 Sep 2010 01:14:13 GMT"},
    {"Server", "Apache"},
    {"Cache-Control", "no-cache, must-revalidate"},
    {"Expires", "Mon, 26 Jul 1997 05:00:00 GMT"},
    {".et-Cookie", "PlaxoCS=1274804622353690521; path=/; domain=.plaxo.com"},
    {"Vary", "Accept-Encoding"},
    {"_eep-Alive", "timeout=45"},
    {"_onnection", "Keep-Alive"},
    {"Transfer-Encoding", "chunked"},
    {"Content-Type", "text/html"},
    {"Connection", "close"} },  // headers
  false,  // should_keep_alive
  kMissingUpgrade,  // upgrade
  1,  // http_major
  1,  // http_minor
  false,  // message_complete_on_eof
};

/* Should handle non-ASCII in status line */
const HttpParserTestMessage
HttpParserTestMessage::kNonAsciiInStatusLine = {
  "non-ASCII in status line",  // name
  "HTTP/1.1 500 Oriëntatieprobleem\r\n"
    "Date: Fri, 5 Nov 2010 23:07:12 GMT+2\r\n"
    "Content-Length: 0\r\n"
    "Connection: close\r\n"
    "\r\n",  // raw
  HTTP_RESPONSE,  // type
  kMissingMethod,  // method
  500,  // status_code
  "Oriëntatieprobleem",  // response_status
  kMissingRequestPath,  // request_path
  kMissingRequestUrl,  // request_url
  kMissingFragment,  // fragment
  kMissingQueryString,  // query_string
  "",  // body
  kMissingBodySize,  // body_size
  kMissingHost,  // host
  kMissingUserinfo,  // userinfo
  kMissingPort,  // port
  3,  // num_headers
  { {"Date", "Fri, 5 Nov 2010 23:07:12 GMT+2"},
    {"Content-Length", "0"},
    {"Connection", "close"} },  // headers
  false,  // should_keep_alive
  kMissingUpgrade,  // upgrade
  1,  // http_major
  1,  // http_minor
  false,  // message_complete_on_eof
};

/* Should handle HTTP/0.9 */
const HttpParserTestMessage
HttpParserTestMessage::kHttpVersion09 = {
  "http version 0.9",  // name
  "HTTP/0.9 200 OK\r\n"
    "\r\n",  // raw
  HTTP_RESPONSE,  // type
  kMissingMethod,  // method
  200,  // status_code
  "OK",  // response_status
  kMissingRequestPath,  // request_path
  kMissingRequestUrl,  // request_url
  kMissingFragment,  // fragment
  kMissingQueryString,  // query_string
  "",  // body
  kMissingBodySize,  // body_size
  kMissingHost,  // host
  kMissingUserinfo,  // userinfo
  kMissingPort,  // port
  0,  // num_headers
  {},  // headers
  false,  // should_keep_alive
  kMissingUpgrade,  // upgrade
  0,  // http_major
  9,  // http_minor
  true,  // message_complete_on_eof
};

/* The client should wait for the server's EOF. That is, when neither
 * content-length nor transfer-encoding is specified, the end of body
 * is specified by the EOF.
 */
const HttpParserTestMessage
HttpParserTestMessage::kNoContentLengthNoTransferEncodingResponse = {
  "neither content-length nor transfer-encoding response",  // name
  "HTTP/1.1 200 OK\r\n"
    "Content-Type: text/plain\r\n"
    "\r\n"
    "hello world",  // raw
  HTTP_RESPONSE,  // type
  kMissingMethod,  // method
  200,  // status_code
  "OK",  // response_status
  kMissingRequestPath,  // request_path
  kMissingRequestUrl,  // request_url
  kMissingFragment,  // fragment
  kMissingQueryString,  // query_string
  "hello world",  // body
  kMissingBodySize,  // body_size
  kMissingHost,  // host
  kMissingUserinfo,  // userinfo
  kMissingPort,  // port
  1,  // num_headers
  { {"Content-Type", "text/plain"} },  // headers
  false,  // should_keep_alive
  kMissingUpgrade,  // upgrade
  1,  // http_major
  1,  // http_minor
  true,  // message_complete_on_eof
};

const HttpParserTestMessage
HttpParserTestMessage::kNoBodyHttp10Ka200 = {
  "HTTP/1.0 with keep-alive and EOF-terminated 200 status",  // name
  "HTTP/1.0 200 OK\r\n"
    "Connection: keep-alive\r\n"
    "\r\n",  // raw
  HTTP_RESPONSE,  // type
  kMissingMethod,  // method
  200,  // status_code
  "OK",  // response_status
  kMissingRequestPath,  // request_path
  kMissingRequestUrl,  // request_url
  kMissingFragment,  // fragment
  kMissingQueryString,  // query_string
  "",  // body
  0,  // body_size
  kMissingHost,  // host
  kMissingUserinfo,  // userinfo
  kMissingPort,  // port
  1,  // num_headers
  { {"Connection", "keep-alive"} },  // headers
  false,  // should_keep_alive
  kMissingUpgrade,  // upgrade
  1,  // http_major
  0,  // http_minor
  true,  // message_complete_on_eof
};

const HttpParserTestMessage
HttpParserTestMessage::kNoBodyHttp10Ka204 = {
  "HTTP/1.0 with keep-alive and a 204 status",  // name
  "HTTP/1.0 204 No content\r\n"
    "Connection: keep-alive\r\n"
    "\r\n",  // raw
  HTTP_RESPONSE,  // type
  kMissingMethod,  // method
  204,  // status_code
  "No content",  // response_status
  kMissingRequestPath,  // request_path
  kMissingRequestUrl,  // request_url
  kMissingFragment,  // fragment
  kMissingQueryString,  // query_string
  "",  // body
  0,  // body_size
  kMissingHost,  // host
  kMissingUserinfo,  // userinfo
  kMissingPort,  // port
  1,  // num_headers
  { {"Connection", "keep-alive"} },  // headers
  true,  // should_keep_alive
  kMissingUpgrade,  // upgrade
  1,  // http_major
  0,  // http_minor
  false,  // message_complete_on_eof
};

const HttpParserTestMessage
HttpParserTestMessage::kNoBodyHttp11Ka200 = {
  "HTTP/1.1 with an EOF-terminated 200 status",  // name
  "HTTP/1.1 200 OK\r\n"
    "\r\n",  // raw
  HTTP_RESPONSE,  // type
  kMissingMethod,  // method
  200,  // status_code
  "OK",  // response_status
  kMissingRequestPath,  // request_path
  kMissingRequestUrl,  // request_url
  kMissingFragment,  // fragment
  kMissingQueryString,  // query_string
  "",  // body
  0,  // body_size
  kMissingHost,  // host
  kMissingUserinfo,  // userinfo
  kMissingPort,  // port
  0,  // num_headers
  {},  // headers
  false,  // should_keep_alive
  kMissingUpgrade,  // upgrade
  1,  // http_major
  1,  // http_minor
  true,  // message_complete_on_eof
};

const HttpParserTestMessage
HttpParserTestMessage::kNoBodyHttp11Ka204 = {
  "HTTP/1.1 with a 204 status",  // name
  "HTTP/1.1 204 No content\r\n"
    "\r\n",  // raw
  HTTP_RESPONSE,  // type
  kMissingMethod,  // method
  204,  // status_code
  "No content",  // response_status
  kMissingRequestPath,  // request_path
  kMissingRequestUrl,  // request_url
  kMissingFragment,  // fragment
  kMissingQueryString,  // query_string
  "",  // body
  0,  // body_size
  kMissingHost,  // host
  kMissingUserinfo,  // userinfo
  kMissingPort,  // port
  0,  // num_headers
  {},  // headers
  true,  // should_keep_alive
  kMissingUpgrade,  // upgrade
  1,  // http_major
  1,  // http_minor
  false,  // message_complete_on_eof
};

const HttpParserTestMessage
HttpParserTestMessage::kNoBodyHttp11Noka204 = {
  "HTTP/1.1 with a 204 status and keep-alive disabled",  // name
  "HTTP/1.1 204 No content\r\n"
    "Connection: close\r\n"
    "\r\n",  // raw
  HTTP_RESPONSE,  // type
  kMissingMethod,  // method
  204,  // status_code
  "No content",  // response_status
  kMissingRequestPath,  // request_path
  kMissingRequestUrl,  // request_url
  kMissingFragment,  // fragment
  kMissingQueryString,  // query_string
  "",  // body
  0,  // body_size
  kMissingHost,  // host
  kMissingUserinfo,  // userinfo
  kMissingPort,  // port
  1,  // num_headers
  { {"Connection", "close"} },  // headers
  false,  // should_keep_alive
  kMissingUpgrade,  // upgrade
  1,  // http_major
  1,  // http_minor
  false,  // message_complete_on_eof
};

const HttpParserTestMessage
HttpParserTestMessage::kNoBodyHttp11KaChunked200 = {
  "HTTP/1.1 with chunked endocing and a 200 response",  // name
  "HTTP/1.1 200 OK\r\n"
    "Transfer-Encoding: chunked\r\n"
    "\r\n"
    "0\r\n"
    "\r\n",  // raw
  HTTP_RESPONSE,  // type
  kMissingMethod,  // method
  200,  // status_code
  "OK",  // response_status
  kMissingRequestPath,  // request_path
  kMissingRequestUrl,  // request_url
  kMissingFragment,  // fragment
  kMissingQueryString,  // query_string
  "",  // body
  0,  // body_size
  kMissingHost,  // host
  kMissingUserinfo,  // userinfo
  kMissingPort,  // port
  1,  // num_headers
  { {"Transfer-Encoding", "chunked"} },  // headers
  true,  // should_keep_alive
  kMissingUpgrade,  // upgrade
  1,  // http_major
  1,  // http_minor
  false,  // message_complete_on_eof
};

/* Should handle spaces in header fields */
const HttpParserTestMessage
HttpParserTestMessage::kSpaceInFieldRes = {
  "field space",  // name
  "HTTP/1.1 200 OK\r\n"
    "Server: Microsoft-IIS/6.0\r\n"
    "X-Powered-By: ASP.NET\r\n"
    "en-US Content-Type: text/xml\r\n" /* this is the problem */
    "Content-Type: text/xml\r\n"
    "Content-Length: 16\r\n"
    "Date: Fri, 23 Jul 2010 18:45:38 GMT\r\n"
    "Connection: keep-alive\r\n"
    "\r\n"
    "<xml>hello</xml>" /* fake body */,  // raw
  HTTP_RESPONSE,  // type
  kMissingMethod,  // method
  200,  // status_code
  "OK",  // response_status
  kMissingRequestPath,  // request_path
  kMissingRequestUrl,  // request_url
  kMissingFragment,  // fragment
  kMissingQueryString,  // query_string
  "<xml>hello</xml>",  // body
  kMissingBodySize,  // body_size
  kMissingHost,  // host
  kMissingUserinfo,  // userinfo
  kMissingPort,  // port
  7,  // num_headers
  { {"Server",  "Microsoft-IIS/6.0"},
    {"X-Powered-By", "ASP.NET"},
    {"en-US Content-Type", "text/xml"},
    {"Content-Type", "text/xml"},
    {"Content-Length", "16"},
    {"Date", "Fri, 23 Jul 2010 18:45:38 GMT"},
    {"Connection", "keep-alive"} },  // headers
  true,  // should_keep_alive
  kMissingUpgrade,  // upgrade
  1,  // http_major
  1,  // http_minor
  false,  // message_complete_on_eof
};

const HttpParserTestMessage
HttpParserTestMessage::kAmazonCom = {
  "amazon.com",  // name
  "HTTP/1.1 301 MovedPermanently\r\n"
    "Date: Wed, 15 May 2013 17:06:33 GMT\r\n"
    "Server: Server\r\n"
    "x-amz-id-1: 0GPHKXSJQ826RK7GZEB2\r\n"
    "p3p: policyref=\"http://www.amazon.com/w3c/p3p.xml\",CP=\"CAO DSP LAW CUR"
    " ADM IVAo IVDo CONo OTPo OUR DELi PUBi OTRi BUS PHY ONL UNI PUR FIN COM"
    " NAV INT DEM CNT STA HEA PRE LOC GOV OTC \"\r\n"
    "x-amz-id-2: STN69VZxIFSz9YJLbz1GDbxpbjG6Qjmmq5E3DxRhOUw+Et0p4hr7c"
    "/Q8qNcx4oAD\r\n"
    "Location: http://www.amazon.com/Dan-Brown/e/B000AP9DSU/ref=s9_pop_gw_al1?"
    "_encoding=UTF8&refinementId=618073011&pf_rd_m=ATVPDKIKX0DER&"
    "pf_rd_s=center-2&pf_rd_r=0SHYY5BZXN3KR20BNFAY&pf_rd_t=101&"
    "pf_rd_p=1263340922&pf_rd_i=507846\r\n"
    "Vary: Accept-Encoding,User-Agent\r\n"
    "Content-Type: text/html; charset=ISO-8859-1\r\n"
    "Transfer-Encoding: chunked\r\n"
    "\r\n"
    "1\r\n"
    "\n\r\n"
    "0\r\n"
    "\r\n",  // raw
  HTTP_RESPONSE,  // type
  kMissingMethod,  // method
  301,  // status_code
  "MovedPermanently",  // response_status
  kMissingRequestPath,  // request_path
  kMissingRequestUrl,  // request_url
  kMissingFragment,  // fragment
  kMissingQueryString,  // query_string
  "\n",  // body
  kMissingBodySize,  // body_size
  kMissingHost,  // host
  kMissingUserinfo,  // userinfo
  kMissingPort,  // port
  9,  // num_headers
  { {"Date", "Wed, 15 May 2013 17:06:33 GMT"},
    {"Server", "Server"},
    {"x-amz-id-1", "0GPHKXSJQ826RK7GZEB2"},
    {"p3p", "policyref=\"http://www.amazon.com/w3c/p3p.xml\",CP=\"CAO DSP LAW"
     " CUR ADM IVAo IVDo CONo OTPo OUR DELi PUBi OTRi BUS PHY ONL UNI PUR FIN"
     " COM NAV INT DEM CNT STA HEA PRE LOC GOV OTC \""},
    {"x-amz-id-2", "STN69VZxIFSz9YJLbz1GDbxpbjG6Qjmmq5E3DxRhOUw+Et0p4hr7c"
     "/Q8qNcx4oAD"},
    {"Location", "http://www.amazon.com/Dan-Brown/e/B000AP9DSU"
     "/ref=s9_pop_gw_al1?_encoding=UTF8&refinementId=618073011&"
     "pf_rd_m=ATVPDKIKX0DER&pf_rd_s=center-2&pf_rd_r=0SHYY5BZXN3KR20BNFAY&"
     "pf_rd_t=101&pf_rd_p=1263340922&pf_rd_i=507846"},
    {"Vary", "Accept-Encoding,User-Agent"},
    {"Content-Type", "text/html; charset=ISO-8859-1"},
    {"Transfer-Encoding", "chunked"} },  // headers
  true,  // should_keep_alive
  kMissingUpgrade,  // upgrade
  1,  // http_major
  1,  // http_minor
  false,  // message_complete_on_eof
};

const HttpParserTestMessage
HttpParserTestMessage::kEmptyReasonPhraseAfterSpace = {
  "empty reason phrase after space",  // name
  "HTTP/1.1 200 \r\n"
    "\r\n",  // raw
  HTTP_RESPONSE,  // type
  kMissingMethod,  // method
  200,  // status_code
  "",  // response_status
  kMissingRequestPath,  // request_path
  kMissingRequestUrl,  // request_url
  kMissingFragment,  // fragment
  kMissingQueryString,  // query_string
  "",  // body
  kMissingBodySize,  // body_size
  kMissingHost,  // host
  kMissingUserinfo,  // userinfo
  kMissingPort,  // port
  0,  // num_headers
  {},  // headers
  false,  // should_keep_alive
  kMissingUpgrade,  // upgrade
  1,  // http_major
  1,  // http_minor
  true,  // message_complete_on_eof
};

StringPiece HttpParserTestMessage::GetFirstHalfCurlGetRaw() {
  StringPiece curl_get_raw(kCurlGet.raw);
  return curl_get_raw.substr(0, curl_get_raw.length() / 2);
}

StringPiece HttpParserTestMessage::GetSecondHalfCurlGetRaw() {
  StringPiece curl_get_raw(kCurlGet.raw);
  return curl_get_raw.substr(curl_get_raw.length() / 2);
}

}  // namespace pjcore
