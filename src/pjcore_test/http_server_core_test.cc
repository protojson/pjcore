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

#include "pjcore/http_server_core.h"

#include <gmock/gmock.h>

#include "pjcore/third_party/chromium/bind.h"

#include "pjcore/error_util.h"
#include "pjcore/logging.h"

#include "pjcore_test/mock_http_server_connection.h"
#include "pjcore_test/is_gmock_verbose.h"
#include "pjcore_test/mock_counter.h"
#include "pjcore_test/mock_http_handler.h"
#include "pjcore_test/mock_http_parser.h"
#include "pjcore_test/mock_uv_base.h"

namespace pjcore {

using ::testing::AssertionResult;
using ::testing::AssertionSuccess;
using ::testing::AssertionFailure;
using ::testing::ByRef;
using ::testing::Eq;
using ::testing::Exactly;
using ::testing::InSequence;
using ::testing::NotNull;
using ::testing::Return;
using ::testing::SaveArg;
using ::testing::_;

class HttpServerCoreTest : public ::testing::Test,
                           public MockUvLoopNewDelete,
                           public AbstractHttpServerConnectionFactory,
                           public AbstractHttpParserFactory {
 public:
  HttpServerCoreTest()
      : live_list_(IsGmockVerbose()),
        shared_loop_(CreateUvLoop(&live_list_, this)),
        handler_(new MockHttpHandler(&live_list_)),
        core_(new HttpServerCore(&live_list_, this)),
        listen_tcp_(NULL),
        close_cb_(NULL),
        in_invoke_connection_(false),
        pending_connection_(NULL) {}

  ~HttpServerCoreTest() {
    if (IsGmockVerbose()) {
      PJCORE_LOG_ALL_LIVE_JSON;
    }
  }

  void SetUp() OVERRIDE {
    config_.set_port(HttpServerCore::kDefaultPort);
    config_.set_backlog(HttpServerCore::kDefaultBacklog);
  }

  LiveCapturableList* live_list() { return &live_list_; }

  uv_loop_t* loop() const { return shared_loop_->loop(); }

  uv_handle_t*& listen_handle() {
    return *reinterpret_cast<uv_handle_t**>(&listen_tcp_);
  }

  uv_stream_t*& listen_stream() {
    return *reinterpret_cast<uv_stream_t**>(&listen_tcp_);
  }

  uv_tcp_t*& listen_tcp() { return listen_tcp_; }

  MOCK_METHOD3(ip4_addr,
               int(const char* ip, int port, struct sockaddr_in* addr));
  MOCK_METHOD2(tcp_init, int(uv_loop_t* loop, uv_tcp_t* handle));
  MOCK_METHOD3(tcp_bind, int(uv_tcp_t* handle, const struct sockaddr* addr,
                             unsigned int flags));
  MOCK_METHOD3(listen,
               int(uv_stream_t* stream, int backlog, uv_connection_cb cb));
  MOCK_METHOD2(accept, int(uv_stream_t* server, uv_stream_t* client));
  MOCK_METHOD2(close, void(uv_handle_t* handle, uv_close_cb close_cb));

  MOCK_METHOD2(OnClosing, void(bool result, const Error& error));
  MOCK_METHOD0(OnClose, void());

  MOCK_METHOD0(OnDestroy, void());

  scoped_ptr<AbstractHttpServerConnection> CreateConnection(
      LiveCapturableList* live_list, const SharedUvLoop& shared_loop,
      AbstractCounter* pending_connection_counter,
      AbstractCounter* pending_transaction_counter, uv_tcp_t* listen_handle,
      const SharedHttpHandler& handler) OVERRIDE {
    PJCORE_CHECK(in_invoke_connection_);
    PJCORE_CHECK(!pending_connection_);

    pending_connection_ = new MockHttpServerConnection(live_list);

    bool init_sync_result = CreateConnectionProxy(listen_tcp());

    {
      InSequence s;
      EXPECT_CALL(*pending_connection_, InitSync(this, NotNull()))
          .WillOnce(Return(init_sync_result));

      if (!init_sync_result) {
        EXPECT_CALL(*pending_connection_, InitAsyncProxy()).Times(Exactly(0));
      } else {
        EXPECT_CALL(*pending_connection_, InitAsyncProxy()).Times(Exactly(1));
        EXPECT_CALL(*pending_connection_, Close(_, _)).Times(Exactly(1));
      }

      EXPECT_CALL(*pending_connection_, Die()).Times(Exactly(1));
    }

    return scoped_ptr<AbstractHttpServerConnection>(pending_connection_);
  }

  MOCK_METHOD1(CreateConnectionProxy, bool(uv_tcp_t* listen_handle));

  scoped_ptr<AbstractHttpParser> CreateParser(LiveCapturableList* live_list,
                                              HttpParserType type) OVERRIDE {
    CreateParserProxy(type);
    scoped_ptr<MockHttpParser> parser(new MockHttpParser(live_list));
    return scoped_ptr<AbstractHttpParser>(parser.Pass());
  }

  MOCK_METHOD1(CreateParserProxy, void(HttpParserType type));

  AssertionResult InvokeClose() {
    if (!close_cb_) {
      return AssertionFailure() << "Close callback not set";
    }
    if (!listen_handle()) {
      return AssertionFailure() << "Listen handle not set";
    }
    (*close_cb_)(listen_handle());
    return AssertionSuccess();
  }

  void DestroyShell() {
    PJCORE_CHECK(core_);
    HttpServerCore* local_core = core_;
    core_ = NULL;
    local_core->OnShellDestroyed(
        this, Bind(&HttpServerCoreTest::OnDestroy, Unretained(this)));
  }

 protected:
  LiveCapturableList live_list_;
  SharedUvLoop shared_loop_;
  SharedMockHttpHandler handler_;
  HttpServerCore* core_;

  HttpServerConfig config_;

  uv_tcp_t* listen_tcp_;

  uv_close_cb close_cb_;

  bool in_invoke_connection_;

  MockHttpServerConnection* pending_connection_;
};

TEST_F(HttpServerCoreTest, Ip4AddrFailure) {
  {
    InSequence s;
    EXPECT_CALL(*this, ip4_addr(_, _, NotNull())).WillOnce(Return(UV_UNKNOWN));
    EXPECT_CALL(*this, OnDestroy()).Times(Exactly(1));
  }

  {
    GlobalLogOverride global_log_override;
    Error error;
    ASSERT_FALSE(
        core_->InitSync(config_, shared_loop_, handler_, this, this, &error));
  }

  DestroyShell();
}

TEST_F(HttpServerCoreTest, TcpInitFailure) {
  {
    InSequence s;
    EXPECT_CALL(*this, ip4_addr(_, _, NotNull())).WillOnce(Return(0));
    EXPECT_CALL(*this, tcp_init(loop(), NotNull()))
        .WillOnce(Return(UV_UNKNOWN));
    EXPECT_CALL(*this, OnDestroy()).Times(Exactly(1));
  }

  Error error;
  ASSERT_TRUE(
      core_->InitSync(config_, shared_loop_, handler_, this, this, &error))
      << ErrorToString(error);

  {
    GlobalLogOverride global_log_override;
    ASSERT_FALSE(core_->InitAsync(&error));
  }

  DestroyShell();
}

TEST_F(HttpServerCoreTest, TcpBindFailure) {
  {
    InSequence s;
    EXPECT_CALL(*this, ip4_addr(_, _, NotNull())).WillOnce(Return(0));
    EXPECT_CALL(*this, tcp_init(loop(), NotNull()))
        .WillOnce(DoAll(SaveArg<1>(&listen_tcp()), Return(0)));
    EXPECT_CALL(*this, tcp_bind(Eq(ByRef(listen_tcp())), NotNull(), _))
        .WillOnce(Return(UV_UNKNOWN));
    EXPECT_CALL(*this, close(Eq(ByRef(listen_handle())), NotNull()))
        .WillOnce(SaveArg<1>(&close_cb_));
    EXPECT_CALL(*this, OnDestroy()).Times(Exactly(1));
  }

  Error error;
  ASSERT_TRUE(
      core_->InitSync(config_, shared_loop_, handler_, this, this, &error))
      << ErrorToString(error);

  {
    GlobalLogOverride global_log_override;
    ASSERT_FALSE(core_->InitAsync(&error));
  }

  DestroyShell();
  ASSERT_TRUE(InvokeClose());
}

TEST_F(HttpServerCoreTest, ListenFailure) {
  {
    InSequence s;
    EXPECT_CALL(*this, ip4_addr(_, _, NotNull())).WillOnce(Return(0));
    EXPECT_CALL(*this, tcp_init(loop(), NotNull()))
        .WillOnce(DoAll(SaveArg<1>(&listen_tcp()), Return(0)));
    EXPECT_CALL(*this, tcp_bind(Eq(ByRef(listen_tcp())), NotNull(), _))
        .WillOnce(Return(0));
    EXPECT_CALL(*this, listen(Eq(ByRef(listen_stream())), config_.backlog(),
                              NotNull())).WillOnce(Return(UV_UNKNOWN));
    EXPECT_CALL(*this, close(Eq(ByRef(listen_handle())), NotNull()))
        .WillOnce(SaveArg<1>(&close_cb_));
    EXPECT_CALL(*this, OnDestroy()).Times(Exactly(1));
  }

  Error error;
  ASSERT_TRUE(
      core_->InitSync(config_, shared_loop_, handler_, this, this, &error))
      << ErrorToString(error);

  {
    GlobalLogOverride global_log_override;
    ASSERT_FALSE(core_->InitAsync(&error));
  }

  DestroyShell();
  ASSERT_TRUE(InvokeClose());
}

class HttpServerCoreRunTest : public HttpServerCoreTest {
 public:
  HttpServerCoreRunTest() : HttpServerCoreTest(), connection_cb_(NULL) {}

