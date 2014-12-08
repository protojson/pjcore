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

#ifndef PJCORE_ABSTRACT_HTTP_SERVER_CONNECTION_H_
#define PJCORE_ABSTRACT_HTTP_SERVER_CONNECTION_H_

#include "pjcore/abstract_uv.h"

#include "pjcore/third_party/chromium/scoped_ptr.h"
#include "pjcore/third_party/chromium/callback.h"

#include "pjcore/abstract_counter.h"
#include "pjcore/abstract_http_handler.h"
#include "pjcore/abstract_http_parser.h"
#include "pjcore/live_capturable.h"
#include "pjcore/shared_uv_loop.h"

namespace pjcore {

class AbstractHttpServerConnection : public LiveCapturable {
 public:
  virtual ~AbstractHttpServerConnection();

  virtual bool InitSync(AbstractHttpParserFactory* parser_factory,
                        Error* error) = 0;

  virtual void InitAsync(
      const Callback<void(bool result, const Error& error)>& on_closing,
      const Closure& on_close) = 0;

  virtual void Close(bool result, const Error& error) = 0;

 protected:
  AbstractHttpServerConnection(const char* live_class,
                               LiveCapturableList* live_list);
};

class AbstractHttpServerConnectionFactory {
 public:
  virtual ~AbstractHttpServerConnectionFactory();

  virtual scoped_ptr<AbstractHttpServerConnection> CreateConnection(
      LiveCapturableList* live_list, const SharedUvLoop& shared_loop,
      AbstractCounter* pending_connection_counter,
      AbstractCounter* pending_transaction_counter, uv_tcp_t* listen_handle,
      const SharedHttpHandler& handler) = 0;
};

AbstractHttpServerConnectionFactory* GetDefaultHttpServerConnectionFactory();

}  // namespace pjcore

#endif  // PJCORE_ABSTRACT_HTTP_SERVER_CONNECTION_H_
