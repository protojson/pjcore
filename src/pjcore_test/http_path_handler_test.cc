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

#include "pjcore/http_path_handler.h"

#include <gmock/gmock.h>

#include <http_parser.h>

#include "pjcore/third_party/chromium/bind.h"

#include "pjcore/error_util.h"
#include "pjcore/url_util.h"

#include "pjcore_test/is_gmock_verbose.h"
#include "pjcore_test/mock_http_handler.h"

namespace pjcore {

using ::testing::Exactly;
using ::testing::NotNull;
using ::testing::Return;
using ::testing::_;

namespace {

class MockHttpResponseSink {
 public:
  void Sink(scoped_ptr<HttpResponse> result, const Error& error) {
    SinkProxy(result.get(), error);
  }

  MOCK_METHOD2(SinkProxy, void(HttpResponse* result, const Error& error));
};

class HttpPathHandlerTest : public ::testing::Test {
 public:
  HttpPathHandlerTest()
      : live_list_(new LiveCapturableList(IsGmockVerbose())),
        path_handler_(new HttpPathHandler(live_list_.get())) {}

  ::testing::AssertionResult TestAsyncHandle(
      StringPiece path, MockHttpResponseSink* response_sink) {
    scoped_ptr<HttpRequest> request(new HttpRequest());
    request->set_method(HTTP_GET);
    path.AppendToString(request->mutable_url());

    Error url_error;
    if (!ParseUrl(request->url(), false, request->mutable_parsed_url(),
                  &url_error)) {
      return ::testing::AssertionFailure() << "Failed to parse URL "
                                           << request->url() << ": "
                                           << ErrorToString(url_error);
    }

    path_handler_->AsyncHandle(request.Pass(), Bind(&MockHttpResponseSink::Sink,
                                                    Unretained(response_sink)));
    return ::testing::AssertionSuccess();
  }

  scoped_ptr<LiveCapturableList> live_list_;

  SharedHttpPathHandler path_handler_;
};

}  // unnamed namespace

TEST_F(HttpPathHandlerTest, NoPaths) {
  MockHttpResponseSink response_sink;

  EXPECT_CALL(response_sink, SinkProxy(NULL, _)).Times(Exactly(1));

  EXPECT_TRUE(TestAsyncHandle("/alpha", &response_sink));
}

TEST_F(HttpPathHandlerTest, OnePathMatch) {
  SharedMockHttpHandler alpha_handler(new MockHttpHandler(live_list_.get()));
  (*path_handler_->mutable_map())["/alpha"] = alpha_handler;

  MockHttpResponseSink response_sink;

  EXPECT_CALL(*alpha_handler, AsyncHandleProxy(_, _, _, _))
      .WillOnce(Return(true));
  EXPECT_CALL(response_sink, SinkProxy(NotNull(), _)).Times(Exactly(1));

  EXPECT_TRUE(TestAsyncHandle("/alpha", &response_sink));
}

TEST_F(HttpPathHandlerTest, OnePathMiss) {
  SharedMockHttpHandler alpha_handler(new MockHttpHandler(live_list_.get()));
  (*path_handler_->mutable_map())["/alpha"] = alpha_handler;

  MockHttpResponseSink response_sink;

  EXPECT_CALL(*alpha_handler, AsyncHandleProxy(_, _, _, _)).Times(Exactly(0));
  EXPECT_CALL(response_sink, SinkProxy(NULL, _)).Times(Exactly(1));

  EXPECT_TRUE(TestAsyncHandle("/beta", &response_sink));
}

TEST_F(HttpPathHandlerTest, TwoPathsMatch) {
  SharedMockHttpHandler alpha_handler(new MockHttpHandler(live_list_.get()));
  (*path_handler_->mutable_map())["/alpha"] = alpha_handler;

  SharedMockHttpHandler beta_handler(new MockHttpHandler(live_list_.get()));
  (*path_handler_->mutable_map())["/beta"] = beta_handler;

  MockHttpResponseSink response_sink;

  EXPECT_CALL(*alpha_handler, AsyncHandleProxy(_, _, _, _)).Times(Exactly(0));
  EXPECT_CALL(*beta_handler, AsyncHandleProxy(_, _, _, _))
      .WillOnce(Return(true));
  EXPECT_CALL(response_sink, SinkProxy(NotNull(), _)).Times(Exactly(1));

  EXPECT_TRUE(TestAsyncHandle("/beta", &response_sink));
}

TEST_F(HttpPathHandlerTest, TwoPathsMiss) {
  SharedMockHttpHandler alpha_handler(new MockHttpHandler(live_list_.get()));
  (*path_handler_->mutable_map())["/alpha"] = alpha_handler;

  SharedMockHttpHandler beta_handler(new MockHttpHandler(live_list_.get()));
  (*path_handler_->mutable_map())["/beta"] = beta_handler;

  MockHttpResponseSink response_sink;

  EXPECT_CALL(*alpha_handler, AsyncHandleProxy(_, _, _, _)).Times(Exactly(0));
  EXPECT_CALL(*beta_handler, AsyncHandleProxy(_, _, _, _)).Times(Exactly(0));
  EXPECT_CALL(response_sink, SinkProxy(NULL, _)).Times(Exactly(1));

  EXPECT_TRUE(TestAsyncHandle("/gamma", &response_sink));
}

TEST_F(HttpPathHandlerTest, Default) {
  SharedMockHttpHandler alpha_handler(new MockHttpHandler(live_list_.get()));
  (*path_handler_->mutable_map())["/alpha"] = alpha_handler;

  SharedMockHttpHandler beta_handler(new MockHttpHandler(live_list_.get()));
  (*path_handler_->mutable_map())["/beta"] = beta_handler;

  SharedMockHttpHandler default_handler(new MockHttpHandler(live_list_.get()));
  path_handler_->set_default_handler(default_handler);

  MockHttpResponseSink response_sink;

  EXPECT_CALL(*alpha_handler, AsyncHandleProxy(_, _, _, _)).Times(Exactly(0));
  EXPECT_CALL(*beta_handler, AsyncHandleProxy(_, _, _, _)).Times(Exactly(0));
  EXPECT_CALL(*default_handler, AsyncHandleProxy(_, _, _, _))
      .WillOnce(Return(true));
  EXPECT_CALL(response_sink, SinkProxy(NotNull(), _)).Times(Exactly(1));

  EXPECT_TRUE(TestAsyncHandle("/gamma", &response_sink));
}

}  // namespace pjcore
