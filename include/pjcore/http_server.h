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

#ifndef PJCORE_HTTP_SERVER_H_
#define PJCORE_HTTP_SERVER_H_

#include "pjcore/abstract_http_handler.h"
#include "pjcore/shared_uv_loop.h"

namespace pjcore {

class AbstractHttpServerCore;
class AbstractHttpServerCoreFactory;
class AbstractHttpServerConnectionFactory;
class AbstractHttpParserFactory;

class HttpServer;
typedef scoped_refptr<HttpServer> SharedHttpServer;

class HttpServer : public LiveCapturable, public RefCounted<HttpServer> {
 public:
  static SharedHttpServer Create(const HttpServerConfig& config,
                                 const SharedHttpHandler& handler,
                                 Error* error);

  static SharedHttpServer Create(LiveCapturableList* live_list,
                                 const HttpServerConfig& config,
                                 const SharedUvLoop& shared_loop,
                                 const SharedHttpHandler& handler,
                                 Error* error);

  static SharedHttpServer Create(
      LiveCapturableList* live_list, const HttpServerConfig& config,
      const SharedUvLoop& shared_loop, const SharedHttpHandler& handler,
      AbstractHttpServerCoreFactory* core_factory,
      AbstractHttpServerConnectionFactory* connection_factory,
      AbstractHttpParserFactory* parser_factory, Error* error);

  bool InitAsync(const Closure& on_destroy, Error* error);

 protected:
  scoped_ptr<google::protobuf::Message> CaptureLive() const OVERRIDE;

 private:
  friend class RefCounted<HttpServer>;

  HttpServer(LiveCapturableList* live_list,
             AbstractHttpServerCoreFactory* core_factory);

  ~HttpServer();

  AbstractHttpServerCore* core_;

  Closure on_destroy_;
};

}  // namespace pjcore

#endif  // PJCORE_HTTP_SERVER_H_
