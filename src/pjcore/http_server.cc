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

#include <algorithm>

#include "pjcore/abstract_http_server_core.h"
#include "pjcore/live_http.pb.h"
#include "pjcore/live_util.h"
#include "pjcore/logging.h"

namespace pjcore {

SharedHttpServer HttpServer::Create(const HttpServerConfig& config,
                                    const SharedHttpHandler& handler,
                                    Error* error) {
  return Create(NULL, config, GetDefaultUvLoop(NULL), handler, error);
}

SharedHttpServer HttpServer::Create(LiveCapturableList* live_list,
                                    const HttpServerConfig& config,
                                    const SharedUvLoop& shared_loop,
                                    const SharedHttpHandler& handler,
                                    Error* error) {
  return Create(live_list, config, shared_loop, handler,
                GetDefaultHttpServerCoreFactory(),
                GetDefaultHttpServerConnectionFactory(),
                GetDefaultHttpParserFactory(), error);
}

SharedHttpServer HttpServer::Create(
    LiveCapturableList* live_list, const HttpServerConfig& config,
    const SharedUvLoop& shared_loop, const SharedHttpHandler& handler,
    AbstractHttpServerCoreFactory* core_factory,
    AbstractHttpServerConnectionFactory* connection_factory,
    AbstractHttpParserFactory* parser_factory, Error* error) {
  PJCORE_CHECK(core_factory);
  PJCORE_CHECK(error);
  error->Clear();

  SharedHttpServer server(new HttpServer(live_list, core_factory));

  PJCORE_NULL_REQUIRE_SILENT(
      server->core_->InitSync(config, shared_loop, handler, connection_factory,
                              parser_factory, error),
      "Failed to create HTTP server");

  return server;
}

bool HttpServer::InitAsync(const Closure& on_destroy, Error* error) {
  PJCORE_CHECK(!on_destroy.is_null());
  PJCORE_CHECK(error);
  error->Clear();

  PJCORE_CHECK(on_destroy_.is_null());
  on_destroy_ = on_destroy;

  PJCORE_REQUIRE_SILENT(core_->InitAsync(error),
                        "Failed to initialize HTTP server");

  return true;
}

scoped_ptr<google::protobuf::Message> HttpServer::CaptureLive() const {
  scoped_ptr<LiveHttpServer> live(new LiveHttpServer());
  live->set_core_ptr(reinterpret_cast<uint64_t>(core_));
  if (!on_destroy_.is_null()) {
    CaptureLiveCallbackOut(on_destroy_, live->mutable_on_destroy());
  }
  return scoped_ptr<google::protobuf::Message>(live.release());
}

HttpServer::HttpServer(LiveCapturableList* live_list,
                       AbstractHttpServerCoreFactory* core_factory)
    : LiveCapturable("pjcore::HttpServer", live_list),
      core_(core_factory->CreateCore(live_list, this).release()) {
  core_->set_live_parent(this);
}

HttpServer::~HttpServer() {
  LogDestroy();

  AbstractHttpServerCore* local_core = NULL;
  Closure local_on_destroy;

  using std::swap;
  swap(local_core, core_);
  swap(local_on_destroy, on_destroy_);

  local_core->OnShellDestroyed(this, local_on_destroy);
}

}  // namespace pjcore
