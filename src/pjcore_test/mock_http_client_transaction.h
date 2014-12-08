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

#ifndef PJCORE_TEST_MOCK_HTTP_CLIENT_TRANSACTION_H_
#define PJCORE_TEST_MOCK_HTTP_CLIENT_TRANSACTION_H_

#include <gmock/gmock.h>

#include "pjcore/abstract_http_client_transaction.h"

namespace pjcore {

class MockHttpClientTransaction : public AbstractHttpClientTransaction {
 public:
  explicit MockHttpClientTransaction(LiveCapturableList* live_list);

  ~MockHttpClientTransaction();

  MOCK_METHOD0(Die, void());

  MOCK_METHOD1(Init, bool(Error* error));

  MOCK_CONST_METHOD0(GetNodeAndServiceName, NodeAndServiceName());

  MOCK_METHOD2(
      AsyncWrite,
      void(uv_stream_t* handle,
           Callback<void(bool result, const Error& error)> on_write_complete));

  void ResultFailure(scoped_ptr<AbstractHttpClientTransaction> self,
                     const Error& error) OVERRIDE;

  MOCK_METHOD1(ResultFailureProxy, void(const Error& error));

  void ResultSuccess(scoped_ptr<AbstractHttpClientTransaction> self,
                     scoped_ptr<HttpResponse> response) OVERRIDE;

  MOCK_METHOD1(ResultSuccessProxy, void(HttpResponse* response));

  MOCK_METHOD1(PrepareWrite, bool(Error* error));

 protected:
  scoped_ptr<google::protobuf::Message> CaptureLive() const OVERRIDE;
};

}  // namespace pjcore

#endif  // PJCORE_TEST_MOCK_HTTP_CLIENT_TRANSACTION_H_
