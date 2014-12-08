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

#ifndef PJCORE_ABSTRACT_HTTP_SERVER_CORE_H_
#define PJCORE_ABSTRACT_HTTP_SERVER_CORE_H_

#include "pjcore/abstract_http_server_connection.h"

namespace pjcore {

class AbstractHttpServerCore : public LiveCapturable {
 public:
  virtual ~AbstractHttpServerCore();

  virtual bool InitSync(const HttpServerConfig& config,
                        const SharedUvLoop& shared_loop,
                        const SharedHttpHandler& handler,
                        AbstractHttpServerConnectionFactory* connection_factory,
                        AbstractHttpParserFactory* parser_factory,
                        Error* error) = 0;

  virtual bool InitAsync(Error* error) = 0;

  virtual void OnShellDestroyed(void* shell,
                                const Closure& optional_on_destroy) = 0;

 protected:
  AbstractHttpServerCore(const char* live_class, LiveCapturableList* live_list);
};

class AbstractHttpServerCoreFactory {
 public:
  virtual ~AbstractHttpServerCoreFactory();

  virtual scoped_ptr<AbstractHttpServerCore> CreateCore(
      LiveCapturableList* live_list, void* shell) = 0;
};

AbstractHttpServerCoreFactory* GetDefaultHttpServerCoreFactory();

}  // namespace pjcore

#endif  // PJCORE_ABSTRACT_HTTP_SERVER_CORE_H_
