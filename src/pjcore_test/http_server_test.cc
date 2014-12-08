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

#include "pjcore/http_server.h"

#include <gmock/gmock.h>

#include "pjcore/third_party/chromium/bind.h"

#include "pjcore/error_util.h"
#include "pjcore/logging.h"

#include "pjcore_test/is_gmock_verbose.h"
#include "pjcore_test/mock_http_server_core.h"

namespace pjcore {

using ::testing::Assign;
using ::testing::Exactly;
using ::testing::InSequence;
using ::testing::NotNull;
using ::testing::Return;
using ::testing::SetArgPointee;
using ::testing::_;

class HttpServerTest : public ::testing::Test,
                       public AbstractHttpServerCoreFactory {
 public:
  HttpServerTest() : live_list_(IsGmockVerbose()), core_(NULL) {}

  ~HttpServerTest() {
    if (IsGmockVerbose()) {
      PJCORE_LOG_ALL_LIVE_JSON;
    }
  }

  LiveCapturableList* live_list() { return &live_list_; }

  scoped_ptr<AbstractHttpServerCore> CreateCore(LiveCapturableList* live_list,
                                                void* shell) OVERRIDE {
    PJCORE_CHECK(!core_);

    core_ = new MockHttpServerCore(live_list);

    bool init_async_result = false;
    bool init_sync_result =
        CreateCoreProxy(live_list, shell, &init_async_result);

    {
      InSequence s;
      EXPECT_CALL(*core_, InitSync(_, _, _, _, _, NotNull()))
          .WillOnce(Return(init_sync_result));
      if (init_sync_result) {
        EXPECT_CALL(*core_, InitAsync(NotNull()))
            .WillOnce(Return(init_async_result));
        EXPECT_CALL(*core_, OnShellDestroyed(shell, _)).Times(Exactly(1));
      } else {
        EXPECT_CALL(*core_, InitAsync(_)).Times(Exactly(0));
        EXPECT_CALL(*core_, OnShellDestroyed(shell, _)).Times(Exactly(1));
      }
      EXPECT_CALL(*core_, Die())
          .WillOnce(Assign(&core_, static_cast<MockHttpServerCore*>(NULL)));
    }

    return scoped_ptr<AbstractHttpServerCore>(core_);
  }

  MOCK_METHOD3(CreateCoreProxy, bool(LiveCapturableList* live_list, void* shell,
                                     bool* init_async_result));

  MOCK_METHOD0(OnDestroy, void());

 protected:
  LiveCapturableList live_list_;

  HttpServerConfig config_;

  MockHttpServerCore* core_;
};

TEST_F(HttpServerTest, InitSyncFailure) {
  EXPECT_CALL(*this, CreateCoreProxy(_, NotNull(), _)).WillOnce(Return(false));

  {
    GlobalLogOverride global_log_override;
    Error error;
    SharedHttpServer server =
        HttpServer::Create(live_list(), config_, SharedUvLoop(),
                           SharedHttpHandler(), this, NULL, NULL, &error);
    ASSERT_FALSE(server);
  }

  ASSERT_TRUE(core_);
  delete core_;
  core_ = NULL;
}

TEST_F(HttpServerTest, InitAsyncFailure) {
  EXPECT_CALL(*this, CreateCoreProxy(_, NotNull(), _))
      .WillOnce(DoAll(SetArgPointee<2>(false), Return(true)));

  // TODO(pjcore): verify passing on_destroy callback
  // EXPECT_CALL(*this, OnDestroy()).Times(Exactly(1));

  Error error;
  SharedHttpServer server =
      HttpServer::Create(live_list(), config_, SharedUvLoop(),
                         SharedHttpHandler(), this, NULL, NULL, &error);
  ASSERT_TRUE(server) << ErrorToString(error);

  {
    GlobalLogOverride global_log_override;
    ASSERT_FALSE(server->InitAsync(
        Bind(&HttpServerTest::OnDestroy, Unretained(this)), &error));
  }

  server = NULL;

  ASSERT_TRUE(core_);
  delete core_;
  core_ = NULL;
}

TEST_F(HttpServerTest, InitSuccess) {
  EXPECT_CALL(*this, CreateCoreProxy(_, NotNull(), _))
      .WillOnce(DoAll(SetArgPointee<2>(true), Return(true)));

  Error error;
  SharedHttpServer server =
      HttpServer::Create(live_list(), config_, SharedUvLoop(),
                         SharedHttpHandler(), this, NULL, NULL, &error);
  ASSERT_TRUE(server) << ErrorToString(error);

  ASSERT_TRUE(server->InitAsync(
      Bind(&HttpServerTest::OnDestroy, Unretained(this)), &error));

  server = NULL;

  ASSERT_TRUE(core_);
  delete core_;
  core_ = NULL;
}

}  // namespace pjcore
