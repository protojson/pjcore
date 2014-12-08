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

#ifndef PJCORE_ABSTRACT_HTTP_PARSER_H_
#define PJCORE_ABSTRACT_HTTP_PARSER_H_

#include "pjcore/third_party/chromium/string_piece.h"
#include "pjcore/third_party/chromium/scoped_ptr.h"

#include "pjcore/http.pb.h"
#include "pjcore/live_capturable.h"

namespace pjcore {

class AbstractHttpParser : public LiveCapturable {
 public:
  virtual ~AbstractHttpParser();

  virtual bool Read(StringPiece data, size_t* offset, Error* error) = 0;

  virtual scoped_ptr<HttpRequest> NextRequest() = 0;

  virtual scoped_ptr<HttpResponse> NextResponse() = 0;

 protected:
  AbstractHttpParser(const char* live_class, LiveCapturableList* live_list);
};

class AbstractHttpParserFactory {
 public:
  virtual ~AbstractHttpParserFactory();

  virtual scoped_ptr<AbstractHttpParser> CreateParser(
      LiveCapturableList* live_list, HttpParserType type) = 0;
};

AbstractHttpParserFactory* GetDefaultHttpParserFactory();

}  // namespace pjcore

#endif  // PJCORE_ABSTRACT_HTTP_PARSER_H_
