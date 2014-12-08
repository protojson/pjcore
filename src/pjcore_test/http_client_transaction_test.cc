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

#include "pjcore/http_client_transaction.h"

#include <gmock/gmock.h>

#include "pjcore/third_party/chromium/bind.h"

#include "pjcore/error_util.h"
#include "pjcore/logging.h"

#include "pjcore_test/is_gmock_verbose.h"
#include "pjcore_test/mock_uv_base.h"
#include "pjcore_test/mock_counter.h"

namespace pjcore {

using ::testing::DoAll;
using ::testing::Exactly;
using ::testing::InSequence;
using ::testing::NotNull;
using ::testing::Return;
using ::testing::SaveArg;
using ::testing::_;

class HttpClientTransactionTest : public ::testing::Test, public MockUvBase {
 public:
  HttpClientTransactionTest() : live_list_(IsGmockVerbose()) {}

  LiveCapturableList* live_list() { return &live_list_; }

  MOCK_METHOD5(write,
               int(uv_write_t* req, uv_stream_t* handle, const uv_buf_t bufs[],
                   unsigned int nbufs, uv_write_cb cb));

  MOCK_METHOD2(OnWriteComplete, void(bool result, const Error& error));

  void OnResponse(scoped_ptr<HttpResponse> result, const Error& error) {
    OnResponseProxy(result.get(), error);
  }

  MOCK_METHOD2(OnResponseProxy, void(HttpResponse* result, const Error& error));

 protected:
  LiveCapturableList live_list_;
};

TEST_F(HttpClientTransactionTest, NoUrlFailure) {
  MockCounter pending_transaction_counter;

  {
    InSequence s;
    EXPECT_CALL(pending_transaction_counter, Increment());
    EXPECT_CALL(*this, OnResponseProxy(NULL, _));
    EXPECT_CALL(pending_transaction_counter, Decrement());
  }

  scoped_ptr<HttpRequest> request(new HttpRequest());

  HttpClientTransaction transaction(
      live_list(), this, &pending_transaction_counter, request.Pass(),
      Bind(&HttpClientTransactionTest::OnResponse, Unretained(this)));

  {
    GlobalLogOverride global_log_override;
    Error error;
    ASSERT_FALSE(transaction.Init(&error));
  }
}

TEST_F(HttpClientTransactionTest, WriteImmediateFailure) {
  MockCounter pending_transaction_counter;

  {
    // InSequence s;
    EXPECT_CALL(pending_transaction_counter, Increment());
    EXPECT_CALL(*this, write(NotNull(), NULL, _, _, _))
        .WillOnce(Return(UV_UNKNOWN));
    EXPECT_CALL(*this, OnWriteComplete(false, _)).Times(Exactly(1));
    EXPECT_CALL(*this, OnResponseProxy(NULL, _));
    EXPECT_CALL(pending_transaction_counter, Decrement());
  }

  scoped_ptr<HttpRequest> request(new HttpRequest());
  request->set_url("http://localhost:8080/json");

  HttpClientTransaction transaction(
      live_list(), this, &pending_transaction_counter, request.Pass(),
      Bind(&HttpClientTransactionTest::OnResponse, Unretained(this)));

  Error error;
  ASSERT_TRUE(transaction.Init(&error)) << ErrorToString(error);
  ASSERT_TRUE(transaction.PrepareWrite(&error)) << ErrorToString(error);

  {
    GlobalLogOverride global_log_override;
    transaction.AsyncWrite(
        NULL,
        Bind(&HttpClientTransactionTest::OnWriteComplete, Unretained(this)));
  }
}

TEST_F(HttpClientTransactionTest, WriteDeferredFailure) {
  MockCounter pending_transaction_counter;
  uv_write_t* req = NULL;
  uv_write_cb cb = NULL;

  {
    // InSequence s;
    EXPECT_CALL(pending_transaction_counter, Increment());
    EXPECT_CALL(*this, write(NotNull(), NULL, _, _, _))
        .WillOnce(DoAll(SaveArg<0>(&req), SaveArg<4>(&cb), Return(0)));
    EXPECT_CALL(*this, OnWriteComplete(false, _)).Times(Exactly(1));
    EXPECT_CALL(*this, OnResponseProxy(NULL, _));
    EXPECT_CALL(pending_transaction_counter, Decrement());
  }

  scoped_ptr<HttpRequest> request(new HttpRequest());
  request->set_url("http://localhost:8080/json");

  HttpClientTransaction transaction(
      live_list(), this, &pending_transaction_counter, request.Pass(),
      Bind(&HttpClientTransactionTest::OnResponse, Unretained(this)));

  Error error;
  ASSERT_TRUE(transaction.Init(&error)) << ErrorToString(error);
  ASSERT_TRUE(transaction.PrepareWrite(&error)) << ErrorToString(error);

  transaction.AsyncWrite(NULL, Bind(&HttpClientTransactionTest::OnWriteComplete,
                                    Unretained(this)));

  {
    GlobalLogOverride global_log_override;

    (*cb)(req, UV_UNKNOWN);
  }
}

TEST_F(HttpClientTransactionTest, WriteSuccess) {
  MockCounter pending_transaction_counter;
  uv_write_t* req = NULL;
  uv_write_cb cb = NULL;

  {
    // InSequence s;
    EXPECT_CALL(pending_transaction_counter, Increment());
    EXPECT_CALL(*this, write(NotNull(), NULL, _, _, _))
        .WillOnce(DoAll(SaveArg<0>(&req), SaveArg<4>(&cb), Return(0)));
    EXPECT_CALL(*this, OnWriteComplete(false, _)).Times(Exactly(1));
    EXPECT_CALL(*this, OnResponseProxy(NULL, _));
    EXPECT_CALL(pending_transaction_counter, Decrement());
  }

  scoped_ptr<HttpRequest> request(new HttpRequest());
  request->set_url("http://localhost:8080/json");

  HttpClientTransaction transaction(
      live_list(), this, &pending_transaction_counter, request.Pass(),
      Bind(&HttpClientTransactionTest::OnResponse, Unretained(this)));

  Error error;
  ASSERT_TRUE(transaction.Init(&error)) << ErrorToString(error);
  ASSERT_TRUE(transaction.PrepareWrite(&error)) << ErrorToString(error);

  transaction.AsyncWrite(NULL, Bind(&HttpClientTransactionTest::OnWriteComplete,
                                    Unretained(this)));

  {
    GlobalLogOverride global_log_override;

    (*cb)(req, UV_UNKNOWN);
  }
}

}  // namespace pjcore
