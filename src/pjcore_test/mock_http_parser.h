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

#ifndef PJCORE_TEST_MOCK_HTTP_PARSER_H_
#define PJCORE_TEST_MOCK_HTTP_PARSER_H_

#include "gmock/gmock.h"

#include "pjcore/abstract_http_parser.h"

namespace pjcore {

class MockHttpParser : public AbstractHttpParser {
 public:
  explicit MockHttpParser(LiveCapturableList* live_list);

  ~MockHttpParser();

  MOCK_METHOD0(Die, void());

  MOCK_METHOD3(Read, bool(StringPiece data, size_t* offset, Error* error));

  scoped_ptr<HttpRequest> NextRequest() OVERRIDE;

  MOCK_METHOD1(NextRequestProxy, bool(HttpRequest* request));

  scoped_ptr<HttpResponse> NextResponse() OVERRIDE;

  MOCK_METHOD1(NextResponseProxy, bool(HttpResponse* response));

 protected:
  scoped_ptr<google::protobuf::Message> CaptureLive() const OVERRIDE;
};

}  // namespace pjcore

#endif  // PJCORE_TEST_MOCK_HTTP_PARSER_H_
