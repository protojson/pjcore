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

#include <uv.h>

#include "pjcore/abstract_http_server_connection.h"

#include "pjcore/http_server_connection.h"

namespace pjcore {

namespace {

class DefaultHttpServerConnectionFactory
    : public AbstractHttpServerConnectionFactory {
 public:
  scoped_ptr<AbstractHttpServerConnection> CreateConnection(
      LiveCapturableList* live_list, const SharedUvLoop& shared_loop,
      AbstractCounter* pending_connection_counter,
      AbstractCounter* pending_transaction_counter, uv_tcp_t* listen_handle,
      const SharedHttpHandler& handler) {
    return scoped_ptr<AbstractHttpServerConnection>(new HttpServerConnection(
        live_list, shared_loop, pending_connection_counter,
        pending_transaction_counter, listen_handle, handler));
  }
};

DefaultHttpServerConnectionFactory gDefaultHttpServerConnectionFactory;

}  // unnamed namespace

AbstractHttpServerConnection::~AbstractHttpServerConnection() {}

AbstractHttpServerConnection::AbstractHttpServerConnection(
    const char* live_class, LiveCapturableList* live_list)
    : LiveCapturable(live_class, live_list) {}

AbstractHttpServerConnectionFactory::~AbstractHttpServerConnectionFactory() {}

AbstractHttpServerConnectionFactory* GetDefaultHttpServerConnectionFactory() {
  return &gDefaultHttpServerConnectionFactory;
}

}  // namespace pjcore
