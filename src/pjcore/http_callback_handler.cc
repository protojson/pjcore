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

#include "pjcore/http_callback_handler.h"

#include "pjcore/logging.h"
#include "pjcore/live_http.pb.h"
#include "pjcore/live_util.h"

namespace pjcore {

HttpCallbackHandler::HttpCallbackHandler(
    LiveCapturableList* live_list,
    const Callback<void(scoped_ptr<HttpRequest> request,
                        const HttpResponseCallback& on_response)>& callback)
    : AbstractHttpHandler("pjcore::HttpCallbackHandler", live_list),
      callback_(callback) {
  PJCORE_CHECK(!callback_.is_null());
}

void HttpCallbackHandler::AsyncHandle(scoped_ptr<HttpRequest> request,
                                      const HttpResponseCallback& on_response) {
  PJCORE_CHECK(request);
  PJCORE_CHECK(!on_response.is_null());
  callback_.Run(request.Pass(), on_response);
}

HttpCallbackHandler::~HttpCallbackHandler() { LogDestroy(); }

scoped_ptr<google::protobuf::Message> HttpCallbackHandler::CaptureLive() const {
  scoped_ptr<LiveHttpCallbackHandler> live(new LiveHttpCallbackHandler());
  if (!callback_.is_null()) {
    CaptureLiveCallbackOut(callback_, live->mutable_callback());
  }
  return scoped_ptr<google::protobuf::Message>(live.release());
}

}  // namespace pjcore
