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

#include <algorithm>

#include "pjcore/third_party/chromium/bind.h"

#include "pjcore/error_util.h"
#include "pjcore/logging.h"
#include "pjcore/live_util.h"
#include "pjcore/http_util.h"

namespace pjcore {

HttpServerCore::HttpServerCore(LiveCapturableList* live_list, void* shell)
    : AbstractHttpServerCore("pjcore::HttpServerCore", live_list),
      shell_(shell),
      open_(false),
      pending_connection_counter_(
          Bind(&HttpServerCore::ConsiderDestroy, Unretained(this))),
      pending_transaction_counter_(
          Bind(&HttpServerCore::ConsiderDestroy, Unretained(this))),
      connection_factory_(NULL),
      parser_factory_(NULL) {
  memset(&listen_handle_, 0, sizeof(listen_handle_));
  listen_handle_.data = this;
}

HttpServerCore::~HttpServerCore() {
  LogDestroy();

  PJCORE_CHECK(!shell_);
  PJCORE_CHECK(!open_);
  PJCORE_CHECK(!pending_connection_counter_.count());
  PJCORE_CHECK(!pending_transaction_counter_.count());

  PJCORE_CHECK(on_destroy_.is_null());
}

bool HttpServerCore::InitSync(
    const HttpServerConfig& config, const SharedUvLoop& shared_loop,
    const SharedHttpHandler& handler,
    AbstractHttpServerConnectionFactory* connection_factory,
    AbstractHttpParserFactory* parser_factory, Error* error) {
  PJCORE_CHECK(shared_loop);
  PJCORE_CHECK(handler);
  PJCORE_CHECK(connection_factory);
  PJCORE_CHECK(parser_factory);

  PJCORE_CHECK(error);
  error->Clear();

  config_ = config;

  if (!config_.has_port()) {
    config_.set_port(kDefaultPort);
  }

  if (!config_.has_backlog()) {
    config_.set_backlog(kDefaultBacklog);
  }

  shared_loop_ = shared_loop;
  handler_ = handler;
  connection_factory_ = connection_factory;
  parser_factory_ = parser_factory;

  PJCORE_UV_REQUIRE(uv()->ip4_addr("0.0.0.0", config_.port(), &address_),
                    "Failed to convert port");
  return true;
}

bool HttpServerCore::InitAsync(Error* error) {
  PJCORE_CHECK(error);
  error->Clear();

  PJCORE_CHECK(!open_);

  PJCORE_UV_REQUIRE(uv()->tcp_init(shared_loop_->loop(), &listen_handle_),
                    "Failed to initialize TCP");

  open_ = true;

  PJCORE_UV_REQUIRE(
      uv()->tcp_bind(&listen_handle_, (const struct sockaddr*)&address_, 0),
      "Failed to bind to port");

  PJCORE_UV_REQUIRE(
      uv()->listen(reinterpret_cast<uv_stream_t*>(&listen_handle_),
                   config_.backlog(), &HttpServerCore::StaticOnConnected),
      "Failed to listen on port");

  return true;
}

void HttpServerCore::OnShellDestroyed(void* shell,
                                      const Closure& optional_on_destroy) {
  PJCORE_CHECK(shell);
  PJCORE_CHECK(shell_);
  PJCORE_CHECK_EQ(shell_, shell);
  PJCORE_CHECK(on_destroy_.is_null());

  shell_ = NULL;
  on_destroy_ = optional_on_destroy;

  if (open_) {
    uv()->close(reinterpret_cast<uv_handle_t*>(&listen_handle_),
                &StaticOnClose);
  }

  ConsiderDestroy();
}

scoped_ptr<google::protobuf::Message> HttpServerCore::CaptureLive() const {
  scoped_ptr<LiveHttpServerCore> live(new LiveHttpServerCore());
  if (shell_) {
    live->set_shell_ptr(reinterpret_cast<int64_t>(shell_));
  }
  if (open_) {
    live->set_open(open_);
  }
  if (pending_connection_counter_.count()) {
    live->set_pending_connection_count(pending_connection_counter_.count());
  }
  if (pending_transaction_counter_.count()) {
    live->set_pending_transaction_count(pending_transaction_counter_.count());
  }
  live->mutable_config()->CopyFrom(config_);
  live->set_shared_loop_ptr(reinterpret_cast<uint64_t>(shared_loop_.get()));
  if (handler_) {
    live->set_handler_ptr(reinterpret_cast<int64_t>(handler_.get()));
  }
  live->set_listen_handle_ptr(reinterpret_cast<uint64_t>(&listen_handle_));
  if (!on_destroy_.is_null()) {
    CaptureLiveCallbackOut(on_destroy_, live->mutable_on_destroy());
  }
  return scoped_ptr<google::protobuf::Message>(live.release());
}

void HttpServerCore::StaticOnConnected(uv_stream_t* handle, int status) {
  PJCORE_CHECK(handle);

  HttpServerCore* core = static_cast<HttpServerCore*>(handle->data);
  PJCORE_CHECK(core);

  Error error;
  if (!core->OnConnected(status, &error)) {
    PJCORE_LOG_ERROR("OnConnected callback has failed", &error);
  }
}

bool HttpServerCore::OnConnected(int status, Error* error) {
  PJCORE_CHECK(error);
  error->Clear();

  PJCORE_UV_REQUIRE(status, "Error status");

  scoped_ptr<AbstractHttpServerConnection> connection =
      connection_factory_->CreateConnection(
          live_list(), shared_loop_, &pending_connection_counter_,
          &pending_transaction_counter_, &listen_handle_, handler_);
  connection->set_live_parent(this);

  PJCORE_REQUIRE_SILENT(connection->InitSync(parser_factory_, error),
                        "Failed to accept");

  open_connections_.push_back(connection.release());

  open_connections_.back()->InitAsync(
      Bind(&HttpServerCore::OnConnectionClosing, Unretained(this),
           open_connections_.back()),
      Bind(&HttpServerCore::OnConnectionClose, Unretained(this),
           open_connections_.back()));

  return true;
}

void HttpServerCore::StaticOnClose(uv_handle_t* handle) {
  PJCORE_CHECK(handle);

  HttpServerCore* core = static_cast<HttpServerCore*>(handle->data);
  PJCORE_CHECK(core);
  PJCORE_CHECK(core->open_);

  core->open_ = false;

  std::list<AbstractHttpServerConnection*> local_connections =
      core->open_connections_;
  for (std::list<AbstractHttpServerConnection*>::iterator it =
           local_connections.begin();
       it != local_connections.end(); ++it) {
    (*it)->Close(true, Error());
  }

  core->ConsiderDestroy();
}

void HttpServerCore::OnConnectionClosing(
    AbstractHttpServerConnection* connection, bool /* result */,
    const Error& /* error */) {
  PJCORE_CHECK(connection);

  std::list<AbstractHttpServerConnection*>::iterator it =
      std::find(open_connections_.begin(), open_connections_.end(), connection);

  PJCORE_CHECK(it != open_connections_.end());

  open_connections_.erase(it);

  closing_connections_.push_back(connection);
}

void HttpServerCore::OnConnectionClose(
    AbstractHttpServerConnection* connection) {
  PJCORE_CHECK(connection);

  std::list<AbstractHttpServerConnection*>::iterator it = std::find(
      closing_connections_.begin(), closing_connections_.end(), connection);

  PJCORE_CHECK(it != closing_connections_.end());

  closing_connections_.erase(it);

  delete connection;
}

void HttpServerCore::ConsiderDestroy() {
  if (shell_ || open_ || pending_connection_counter_.count() ||
      pending_transaction_counter_.count()) {
    return;
  }

  Closure local_on_destroy;
  using std::swap;
  swap(local_on_destroy, on_destroy_);

  delete this;

  if (!local_on_destroy.is_null()) {
    local_on_destroy.Run();
  }
}

}  // namespace pjcore
