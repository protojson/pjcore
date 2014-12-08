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

#include "pjcore/http_client_connection.h"

#include <gmock/gmock.h>

#include "pjcore/third_party/chromium/bind.h"

#include "pjcore/error_util.h"
#include "pjcore/logging.h"

#include "pjcore_test/http_parser_test_message.h"
#include "pjcore_test/is_gmock_verbose.h"
#include "pjcore_test/mock_counter.h"
#include "pjcore_test/mock_http_client_transaction.h"
#include "pjcore_test/mock_http_parser.h"
#include "pjcore_test/mock_uv_base.h"

namespace pjcore {

using ::testing::AssertionResult;
using ::testing::AssertionSuccess;
using ::testing::AssertionFailure;

using ::testing::ByRef;
using ::testing::DoAll;
using ::testing::Eq;
using ::testing::InSequence;
using ::testing::NotNull;
using ::testing::Return;
using ::testing::SaveArg;
using ::testing::_;

class HttpClientConnectionTest : public ::testing::Test,
                                 public MockUvLoopNewDelete,
                                 public AbstractHttpParserFactory {
 public:
  HttpClientConnectionTest()
      : live_list_(IsGmockVerbose()),
        shared_loop_(CreateUvLoop(&live_list_, this)),
        ai_addr_(),
        addr_info_(),
        connect_stream_(NULL),
        connect_req_(NULL),
        connect_cb_(NULL),
        alloc_cb_(NULL),
        read_cb_(NULL),
        close_cb_(NULL) {}

  void SetUp() OVERRIDE {
    addr_info_.ai_addr = &ai_addr_;

    ON_CALL(*this, tcp_init(uv_loop(), NotNull()))
        .WillByDefault(DoAll(SaveArg<1>(&connect_tcp()), Return(0)));

    ON_CALL(*this, tcp_connect(NotNull(), Eq(ByRef(connect_tcp())), &ai_addr_,
                               NotNull()))
        .WillByDefault(DoAll(SaveArg<0>(&connect_req_),
                             SaveArg<3>(&connect_cb_), Return(0)));

    ON_CALL(*this, close(Eq(ByRef(connect_handle())), NotNull()))
        .WillByDefault(SaveArg<1>(&close_cb_));

    ON_CALL(*this,
            read_start(Eq(ByRef(connect_stream())), NotNull(), NotNull()))
        .WillByDefault(
            DoAll(SaveArg<1>(&alloc_cb_), SaveArg<2>(&read_cb_), Return(0)));

    {
      InSequence s;
      EXPECT_CALL(pending_connection_counter_, Increment());
      EXPECT_CALL(pending_connection_counter_, Decrement());
    }

    {
      InSequence s;
      EXPECT_CALL(*this, freeaddrinfo(&addr_info_));
    }

    addr_info_list_ = new AddrInfoList(this, &addr_info_);
  }

  void TearDown() OVERRIDE {
    PJCORE_CHECK(init_transactions_.empty());

    if (close_cb_) {
      PJCORE_CHECK(connect_handle());

      uv_close_cb local_close_cb = close_cb_;
      close_cb_ = NULL;

      (*local_close_cb)(connect_handle());
    }
  }

  LiveCapturableList* live_list() { return &live_list_; }

  const SharedUvLoop& shared_loop() const { return shared_loop_; }

  uv_loop_t* uv_loop() const { return shared_loop()->loop(); }

  uv_handle_t*& connect_handle() {
    return *reinterpret_cast<uv_handle_t**>(&connect_stream_);
  }

  uv_stream_t*& connect_stream() { return connect_stream_; }

  uv_tcp_t*& connect_tcp() {
    return *reinterpret_cast<uv_tcp_t**>(&connect_stream_);
  }

  MOCK_METHOD1(freeaddrinfo, void(addrinfo* ai));
  MOCK_METHOD2(tcp_init, int(uv_loop_t* loop, uv_tcp_t* handle));
  MOCK_METHOD4(tcp_connect, int(uv_connect_t* req, uv_tcp_t* handle,
                                const struct sockaddr* addr, uv_connect_cb cb));
  MOCK_METHOD3(read_start, int(uv_stream_t* stream, uv_alloc_cb alloc_cb,
                               uv_read_cb read_cb));
  MOCK_METHOD1(read_stop, int(uv_stream_t* stream));
  MOCK_METHOD5(write,
               int(uv_write_t* req, uv_stream_t* handle, const uv_buf_t bufs[],
                   unsigned int nbufs, uv_write_cb cb));
  MOCK_METHOD2(close, void(uv_handle_t* handle, uv_close_cb close_cb));

  MOCK_METHOD2(OnClosing, void(bool result, const Error& error));
  MOCK_METHOD0(OnClose, void());

  scoped_ptr<AbstractHttpParser> CreateParser(LiveCapturableList* live_list,
                                              HttpParserType type) OVERRIDE {
    // CreateParserProxy(type);
    scoped_ptr<MockHttpParser> parser(new MockHttpParser(live_list));
    return scoped_ptr<AbstractHttpParser>(parser.Pass());
  }

  MOCK_METHOD1(CreateParserProxy, void(HttpParserType type));

  AssertionResult InvokeConnect(int connect_status) {
    if (!connect_req_) {
      return AssertionFailure() << "Connect request not set";
    }

    if (!connect_cb_) {
      return AssertionFailure() << "Connect callback not set";
    }

    (*connect_cb_)(connect_req_, connect_status);

    return AssertionSuccess();
  }

 protected:
  LiveCapturableList live_list_;
  SharedUvLoop shared_loop_;

  sockaddr ai_addr_;
  addrinfo addr_info_;

  SharedAddrInfoList addr_info_list_;

  MockCounter pending_connection_counter_;

  scoped_ptr<HttpClientConnection> connection_;

  uv_stream_t* connect_stream_;

  uv_connect_t* connect_req_;
  uv_connect_cb connect_cb_;

  uv_alloc_cb alloc_cb_;
  uv_read_cb read_cb_;

  uv_close_cb close_cb_;

  std::list<AbstractHttpClientTransaction*> init_transactions_;
};

TEST_F(HttpClientConnectionTest, NoAddrInfoList) {
  connection_.reset(new HttpClientConnection(
      live_list(), shared_loop(), &pending_connection_counter_, NULL));

  {
    GlobalLogOverride glboal_log_override;
    Error error;
    ASSERT_FALSE(connection_->InitSync(this, &error));
  }
}

TEST_F(HttpClientConnectionTest, TcpInitFailure) {
  InSequence s;
  EXPECT_CALL(*this, tcp_init(uv_loop(), NotNull()))
      .WillOnce(Return(UV_UNKNOWN));

  connection_.reset(new HttpClientConnection(live_list(), shared_loop(),
                                             &pending_connection_counter_,
                                             addr_info_list_));

  {
    GlobalLogOverride glboal_log_override;
    Error error;
    ASSERT_FALSE(connection_->InitSync(this, &error));
  }
}

TEST_F(HttpClientConnectionTest, ConnectImmediateFailure) {
  InSequence s;
  EXPECT_CALL(*this, tcp_init(uv_loop(), NotNull()));
  EXPECT_CALL(*this, tcp_connect(NotNull(), Eq(ByRef(connect_tcp())), &ai_addr_,
                                 NotNull())).WillOnce(Return(UV_UNKNOWN));
  EXPECT_CALL(*this, OnClosing(false, _));
  EXPECT_CALL(*this, close(Eq(ByRef(connect_handle())), NotNull()));
  EXPECT_CALL(*this, OnClose());

  connection_.reset(new HttpClientConnection(live_list(), shared_loop(),
                                             &pending_connection_counter_,
                                             addr_info_list_));

  Error error;
  ASSERT_TRUE(connection_->InitSync(this, &error));

  {
    GlobalLogOverride glboal_log_override;
    connection_->InitAsync(
        &init_transactions_,
        Bind(&HttpClientConnectionTest::OnClosing, Unretained(this)),
        Bind(&HttpClientConnectionTest::OnClose, Unretained(this)));
  }
}

TEST_F(HttpClientConnectionTest, ConnectDeferredFailure) {
  InSequence s;
  EXPECT_CALL(*this, tcp_init(uv_loop(), NotNull()));
  EXPECT_CALL(*this, tcp_connect(NotNull(), Eq(ByRef(connect_tcp())), &ai_addr_,
                                 NotNull()));
  EXPECT_CALL(*this, OnClosing(false, _));
  EXPECT_CALL(*this, close(Eq(ByRef(connect_handle())), NotNull()));
  EXPECT_CALL(*this, OnClose());

  connection_.reset(new HttpClientConnection(live_list(), shared_loop(),
                                             &pending_connection_counter_,
                                             addr_info_list_));

  Error error;
  ASSERT_TRUE(connection_->InitSync(this, &error));

  connection_->InitAsync(
      &init_transactions_,
      Bind(&HttpClientConnectionTest::OnClosing, Unretained(this)),
      Bind(&HttpClientConnectionTest::OnClose, Unretained(this)));

  {
    GlobalLogOverride global_log_override;
    ASSERT_TRUE(InvokeConnect(UV_UNKNOWN));
  }
}

TEST_F(HttpClientConnectionTest, ReadStartFailure) {
  InSequence s;
  EXPECT_CALL(*this, tcp_init(uv_loop(), NotNull()));
  EXPECT_CALL(*this, tcp_connect(NotNull(), Eq(ByRef(connect_tcp())), &ai_addr_,
                                 NotNull()));
  EXPECT_CALL(*this, read_start(Eq(ByRef(connect_stream())), NotNull(),
                                NotNull())).WillOnce(Return(UV_UNKNOWN));
  EXPECT_CALL(*this, OnClosing(false, _));
  EXPECT_CALL(*this, close(Eq(ByRef(connect_handle())), NotNull()));
  EXPECT_CALL(*this, OnClose());

  connection_.reset(new HttpClientConnection(live_list(), shared_loop(),
                                             &pending_connection_counter_,
                                             addr_info_list_));

  Error error;
  ASSERT_TRUE(connection_->InitSync(this, &error));

  connection_->InitAsync(
      &init_transactions_,
      Bind(&HttpClientConnectionTest::OnClosing, Unretained(this)),
      Bind(&HttpClientConnectionTest::OnClose, Unretained(this)));

  {
    GlobalLogOverride global_log_override;
    ASSERT_TRUE(InvokeConnect(0));
  }
}

TEST_F(HttpClientConnectionTest, InitAndClose) {
  InSequence s;
  EXPECT_CALL(*this, tcp_init(uv_loop(), NotNull()));
  EXPECT_CALL(*this, tcp_connect(NotNull(), Eq(ByRef(connect_tcp())), &ai_addr_,
                                 NotNull()));
  EXPECT_CALL(*this, read_start(Eq(ByRef(connect_stream())), NotNull(),
                                NotNull())).WillOnce(Return(0));
  EXPECT_CALL(*this, OnClosing(true, _));
  EXPECT_CALL(*this, read_stop(Eq(ByRef(connect_stream()))));
  EXPECT_CALL(*this, close(Eq(ByRef(connect_handle())), NotNull()));
  EXPECT_CALL(*this, OnClose());

  connection_.reset(new HttpClientConnection(live_list(), shared_loop(),
                                             &pending_connection_counter_,
                                             addr_info_list_));

  Error error;
  ASSERT_TRUE(connection_->InitSync(this, &error));

  connection_->InitAsync(
      &init_transactions_,
      Bind(&HttpClientConnectionTest::OnClosing, Unretained(this)),
      Bind(&HttpClientConnectionTest::OnClose, Unretained(this)));

  {
    GlobalLogOverride global_log_override;
    ASSERT_TRUE(InvokeConnect(0));
  }

  connection_->Close(true, Error());
}

TEST_F(HttpClientConnectionTest, InitTransactionPrepareWriteFailure) {
  scoped_ptr<MockHttpClientTransaction> transaction(
      new MockHttpClientTransaction(live_list()));

  InSequence s;
  EXPECT_CALL(*this, tcp_init(uv_loop(), NotNull()));
  EXPECT_CALL(*this, tcp_connect(NotNull(), Eq(ByRef(connect_tcp())), &ai_addr_,
                                 NotNull()));
  EXPECT_CALL(*this, read_start(Eq(ByRef(connect_stream())), NotNull(),
                                NotNull())).WillOnce(Return(0));
  EXPECT_CALL(*transaction, PrepareWrite(NotNull())).WillOnce(Return(false));
  EXPECT_CALL(*transaction, ResultFailureProxy(_));
  EXPECT_CALL(*transaction, Die());
  EXPECT_CALL(*this, OnClosing(true, _));
  EXPECT_CALL(*this, read_stop(Eq(ByRef(connect_stream()))));
  EXPECT_CALL(*this, close(Eq(ByRef(connect_handle())), NotNull()));
  EXPECT_CALL(*this, OnClose());

  init_transactions_.push_back(transaction.release());

  connection_.reset(new HttpClientConnection(live_list(), shared_loop(),
                                             &pending_connection_counter_,
                                             addr_info_list_));

  Error error;
  ASSERT_TRUE(connection_->InitSync(this, &error));

  connection_->InitAsync(
      &init_transactions_,
      Bind(&HttpClientConnectionTest::OnClosing, Unretained(this)),
      Bind(&HttpClientConnectionTest::OnClose, Unretained(this)));

  {
    GlobalLogOverride global_log_override;
    ASSERT_TRUE(InvokeConnect(0));
  }

  connection_->Close(true, Error());
}

TEST_F(HttpClientConnectionTest, InitTransactionWriteFailure) {
  scoped_ptr<MockHttpClientTransaction> transaction(
      new MockHttpClientTransaction(live_list()));

  Callback<void(bool result, const Error& error)> on_write_complete;

  InSequence s;
  EXPECT_CALL(*this, tcp_init(uv_loop(), NotNull()));
  EXPECT_CALL(*this, tcp_connect(NotNull(), Eq(ByRef(connect_tcp())), &ai_addr_,
                                 NotNull()));
  EXPECT_CALL(*this, read_start(Eq(ByRef(connect_stream())), NotNull(),
                                NotNull())).WillOnce(Return(0));
  EXPECT_CALL(*transaction, PrepareWrite(NotNull())).WillOnce(Return(true));
  EXPECT_CALL(*transaction, AsyncWrite(Eq(ByRef(connect_stream())), _))
      .WillOnce(SaveArg<1>(&on_write_complete));
  EXPECT_CALL(*transaction, ResultFailureProxy(_));
  EXPECT_CALL(*transaction, Die());
  EXPECT_CALL(*this, OnClosing(false, _));
  EXPECT_CALL(*this, read_stop(Eq(ByRef(connect_stream()))));
  EXPECT_CALL(*this, close(Eq(ByRef(connect_handle())), NotNull()));
  EXPECT_CALL(*this, OnClose());

  init_transactions_.push_back(transaction.release());

  connection_.reset(new HttpClientConnection(live_list(), shared_loop(),
                                             &pending_connection_counter_,
                                             addr_info_list_));

  Error error;
  ASSERT_TRUE(connection_->InitSync(this, &error));

  connection_->InitAsync(
      &init_transactions_,
      Bind(&HttpClientConnectionTest::OnClosing, Unretained(this)),
      Bind(&HttpClientConnectionTest::OnClose, Unretained(this)));

  ASSERT_TRUE(InvokeConnect(0));

  ASSERT_FALSE(on_write_complete.is_null());

  {
    GlobalLogOverride global_log_override;
    on_write_complete.Run(false, Error());
  }
}

}  // namespace pjcore
