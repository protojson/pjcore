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

#include "pjcore_test/mock_http_handler.h"

#include "pjcore/logging.h"
#include "pjcore/third_party/chromium/callback_helpers.h"

namespace pjcore {

MockHttpHandler::MockHttpHandler(LiveCapturableList* live_list)
    : AbstractHttpHandler("pjcore::MockHttpHandler", live_list) {}

MockHttpHandler::~MockHttpHandler() {
  while (!deferred_.empty()) {
    scoped_ptr<Context> context(deferred_.front());
    deferred_.pop();
  }

  // Die();
}

void MockHttpHandler::AsyncHandle(scoped_ptr<HttpRequest> request,
                                  const HttpResponseCallback& on_response) {
  scoped_ptr<Context> context(new Context(on_response));
  bool defer_response = false;
  if (!AsyncHandleProxy(*request, context->response_.get(), &context->error_,
                        &defer_response)) {
    context->response_.reset();
  }
  if (defer_response) {
    deferred_.push(context.release());
  } else {
    context->Invoke();
  }
}

void MockHttpHandler::InvokeAllDeferred() {
  while (!deferred_.empty()) {
    scoped_ptr<Context> context(deferred_.front());
    deferred_.pop();
    context->Invoke();
  }
}

scoped_ptr<google::protobuf::Message> MockHttpHandler::CaptureLive() const {
  return scoped_ptr<google::protobuf::Message>();
}

MockHttpHandler::Context::Context(const HttpResponseCallback& on_response)
    : on_response_(on_response), response_(new HttpResponse()) {}

MockHttpHandler::Context::~Context() { PJCORE_CHECK(on_response_.is_null()); }

void MockHttpHandler::Context::Invoke() {
  PJCORE_CHECK(!on_response_.is_null());
  ResetAndReturn(&on_response_).Run(response_.Pass(), error_);
}

}  // namespace pjcore
