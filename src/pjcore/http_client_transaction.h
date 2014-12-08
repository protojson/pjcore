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

#ifndef PJCORE_HTTP_CLIENT_TRANSACTION_H_
#define PJCORE_HTTP_CLIENT_TRANSACTION_H_

#include <string>
#include <vector>

#include "pjcore/abstract_http_client_transaction.h"
#include "pjcore/auto_callback.h"

namespace pjcore {

class HttpClientTransaction : public AbstractHttpClientTransaction {
 public:
  HttpClientTransaction(LiveCapturableList* live_list, AbstractUv* uv,
                        AbstractCounter* pending_transaction_counter,
                        scoped_ptr<HttpRequest> request,
                        const HttpResponseCallback& on_response);
  // TODO(pjcore): use AutoResultErrorCallback for exception safety

  ~HttpClientTransaction();

  bool Init(Error* error) OVERRIDE;

  AbstractUv* uv() const { return uv_; }

  NodeAndServiceName GetNodeAndServiceName() const OVERRIDE {
    return node_and_service_name_;
  }

  bool PrepareWrite(Error* error) OVERRIDE;

  void AsyncWrite(uv_stream_t* handle,
                  Callback<void(bool result, const Error& error)>
                      on_write_complete) OVERRIDE;

  void ResultFailure(scoped_ptr<AbstractHttpClientTransaction> self,
                     const Error& error) OVERRIDE;

  void ResultSuccess(scoped_ptr<AbstractHttpClientTransaction> self,
                     scoped_ptr<HttpResponse> response) OVERRIDE;

 protected:
  scoped_ptr<google::protobuf::Message> CaptureLive() const OVERRIDE;

 private:
  static void StaticOnWriteComplete(uv_write_t* req, int status);

  bool OnWriteComplete(int status, Error* error);

  AbstractUv* uv_;

  AbstractCounter::Unit pending_transaction_counter_unit_;

  scoped_ptr<HttpRequest> request_;

  NodeAndServiceName node_and_service_name_;

  AutoResultErrorCallback<scoped_ptr<HttpResponse> > on_response_;

  Callback<void(bool result, const Error& error)> on_write_complete_;

  std::string request_header_;

  std::vector<uv_buf_t> write_bufs_;

  uv_write_t write_req_;
};

}  // namespace pjcore

#endif  // PJCORE_HTTP_CLIENT_TRANSACTION_H_
