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

#ifndef PJCORE_ABSTRACT_HTTP_CLIENT_TRANSACTION_H_
#define PJCORE_ABSTRACT_HTTP_CLIENT_TRANSACTION_H_

#include "pjcore/abstract_counter.h"
#include "pjcore/abstract_http_handler.h"
#include "pjcore/abstract_uv.h"
#include "pjcore/auto_callback.h"
#include "pjcore/http.pb.h"
#include "pjcore/live_capturable.h"

namespace pjcore {

class AbstractHttpClientTransaction : public LiveCapturable {
 public:
  virtual ~AbstractHttpClientTransaction();

  virtual bool Init(Error* error) = 0;

  virtual NodeAndServiceName GetNodeAndServiceName() const {
    return NodeAndServiceName();
  }

  virtual bool PrepareWrite(Error* error) = 0;

  virtual void AsyncWrite(
      uv_stream_t* handle,
      Callback<void(bool result, const Error& error)> on_write_complete) = 0;

  virtual void ResultFailure(scoped_ptr<AbstractHttpClientTransaction> self,
                             const Error& error) = 0;

  virtual void ResultSuccess(scoped_ptr<AbstractHttpClientTransaction> self,
                             scoped_ptr<HttpResponse> response) = 0;

 protected:
  AbstractHttpClientTransaction(const char* live_class,
                                LiveCapturableList* live_list);
};

class AbstractHttpClientTransactionFactory {
 public:
  virtual ~AbstractHttpClientTransactionFactory();

  virtual scoped_ptr<AbstractHttpClientTransaction> CreateTransaction(
      LiveCapturableList* live_list, AbstractUv* uv,
      AbstractCounter* pending_transaction_counter,
      scoped_ptr<HttpRequest> request,
      const HttpResponseCallback& on_response) = 0;
};

AbstractHttpClientTransactionFactory* GetDefaultHttpClientTransactionFactory();

}  // namespace pjcore

#endif  // PJCORE_ABSTRACT_HTTP_CLIENT_TRANSACTION_H_
