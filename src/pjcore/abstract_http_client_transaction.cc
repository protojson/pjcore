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

#include "pjcore/abstract_http_client_transaction.h"

#include "pjcore/http_client_transaction.h"

namespace pjcore {

namespace {

class DefaultHttpClientTransactionFactory
    : public AbstractHttpClientTransactionFactory {
 public:
  scoped_ptr<AbstractHttpClientTransaction> CreateTransaction(
      LiveCapturableList* live_list, AbstractUv* uv,
      AbstractCounter* pending_transaction_counter,
      scoped_ptr<HttpRequest> request,
      const HttpResponseCallback& on_response) OVERRIDE {
    return scoped_ptr<AbstractHttpClientTransaction>(
        new HttpClientTransaction(live_list, uv, pending_transaction_counter,
                                  request.Pass(), on_response));
  }
};

DefaultHttpClientTransactionFactory gDefaultHttpClientTransactionFactory;

}  // unnamed namespace

AbstractHttpClientTransaction::~AbstractHttpClientTransaction() {}

AbstractHttpClientTransaction::AbstractHttpClientTransaction(
    const char* live_class, LiveCapturableList* live_list)
    : LiveCapturable(live_class, live_list) {}

AbstractHttpClientTransactionFactory::~AbstractHttpClientTransactionFactory() {}

AbstractHttpClientTransactionFactory* GetDefaultHttpClientTransactionFactory() {
  return &gDefaultHttpClientTransactionFactory;
}

}  // namespace pjcore
