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

#ifndef PJCORE_HTTP_SERVER_CORE_H_
#define PJCORE_HTTP_SERVER_CORE_H_

#include <list>

#include "pjcore/abstract_http_server_core.h"

namespace pjcore {

class HttpServerCore : public AbstractHttpServerCore {
 public:
  HttpServerCore(LiveCapturableList* live_list, void* shell);

  ~HttpServerCore();

  bool InitSync(const HttpServerConfig& config, const SharedUvLoop& shared_loop,
                const SharedHttpHandler& handler,
                AbstractHttpServerConnectionFactory* connection_factory,
                AbstractHttpParserFactory* parser_factory,
                Error* error) OVERRIDE;

  bool InitAsync(Error* error) OVERRIDE;

  const HttpServerConfig& config() const { return config_; }

  const SharedUvLoop& shared_loop() const { return shared_loop_; }

  AbstractUv* uv() const { return shared_loop_->uv(); }

  const SharedHttpHandler& handler() const { return handler_; }

  const uv_tcp_t& listen_handle() const { return listen_handle_; }

  uv_tcp_t* mutable_listen_handle() { return &listen_handle_; }

  void OnShellDestroyed(void* shell,
                        const Closure& optional_on_destroy) OVERRIDE;

  static const int32_t kDefaultPort = 80;

  static const int32_t kDefaultBacklog = 128;

 protected:
  scoped_ptr<google::protobuf::Message> CaptureLive() const OVERRIDE;

 private:
  static void StaticOnConnected(uv_stream_t* handle, int status);

  bool OnConnected(int status, Error* error);

  static void StaticOnClose(uv_handle_t* handle);

  void OnConnectionClosing(AbstractHttpServerConnection* connection,
                           bool result, const Error& error);

  void OnConnectionClose(AbstractHttpServerConnection* connection);

  void ConsiderDestroy();

  void* shell_;

  bool open_;

  NonNegativeCounter pending_connection_counter_;

  NonNegativeCounter pending_transaction_counter_;

  HttpServerConfig config_;

  SharedUvLoop shared_loop_;

  SharedHttpHandler handler_;

  AbstractHttpServerConnectionFactory* connection_factory_;

  AbstractHttpParserFactory* parser_factory_;

  uv_tcp_t listen_handle_;

  sockaddr_in address_;

  Closure on_destroy_;

  std::list<AbstractHttpServerConnection*> open_connections_;

  std::list<AbstractHttpServerConnection*> closing_connections_;
};

}  // namespace pjcore

#endif  // PJCORE_HTTP_SERVER_CORE_H_
