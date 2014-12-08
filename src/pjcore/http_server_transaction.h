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

#ifndef PJCORE_HTTP_SERVER_TRANSACTION_H_
#define PJCORE_HTTP_SERVER_TRANSACTION_H_

#include <string>
#include <vector>

#include "pjcore/third_party/chromium/callback.h"
#include "pjcore/third_party/chromium/scoped_ptr.h"

#include "pjcore/abstract_counter.h"
#include "pjcore/abstract_uv.h"
#include "pjcore/http.pb.h"
#include "pjcore/live_capturable.h"

namespace pjcore {

class HttpServerTransaction : public LiveCapturable {
 public:
  HttpServerTransaction(LiveCapturableList* live_list, AbstractUv* uv,
                        AbstractCounter* pending_transaction_counter,
                        scoped_ptr<HttpRequest> request);

  ~HttpServerTransaction();

  AbstractUv* uv() const { return uv_; }

  scoped_ptr<HttpRequest>& request() { return request_; }

  scoped_ptr<HttpResponse>& response() { return response_; }

  bool PrepareWrite(Error* error);

  void AsyncWrite(
      uv_stream_t* handle,
      Callback<void(bool result, const Error& error)> on_write_complete);

 protected:
  scoped_ptr<google::protobuf::Message> CaptureLive() const OVERRIDE;

 private:
  static void StaticOnWriteComplete(uv_write_t* req, int status);

  bool OnWriteComplete(int status, Error* error);

  AbstractUv* uv_;

  AbstractCounter::Unit pending_transaction_counter_unit_;

  scoped_ptr<HttpRequest> request_;

  scoped_ptr<HttpResponse> response_;

  Callback<void(bool result, const Error& error)> on_write_complete_;

  std::string response_header_;

  std::vector<uv_buf_t> write_bufs_;

  uv_write_t write_req_;
};

}  // namespace pjcore

#endif  // PJCORE_HTTP_SERVER_TRANSACTION_H_
