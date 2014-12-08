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

#ifndef PJCORE_HTTP_SERVER_CONNECTION_H_
#define PJCORE_HTTP_SERVER_CONNECTION_H_

#include <list>

#include "pjcore/third_party/chromium/callback.h"
#include "pjcore/third_party/chromium/scoped_ptr.h"

#include "pjcore/abstract_http_handler.h"
#include "pjcore/abstract_http_server_connection.h"
#include "pjcore/abstract_uv.h"
#include "pjcore/abstract_http_parser.h"
#include "pjcore/http_server_transaction.h"
#include "pjcore/live_capturable.h"
#include "pjcore/shared_uv_loop.h"

namespace pjcore {

class HttpServerConnection : public AbstractHttpServerConnection {
 public:
  HttpServerConnection(LiveCapturableList* live_list,
                       const SharedUvLoop& shared_loop,
                       AbstractCounter* pending_connection_counter,
                       AbstractCounter* pending_transaction_counter,
                       uv_tcp_t* listen_handle,
                       const SharedHttpHandler& handler);

  ~HttpServerConnection();

  const SharedUvLoop& shared_loop() const { return shared_loop_; }

  AbstractUv* uv() const { return shared_loop()->uv(); }

  bool InitSync(AbstractHttpParserFactory* parser_factory,
                Error* error) OVERRIDE;

  void InitAsync(
      const Callback<void(bool result, const Error& error)>& on_closing,
      const Closure& on_close) OVERRIDE;

  void Close(bool result, const Error& error) OVERRIDE;

 protected:
  scoped_ptr<google::protobuf::Message> CaptureLive() const OVERRIDE;

 private:
  static void StaticAlloc(uv_handle_t* handle, size_t suggested_size,
                          uv_buf_t* buf);

  bool Alloc(size_t suggested_size, uv_buf_t* buf, Error* error);

  static void StaticOnRead(uv_stream_t* stream, ssize_t nread,
                           const uv_buf_t* buf);

  bool OnRead(ssize_t nread, const uv_buf_t* buf, Error* error);

  void OnTransactionHandleComplete(HttpServerTransaction* transaction,
                                   scoped_ptr<HttpResponse> response,
                                   const Error& error);

  void OnTransactionWriteComplete(HttpServerTransaction* transaction,
                                  bool result, const Error& error);

  void Close();

  static void StaticOnClose(uv_handle_t* handle);

  SharedUvLoop shared_loop_;

  AbstractCounter::Unit pending_connection_counter_unit_;

  NonNegativeCounter local_pending_transaction_counter_;

  CounterSplitter pending_transaction_counter_splitter_;

  uv_tcp_t* listen_handle_;

  SharedHttpHandler handler_;

  bool open_;

  uv_tcp_t accept_handle_;

  std::list<HttpServerTransaction*> handle_queue_;

  std::list<HttpServerTransaction*> write_queue_;

  Callback<void(bool result, const Error& error)> on_closing_;

  Closure on_close_;

  bool reading_;

  bool closing_;

  scoped_ptr<AbstractHttpParser> parser_;

  size_t buffer_size_;

  scoped_ptr<char[]> buffer_;
};

}  // namespace pjcore

#endif  // PJCORE_HTTP_SERVER_CONNECTION_H_
