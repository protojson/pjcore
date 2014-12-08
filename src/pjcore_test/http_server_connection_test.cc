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

#include "pjcore/http_server_connection.h"

#include <gmock/gmock.h>

#include "pjcore/third_party/chromium/bind.h"

#include "pjcore/error_util.h"
#include "pjcore/logging.h"

#include "pjcore_test/http_parser_test_message.h"
#include "pjcore_test/is_gmock_verbose.h"
#include "pjcore_test/mock_counter.h"
#include "pjcore_test/mock_http_handler.h"
#include "pjcore_test/mock_uv_base.h"

namespace pjcore {

using ::testing::AssertionResult;
using ::testing::AssertionSuccess;
using ::testing::AssertionFailure;
using ::testing::ByRef;
using ::testing::DoAll;
using ::testing::Exactly;
using ::testing::Eq;
using ::testing::InSequence;
using ::testing::NotNull;
using ::testing::Ref;
using ::testing::Return;
using ::testing::SaveArg;
using ::testing::SetArgPointee;
using ::testing::_;

class HttpServerConnectionTest : public ::testing::Test,
                                 public MockUvLoopNewDelete {
 public:
  HttpServerConnectionTest()
      : live_list_(IsGmockVerbose()),
        shared_loop_(CreateUvLoop(&live_list_, this)),
        listen_handle_(),
        http_handler_(new MockHttpHandler(&live_list_)),
        connection_(new HttpServerConnection(&live_list_, shared_loop_, NULL,
                                             &pending_transaction_counter_,
                                             &listen_handle_, http_handler_)),
        accept_stream_(NULL),
        close_cb_(NULL),
        alloc_cb_(NULL),
        read_cb_(NULL),
        write_req_(NULL),
        write_cb_(NULL),
        second_write_req_(NULL),
        second_write_cb_(NULL) {}

  LiveCapturableList* live_list() { return &live_list_; }

  const SharedUvLoop& shared_loop() const { return shared_loop_; }

  uv_loop_t* uv_loop() const { return shared_loop()->loop(); }

  uv_stream_t* listen_stream() {
    return reinterpret_cast<uv_stream_t*>(&listen_handle_);
  }

  MockHttpHandler* http_handler() { return http_handler_.get(); }

  HttpServerConnection* connection() { return connection_.get(); }

  uv_handle_t*& accept_handle() {
    return *reinterpret_cast<uv_handle_t**>(&accept_stream_);
  }

  uv_stream_t*& accept_stream() { return accept_stream_; }

  uv_tcp_t*& accept_tcp() {
    return *reinterpret_cast<uv_tcp_t**>(&accept_stream_);
  }

  MockCounter* pending_transaction_counter() {
    return &pending_transaction_counter_;
  }

  MOCK_METHOD2(tcp_init, int(uv_loop_t* loop, uv_tcp_t* handle));
  MOCK_METHOD2(accept, int(uv_stream_t* server, uv_stream_t* client));
  MOCK_METHOD3(read_start, int(uv_stream_t* stream, uv_alloc_cb alloc_cb,
                               uv_read_cb read_cb));
  MOCK_METHOD1(read_stop, int(uv_stream_t* stream));
  MOCK_METHOD5(write,
               int(uv_write_t* req, uv_stream_t* handle, const uv_buf_t bufs[],
                   unsigned int nbufs, uv_write_cb cb));
  MOCK_METHOD2(close, void(uv_handle_t* handle, uv_close_cb close_cb));

  MOCK_METHOD2(OnClosing, void(bool result, const Error& error));
  MOCK_METHOD0(OnClose, void());

  void ExpectInitAndClose(bool closing_result) {
    InSequence s;
    EXPECT_CALL(*this, tcp_init(uv_loop(), NotNull()))
        .WillOnce(DoAll(SaveArg<1>(&accept_tcp()), Return(0)));
    EXPECT_CALL(*this, accept(listen_stream(), NotNull())).WillOnce(Return(0));
    EXPECT_CALL(*this,
                read_start(Eq(ByRef(accept_stream())), NotNull(), NotNull()))
        .WillOnce(
            DoAll(SaveArg<1>(&alloc_cb_), SaveArg<2>(&read_cb_), Return(0)));
    EXPECT_CALL(*this, OnClosing(closing_result, _)).Times(Exactly(1));
    EXPECT_CALL(*this, read_stop(Eq(ByRef(accept_stream()))))
        .WillOnce(Return(0));
    EXPECT_CALL(*this, close(Eq(ByRef(accept_handle())), NotNull()))
        .WillOnce(SaveArg<1>(&close_cb_));
    EXPECT_CALL(*this, OnClose()).Times(Exactly(1));
  }

  void ExpectTransactions(int transaction_count) {
    EXPECT_CALL(pending_transaction_counter_, Increment())
        .Times(Exactly(transaction_count));
    EXPECT_CALL(pending_transaction_counter_, Decrement())
        .Times(Exactly(transaction_count));
  }

  void ExpectWrite(int write_status) {
    EXPECT_CALL(*this,
                write(NotNull(), Eq(ByRef(accept_stream())), _, _, NotNull()))
        .WillOnce(DoAll(SaveArg<0>(&write_req_), SaveArg<4>(&write_cb_),
                        Return(write_status)));
  }

  void ExpectTwoWrites(int second_write_status) {
    EXPECT_CALL(*this,
                write(NotNull(), Eq(ByRef(accept_stream())), _, _, NotNull()))
        .WillOnce(
             DoAll(SaveArg<0>(&write_req_), SaveArg<4>(&write_cb_), Return(0)))
        .WillOnce(DoAll(SaveArg<0>(&second_write_req_),
                        SaveArg<4>(&second_write_cb_),
                        Return(second_write_status)));
  }

  AssertionResult InitAsync() {
    connection()->InitAsync(
        Bind(&HttpServerConnectionTest::OnClosing, Unretained(this)),
        Bind(&HttpServerConnectionTest::OnClose, Unretained(this)));
    return AssertionSuccess();
  }

  AssertionResult InvokeReadEof() {
    if (!accept_handle()) {
      return AssertionFailure() << "Accept handle not set";
    }

    if (!alloc_cb_) {
      return AssertionFailure() << "Alloc callback not set";
    }

    if (!read_cb_) {
      return AssertionFailure() << "Read callback not set";
    }

    (*read_cb_)(accept_stream(), UV_EOF, NULL);

    return AssertionSuccess();
  }

  AssertionResult InvokeRead(StringPiece content) {
    if (!accept_handle()) {
      return AssertionFailure() << "Accept handle not set";
    }

    if (!alloc_cb_) {
      return AssertionFailure() << "Alloc callback not set";
    }

    if (!read_cb_) {
      return AssertionFailure() << "Read callback not set";
    }

    uv_buf_t buf;
    (*alloc_cb_)(accept_handle(), content.size(), &buf);

    if (buf.len != content.size()) {
      return AssertionFailure() << "Alloc expected buf size " << content.size()
                                << ", actual buf size " << buf.len;
    }

    memcpy(buf.base, content.data(), content.size());

    (*read_cb_)(accept_stream(), content.size(), &buf);

    return AssertionSuccess();
  }

  AssertionResult InvokeWrite(int write_status) {
    if (!write_req_) {
      return AssertionFailure() << "Write request not set";
    }

    if (!write_cb_) {
      return AssertionFailure() << "Write callback not set";
    }

    (*write_cb_)(write_req_, write_status);

    return AssertionSuccess();
  }

  AssertionResult InvokeSecondWrite(int second_write_status) {
    if (!second_write_req_) {
      return AssertionFailure() << "Second write request not set";
    }

    if (!second_write_cb_) {
      return AssertionFailure() << "Second write callback not set";
    }

    (*second_write_cb_)(second_write_req_, second_write_status);

    return AssertionSuccess();
  }

  AssertionResult InvokeClose() {
    if (!close_cb_) {
      return AssertionFailure() << "Close callback not set";
    }
    if (!accept_handle()) {
      return AssertionFailure() << "Accept handle not set";
    }
    (*close_cb_)(accept_handle());
    return AssertionSuccess();
  }

 protected:
  LiveCapturableList live_list_;
  SharedUvLoop shared_loop_;
  MockCounter pending_transaction_counter_;
  uv_tcp_t listen_handle_;
  SharedMockHttpHandler http_handler_;
  scoped_ptr<HttpServerConnection> connection_;

  uv_stream_t* accept_stream_;

  uv_close_cb close_cb_;
  uv_alloc_cb alloc_cb_;
  uv_read_cb read_cb_;

  uv_write_t* write_req_;
  uv_write_cb write_cb_;

  uv_write_t* second_write_req_;
  uv_write_cb second_write_cb_;
};

TEST(HttpServerConnection, PendingConnectionCounter) {
  MockCounter mock_counter;

  InSequence s;
  EXPECT_CALL(mock_counter, Increment()).Times(Exactly(1));
  EXPECT_CALL(mock_counter, Decrement()).Times(Exactly(1));

  HttpServerConnection connection(
      NULL, CreateUvLoop(NULL, GetMockUvLoopNewDelete()), &mock_counter, NULL,
      NULL, SharedHttpHandler(new MockHttpHandler(NULL)));
}

TEST_F(HttpServerConnectionTest, TcpInitFailure) {
  EXPECT_CALL(*this, tcp_init(uv_loop(), NotNull()))
      .WillOnce(Return(UV_UNKNOWN));

  GlobalLogOverride global_log_override;
  Error error;
  ASSERT_FALSE(connection()->InitSync(GetDefaultHttpParserFactory(), &error));
}

TEST_F(HttpServerConnectionTest, AcceptFailure) {
  InSequence s;
  EXPECT_CALL(*this, tcp_init(uv_loop(), NotNull()))
      .WillOnce(DoAll(SaveArg<1>(&accept_tcp()), Return(0)));
  EXPECT_CALL(*this, accept(listen_stream(), NotNull()))
      .WillOnce(Return(UV_UNKNOWN));
  EXPECT_CALL(*this, OnClosing(false, _)).Times(Exactly(1));
  EXPECT_CALL(*this, close(Eq(ByRef(accept_handle())), NotNull()))
      .WillOnce(SaveArg<1>(&close_cb_));
  EXPECT_CALL(*this, OnClose()).Times(Exactly(1));

  Error error;
  ASSERT_TRUE(connection()->InitSync(GetDefaultHttpParserFactory(), &error))
      << ErrorToString(error);

  {
    GlobalLogOverride global_log_override;
    ASSERT_TRUE(InitAsync());
  }

  ASSERT_TRUE(InvokeClose());
}

TEST_F(HttpServerConnectionTest, ReadStartFailure) {
  {
    EXPECT_CALL(*this, tcp_init(uv_loop(), NotNull()))
        .WillOnce(DoAll(SaveArg<1>(&accept_tcp()), Return(0)));
    EXPECT_CALL(*this, accept(listen_stream(), NotNull())).WillOnce(Return(0));
    EXPECT_CALL(*this,
                read_start(Eq(ByRef(accept_stream())), NotNull(), NotNull()))
        .WillOnce(DoAll(SaveArg<1>(&alloc_cb_), SaveArg<2>(&read_cb_),
                        Return(UV_UNKNOWN)));
    EXPECT_CALL(*this, OnClosing(false, _)).Times(Exactly(1));
    EXPECT_CALL(*this, close(Eq(ByRef(accept_handle())), NotNull()))
        .WillOnce(SaveArg<1>(&close_cb_));
    EXPECT_CALL(*this, OnClose()).Times(Exactly(1));
  }

  Error error;
  ASSERT_TRUE(connection()->InitSync(GetDefaultHttpParserFactory(), &error))
      << ErrorToString(error);

  {
    GlobalLogOverride global_log_override;
    ASSERT_TRUE(InitAsync());
  }

  ASSERT_TRUE(InvokeClose());
}

TEST_F(HttpServerConnectionTest, ReadImmediateEof) {
  ExpectInitAndClose(true);

  Error error;
  ASSERT_TRUE(connection()->InitSync(GetDefaultHttpParserFactory(), &error))
      << ErrorToString(error);
  ASSERT_TRUE(InitAsync());

  ASSERT_TRUE(InvokeReadEof());
  ASSERT_TRUE(InvokeClose());
}

TEST_F(HttpServerConnectionTest, CurlGetImmediateSuccess) {
  ExpectInitAndClose(true);
  ExpectTransactions(1);
  EXPECT_CALL(*http_handler(), AsyncHandleProxy(_, _, _, _))
      .WillOnce(Return(true));
  ExpectWrite(0);

  Error error;
  ASSERT_TRUE(connection()->InitSync(GetDefaultHttpParserFactory(), &error))
      << ErrorToString(error);
  ASSERT_TRUE(InitAsync());

  ASSERT_TRUE(InvokeRead(HttpParserTestMessage::kCurlGet.raw));
  ASSERT_TRUE(InvokeWrite(0));
  ASSERT_TRUE(InvokeReadEof());
  ASSERT_TRUE(InvokeClose());
}

TEST_F(HttpServerConnectionTest, CurlGetImmediateFailure) {
  ExpectInitAndClose(true);
  ExpectTransactions(1);
  EXPECT_CALL(*http_handler(), AsyncHandleProxy(_, _, _, _))
      .WillOnce(Return(false));
  ExpectWrite(0);

  Error error;
  ASSERT_TRUE(connection()->InitSync(GetDefaultHttpParserFactory(), &error))
      << ErrorToString(error);
  ASSERT_TRUE(InitAsync());

  ASSERT_TRUE(InvokeRead(HttpParserTestMessage::kCurlGet.raw));
  ASSERT_TRUE(InvokeWrite(0));
  ASSERT_TRUE(InvokeReadEof());
  ASSERT_TRUE(InvokeClose());
}

TEST_F(HttpServerConnectionTest, CurlGetDeferredSuccess) {
  ExpectInitAndClose(true);
  ExpectTransactions(1);
  EXPECT_CALL(*http_handler(), AsyncHandleProxy(_, _, _, _))
      .WillOnce(DoAll(SetArgPointee<3>(true), Return(true)));
  ExpectWrite(0);

  Error error;
  ASSERT_TRUE(connection()->InitSync(GetDefaultHttpParserFactory(), &error))
      << ErrorToString(error);
  ASSERT_TRUE(InitAsync());

  ASSERT_TRUE(InvokeRead(HttpParserTestMessage::kCurlGet.raw));
  http_handler()->InvokeAllDeferred();
  ASSERT_TRUE(InvokeWrite(0));
  ASSERT_TRUE(InvokeReadEof());
  ASSERT_TRUE(InvokeClose());
}

TEST_F(HttpServerConnectionTest, CurlGetDeferredFailure) {
  ExpectInitAndClose(true);
  ExpectTransactions(1);
  EXPECT_CALL(*http_handler(), AsyncHandleProxy(_, _, _, _))
      .WillOnce(DoAll(SetArgPointee<3>(true), Return(false)));
  ExpectWrite(0);

  Error error;
  ASSERT_TRUE(connection()->InitSync(GetDefaultHttpParserFactory(), &error))
      << ErrorToString(error);
  ASSERT_TRUE(InitAsync());

  ASSERT_TRUE(InvokeRead(HttpParserTestMessage::kCurlGet.raw));
  http_handler()->InvokeAllDeferred();
  ASSERT_TRUE(InvokeWrite(0));
  ASSERT_TRUE(InvokeReadEof());
  ASSERT_TRUE(InvokeClose());
}

TEST_F(HttpServerConnectionTest, CurlGetIncomplete) {
  ExpectInitAndClose(true);

  Error error;
  ASSERT_TRUE(connection()->InitSync(GetDefaultHttpParserFactory(), &error))
      << ErrorToString(error);
  ASSERT_TRUE(InitAsync());

  ASSERT_TRUE(InvokeRead(HttpParserTestMessage::GetFirstHalfCurlGetRaw()));
  ASSERT_TRUE(InvokeReadEof());
  ASSERT_TRUE(InvokeClose());
}

TEST_F(HttpServerConnectionTest, WriteImmediateFailure) {
  ExpectInitAndClose(false);
  ExpectTransactions(1);
  EXPECT_CALL(*http_handler(), AsyncHandleProxy(_, _, _, _))
      .WillOnce(Return(true));
  ExpectWrite(UV_UNKNOWN);

  Error error;
  ASSERT_TRUE(connection()->InitSync(GetDefaultHttpParserFactory(), &error))
      << ErrorToString(error);
  ASSERT_TRUE(InitAsync());

  {
    GlobalLogOverride global_log_override;
    ASSERT_TRUE(InvokeRead(HttpParserTestMessage::kCurlGet.raw));
  }

  ASSERT_TRUE(InvokeClose());
}

TEST_F(HttpServerConnectionTest, WriteDeferredFailure) {
  ExpectInitAndClose(false);
  ExpectTransactions(1);
  EXPECT_CALL(*http_handler(), AsyncHandleProxy(_, _, _, _))
      .WillOnce(Return(true));
  ExpectWrite(0);

  Error error;
  ASSERT_TRUE(connection()->InitSync(GetDefaultHttpParserFactory(), &error))
      << ErrorToString(error);
  ASSERT_TRUE(InitAsync());

  ASSERT_TRUE(InvokeRead(HttpParserTestMessage::kCurlGet.raw));

  {
    GlobalLogOverride global_log_override;
    ASSERT_TRUE(InvokeWrite(UV_UNKNOWN));
  }
  ASSERT_TRUE(InvokeClose());
}

TEST_F(HttpServerConnectionTest, SecondCurlGetSuccess) {
  ExpectInitAndClose(true);
  ExpectTransactions(2);
  EXPECT_CALL(*http_handler(), AsyncHandleProxy(_, _, _, _))
      .WillOnce(Return(true))
      .WillOnce(Return(true));
  ExpectTwoWrites(0);

  Error error;
  ASSERT_TRUE(connection()->InitSync(GetDefaultHttpParserFactory(), &error))
      << ErrorToString(error);
  ASSERT_TRUE(InitAsync());

  ASSERT_TRUE(InvokeRead(HttpParserTestMessage::kCurlGet.raw));
  ASSERT_TRUE(InvokeRead(HttpParserTestMessage::kCurlGet.raw));
  ASSERT_TRUE(InvokeWrite(0));
  ASSERT_TRUE(InvokeSecondWrite(0));
  ASSERT_TRUE(InvokeReadEof());
  ASSERT_TRUE(InvokeClose());
}

TEST_F(HttpServerConnectionTest, SecondWriteImmediateFailure) {
  ExpectInitAndClose(false);
  ExpectTransactions(2);
  EXPECT_CALL(*http_handler(), AsyncHandleProxy(_, _, _, _))
      .WillOnce(Return(true))
      .WillOnce(Return(true));
  ExpectTwoWrites(UV_UNKNOWN);

  Error error;
  ASSERT_TRUE(connection()->InitSync(GetDefaultHttpParserFactory(), &error))
      << ErrorToString(error);
  ASSERT_TRUE(InitAsync());

  ASSERT_TRUE(InvokeRead(HttpParserTestMessage::kCurlGet.raw));

  {
    GlobalLogOverride global_log_override;
    ASSERT_TRUE(InvokeRead(HttpParserTestMessage::kCurlGet.raw));
  }

  ASSERT_TRUE(InvokeWrite(0));
  ASSERT_TRUE(InvokeClose());
}

TEST_F(HttpServerConnectionTest, SecondWriteDeferredFailure) {
  ExpectInitAndClose(false);
  ExpectTransactions(2);
  EXPECT_CALL(*http_handler(), AsyncHandleProxy(_, _, _, _))
      .WillOnce(Return(true))
      .WillOnce(Return(true));
  ExpectTwoWrites(0);

  Error error;
  ASSERT_TRUE(connection()->InitSync(GetDefaultHttpParserFactory(), &error))
      << ErrorToString(error);
  ASSERT_TRUE(InitAsync());

  ASSERT_TRUE(InvokeRead(HttpParserTestMessage::kCurlGet.raw));
  ASSERT_TRUE(InvokeRead(HttpParserTestMessage::kCurlGet.raw));
  ASSERT_TRUE(InvokeWrite(0));

  {
    GlobalLogOverride global_log_override;
    ASSERT_TRUE(InvokeSecondWrite(UV_UNKNOWN));
  }
  ASSERT_TRUE(InvokeClose());
}

}  // namespace pjcore