  void SetUp() OVERRIDE {
    HttpServerCoreTest::SetUp();

    InSequence s;
    EXPECT_CALL(*this, ip4_addr(_, _, NotNull())).WillOnce(Return(0));
    EXPECT_CALL(*this, tcp_init(loop(), NotNull()))
        .WillOnce(DoAll(SaveArg<1>(&listen_tcp()), Return(0)));
    EXPECT_CALL(*this, tcp_bind(Eq(ByRef(listen_tcp())), NotNull(), _))
        .WillOnce(Return(0));
    EXPECT_CALL(
        *this, listen(Eq(ByRef(listen_stream())), config_.backlog(), NotNull()))
        .WillOnce(DoAll(SaveArg<2>(&connection_cb_), Return(0)));
    EXPECT_CALL(*this, close(Eq(ByRef(listen_handle())), NotNull()))
        .WillOnce(SaveArg<1>(&close_cb_));
    EXPECT_CALL(*this, OnDestroy()).Times(Exactly(1));

    Error error;
    PJCORE_CHECK(
        core_->InitSync(config_, shared_loop_, handler_, this, this, &error));

    PJCORE_CHECK(core_->InitAsync(&error));
  }

  void TearDown() OVERRIDE {
    DestroyShell();
    PJCORE_CHECK(InvokeClose());
  }

