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

#include "pjcore/http_server_transaction.h"

#include <gmock/gmock.h>

#include "pjcore/third_party/chromium/bind.h"

#include "pjcore/error_util.h"
#include "pjcore/logging.h"

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

class HttpServerTransactionTest : public ::testing::Test, public MockUvBase {
 public:
  MOCK_METHOD5(write,
               int(uv_write_t* req, uv_stream_t* handle, const uv_buf_t bufs[],
                   unsigned int nbufs, uv_write_cb cb));

  MOCK_METHOD2(OnWriteComplete, void(bool result, const Error& error));
};

TEST_F(HttpServerTransactionTest, WriteImmediateFailure) {
  MockCounter counter;

  {
    InSequence s;
    EXPECT_CALL(counter, Increment()).Times(Exactly(1));
    EXPECT_CALL(*this, write(NotNull(), NULL, _, _, _))
        .WillOnce(Return(UV_UNKNOWN));
    EXPECT_CALL(*this, OnWriteComplete(false, _));
    EXPECT_CALL(counter, Decrement()).Times(Exactly(1));
  }

  scoped_ptr<HttpRequest> request(new HttpRequest());

  HttpServerTransaction transaction(NULL, this, &counter, request.Pass());

  scoped_ptr<HttpResponse> response(new HttpResponse());

  {
    using std::swap;
    swap(transaction.response(), response);
  }

  Error error;
  ASSERT_TRUE(transaction.PrepareWrite(&error)) << ErrorToString(error);

  {
    GlobalLogOverride global_log_override;

    transaction.AsyncWrite(
        NULL,
        Bind(&HttpServerTransactionTest::OnWriteComplete, Unretained(this)));
  }
}

TEST_F(HttpServerTransactionTest, WriteDeferredFailure) {
  MockCounter counter;
  uv_write_t* req = NULL;
  uv_write_cb cb = NULL;

  {
    InSequence s;
    EXPECT_CALL(counter, Increment()).Times(Exactly(1));
    EXPECT_CALL(*this, write(NotNull(), NULL, _, _, _))
        .WillOnce(DoAll(SaveArg<0>(&req), SaveArg<4>(&cb), Return(0)));
    EXPECT_CALL(*this, OnWriteComplete(false, _));
    EXPECT_CALL(counter, Decrement()).Times(Exactly(1));
  }

  scoped_ptr<HttpRequest> request(new HttpRequest());

  HttpServerTransaction transaction(NULL, this, &counter, request.Pass());

  scoped_ptr<HttpResponse> response(new HttpResponse());

  {
    using std::swap;
    swap(transaction.response(), response);
  }

  Error error;
  ASSERT_TRUE(transaction.PrepareWrite(&error)) << ErrorToString(error);

  transaction.AsyncWrite(NULL, Bind(&HttpServerTransactionTest::OnWriteComplete,
                                    Unretained(this)));

  {
    GlobalLogOverride global_log_override;

    (*cb)(req, UV_UNKNOWN);
  }
}

TEST_F(HttpServerTransactionTest, WriteSuccess) {
  MockCounter counter;
  uv_write_t* req = NULL;
  uv_write_cb cb = NULL;

  {
    InSequence s;
    EXPECT_CALL(counter, Increment()).Times(Exactly(1));
    EXPECT_CALL(*this, write(NotNull(), NULL, _, _, _))
        .WillOnce(DoAll(SaveArg<0>(&req), SaveArg<4>(&cb), Return(0)));
    EXPECT_CALL(*this, OnWriteComplete(true, _));
    EXPECT_CALL(counter, Decrement()).Times(Exactly(1));
  }

  scoped_ptr<HttpRequest> request(new HttpRequest());

  HttpServerTransaction transaction(NULL, this, &counter, request.Pass());

  scoped_ptr<HttpResponse> response(new HttpResponse());

  {
    using std::swap;
    swap(transaction.response(), response);
  }

  Error error;
  ASSERT_TRUE(transaction.PrepareWrite(&error)) << ErrorToString(error);

  transaction.AsyncWrite(NULL, Bind(&HttpServerTransactionTest::OnWriteComplete,
                                    Unretained(this)));

  (*cb)(req, 0);
}

}  // namespace pjcore
