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

#ifndef PJCORE_HTTP_PARSER_PLUS_PLUS_H_
#define PJCORE_HTTP_PARSER_PLUS_PLUS_H_

#include <http_parser.h>

#include <list>
#include <string>

#include "pjcore/abstract_http_parser.h"

namespace pjcore {

class HttpParserPlusPlus : public AbstractHttpParser {
 public:
  HttpParserPlusPlus(LiveCapturableList* live_list, HttpParserType type);

  ~HttpParserPlusPlus();

  HttpParserType type() const { return type_; }

  bool Read(StringPiece data, size_t* offset, Error* error) OVERRIDE;

  scoped_ptr<HttpRequest> NextRequest() OVERRIDE;

  scoped_ptr<HttpResponse> NextResponse() OVERRIDE;

 protected:
  scoped_ptr<google::protobuf::Message> CaptureLive() const OVERRIDE;

 private:
  static int StaticOnMessageBegin(http_parser* parser);

  bool OnMessageBegin();

  static int StaticOnUrl(http_parser* parser, const char* at, size_t length);

  bool OnUrl(StringPiece data);

  static int StaticOnStatus(http_parser* parser, const char* at, size_t length);

  bool OnStatus(StringPiece data);

  static int StaticOnHeaderField(http_parser* parser, const char* at,
                                 size_t length);

  bool OnHeaderField(StringPiece data);

  static int StaticOnHeaderValue(http_parser* parser, const char* at,
                                 size_t length);

  bool OnHeaderValue(StringPiece data);

  static int StaticOnHeadersComplete(http_parser* parser);

  bool OnHeadersComplete();

  static int StaticOnBody(http_parser* parser, const char* at, size_t length);

  bool OnBody(StringPiece data);

  static int StaticOnMessageComplete(http_parser* parser);

  bool OnMessageComplete();

  HttpParserType type_;

  scoped_ptr<HttpRequest> pending_request_;

  scoped_ptr<HttpResponse> pending_response_;

  http_parser_settings parser_settings_;

  http_parser parser_;

  Error* error;

  std::list<HttpRequest*> request_queue_;

  std::list<HttpResponse*> response_queue_;

  DISALLOW_EVIL_CONSTRUCTORS(HttpParserPlusPlus);
};

}  // namespace pjcore

#endif  // PJCORE_HTTP_PARSER_PLUS_PLUS_H_