  AssertionResult InvokeConnection(
      int connection_status,
      MockHttpServerConnection** created_connection = NULL) {
    if (connection_status == 0) {
      PJCORE_CHECK(created_connection);
      *created_connection = NULL;
    } else {
      PJCORE_CHECK(!created_connection);
    }

    if (in_invoke_connection_) {
      return AssertionFailure() << "InvokeConnection called recursively";
    }
    if (pending_connection_) {
      return AssertionFailure() << "Pre-existing pending_connection_";
    }
    if (!connection_cb_) {
      return AssertionFailure() << "Connection callback not set";
    }
    if (!listen_stream()) {
      return AssertionFailure() << "Listen stream not set";
    }
    in_invoke_connection_ = true;
    (*connection_cb_)(listen_stream(), connection_status);
    in_invoke_connection_ = false;
    if (connection_status != 0) {
      if (pending_connection_) {
        return AssertionFailure() << "Unexpected pending_connection_";
      }
    } else {
      if (!pending_connection_) {
        return AssertionFailure() << "No pending_connection_";
      }
      *created_connection = pending_connection_;
      pending_connection_ = NULL;
    }

    return AssertionSuccess();
  }

 protected:
  uv_connection_cb connection_cb_;
};

TEST_F(HttpServerCoreRunTest, InitSuccess) {}

TEST_F(HttpServerCoreRunTest, ConnectionFailure) {
  {
    GlobalLogOverride global_log_override;
    ASSERT_TRUE(InvokeConnection(UV_UNKNOWN));
  }
}

TEST_F(HttpServerCoreRunTest, ConnectionInitFailure) {
  EXPECT_CALL(*this, CreateConnectionProxy(listen_tcp()))
      .WillOnce(Return(false));
  MockHttpServerConnection* connection = NULL;

  {
    GlobalLogOverride global_log_override;
    ASSERT_TRUE(InvokeConnection(0, &connection));
  }
}

TEST_F(HttpServerCoreRunTest, ConnectionInitSuccess) {
  EXPECT_CALL(*this, CreateConnectionProxy(listen_tcp()))
      .WillOnce(Return(true));
  MockHttpServerConnection* connection = NULL;
  ASSERT_TRUE(InvokeConnection(0, &connection));
  connection->Close(true, Error());
  ASSERT_TRUE(connection->InvokeOnClosing(true, Error()));
  ASSERT_TRUE(connection->InvokeOnClose());
}

TEST_F(HttpServerCoreRunTest, TwoConnections) {
  EXPECT_CALL(*this, CreateConnectionProxy(listen_tcp()))
      .WillOnce(Return(true))
      .WillOnce(Return(true));
  MockHttpServerConnection* first_connection = NULL;
  ASSERT_TRUE(InvokeConnection(0, &first_connection));
  MockHttpServerConnection* second_connection = NULL;
  ASSERT_TRUE(InvokeConnection(0, &second_connection));
  first_connection->Close(true, Error());
  ASSERT_TRUE(first_connection->InvokeOnClosing(true, Error()));
  second_connection->Close(true, Error());
  ASSERT_TRUE(second_connection->InvokeOnClosing(true, Error()));
  ASSERT_TRUE(first_connection->InvokeOnClose());
  ASSERT_TRUE(second_connection->InvokeOnClose());
}

}  // namespace pjcore
