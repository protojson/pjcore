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

#ifndef PJCORE_HTTP_CLIENT_CONNECTION_H_
#define PJCORE_HTTP_CLIENT_CONNECTION_H_

#include <list>

#include "pjcore/third_party/chromium/scoped_ptr.h"
#include "pjcore/third_party/chromium/callback.h"

#include "pjcore/abstract_http_client_transaction.h"
#include "pjcore/abstract_http_parser.h"
#include "pjcore/http_util.h"
#include "pjcore/shared_addr_info_list.h"
#include "pjcore/shared_uv_loop.h"

namespace pjcore {

class HttpClientConnection : public LiveCapturable {
 public:
  HttpClientConnection(LiveCapturableList* live_list,
                       const SharedUvLoop& shared_loop,
                       AbstractCounter* pending_connection_counter,
                       const SharedAddrInfoList& addr_infos);

  ~HttpClientConnection();

  bool InitSync(AbstractHttpParserFactory* parser_factory, Error* error);

  void InitAsync(
      std::list<AbstractHttpClientTransaction*>* transactions,
      const Callback<void(bool result, const Error& error)>& on_closing,
      const Closure& on_close);

  void Close(bool result, const Error& error);

  const SharedUvLoop& shared_loop() const { return shared_loop_; }

  AbstractUv* uv() const { return shared_loop()->uv(); }

  void AsyncProcess(scoped_ptr<AbstractHttpClientTransaction> transaction);

 protected:
  scoped_ptr<google::protobuf::Message> CaptureLive() const OVERRIDE;

 private:
  static void StaticOnConnect(uv_connect_t* req, int status);

  bool OnConnect(int status);

  static void StaticAlloc(uv_handle_t* handle, size_t suggested_size,
                          uv_buf_t* buf);

  bool Alloc(size_t suggested_size, uv_buf_t* buf, Error* error);

  static void StaticOnRead(uv_stream_t* stream, ssize_t nread,
                           const uv_buf_t* buf);

  bool OnRead(ssize_t nread, const uv_buf_t* buf, Error* error);

  void OnTransactionHandleComplete(AbstractHttpClientTransaction* transaction);

  void OnTransactionWriteComplete(AbstractHttpClientTransaction* transaction,
                                  bool result, const Error& error);

  static void StaticOnClose(uv_handle_t* handle);

  SharedUvLoop shared_loop_;

  AbstractCounter::Unit pending_connection_counter_unit_;

  SharedAddrInfoList addr_infos_;

  bool open_;

  std::list<AbstractHttpClientTransaction*> wait_queue_;

  std::list<AbstractHttpClientTransaction*> write_queue_;

  std::list<AbstractHttpClientTransaction*> read_queue_;

  Callback<void(bool result, const Error& error)> on_closing_;

  Closure on_close_;

  uv_tcp_t connect_handle_;

  uv_connect_t connect_req_;

  bool reading_;

  bool closing_;

  scoped_ptr<AbstractHttpParser> parser_;

  size_t buffer_size_;

  scoped_ptr<char[]> buffer_;
};

}  // namespace pjcore

#endif  // PJCORE_HTTP_CLIENT_CONNECTION_H_
