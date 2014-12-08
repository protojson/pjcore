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

#ifndef PJCORE_HTTP_CLIENT_H_
#define PJCORE_HTTP_CLIENT_H_

#include "pjcore/abstract_http_handler.h"
#include "pjcore/shared_uv_loop.h"

namespace pjcore {

class HttpClientCore;
class AbstractHttpClientTransactionFactory;

class HttpClient : public AbstractHttpHandler {
 public:
  static SharedHttpHandler Create(const HttpClientConfig& config,
                                  const Closure& on_destroy, Error* error);

  static SharedHttpHandler Create(LiveCapturableList* live_list,
                                  const HttpClientConfig& config,
                                  const SharedUvLoop& shared_loop,
                                  const Closure& on_destroy, Error* error);

  static SharedHttpHandler Create(
      LiveCapturableList* live_list, const HttpClientConfig& config,
      const SharedUvLoop& shared_loop, const Closure& on_destroy,
      AbstractHttpClientTransactionFactory* transaction_factory, Error* error);

  void AsyncHandle(scoped_ptr<HttpRequest> request,
                   const HttpResponseCallback& on_response) OVERRIDE;

 protected:
  scoped_ptr<google::protobuf::Message> CaptureLive() const OVERRIDE;

 private:
  friend class RefCounted<HttpClient>;

  explicit HttpClient(LiveCapturableList* live_list);

  ~HttpClient();

  HttpClientCore* core_;

  Closure on_destroy_;
};

}  // namespace pjcore

#endif  // PJCORE_HTTP_CLIENT_H_
