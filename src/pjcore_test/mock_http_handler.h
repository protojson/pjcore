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

#ifndef PJCORE_TEST_MOCK_HTTP_HANDLER_H_
#define PJCORE_TEST_MOCK_HTTP_HANDLER_H_

#include <queue>

#include "gmock/gmock.h"

#include "pjcore/abstract_http_handler.h"

namespace pjcore {

class MockHttpHandler : public AbstractHttpHandler {
 public:
  explicit MockHttpHandler(LiveCapturableList* live_list);

  ~MockHttpHandler();

  MOCK_METHOD0(Die, void());

  void AsyncHandle(scoped_ptr<HttpRequest> request,
                   const HttpResponseCallback& on_response) OVERRIDE;

  MOCK_METHOD4(AsyncHandleProxy,
               bool(const HttpRequest& request, HttpResponse* response,
                    Error* error, bool* defer_response));

  void InvokeAllDeferred();

 protected:
  scoped_ptr<google::protobuf::Message> CaptureLive() const OVERRIDE;

 private:
  struct Context {
    explicit Context(const HttpResponseCallback& on_response);
    ~Context();

    void Invoke();

    HttpResponseCallback on_response_;
    scoped_ptr<HttpResponse> response_;
    Error error_;
  };

  std::queue<Context*> deferred_;
};

typedef scoped_refptr<MockHttpHandler> SharedMockHttpHandler;

}  // namespace pjcore

#endif  // PJCORE_TEST_MOCK_HTTP_HANDLER_H_
