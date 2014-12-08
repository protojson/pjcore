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

#ifndef PJCORE_HTTP_CLIENT_CONNECTION_GROUP_H_
#define PJCORE_HTTP_CLIENT_CONNECTION_GROUP_H_

#include <list>

#include "pjcore/third_party/chromium/scoped_ptr.h"
#include "pjcore/third_party/chromium/callback.h"

#include "pjcore/http_client_connection.h"
#include "pjcore/http_util.h"
#include "pjcore/shared_addr_info_list.h"

namespace pjcore {

class HttpClientConnectionGroup : public LiveCapturable {
 public:
  HttpClientConnectionGroup(LiveCapturableList* live_list,
                            const SharedUvLoop& shared_loop,
                            AbstractCounter* pending_resolve_counter,
                            AbstractCounter* pending_connection_counter,
                            const NodeAndServiceName& node_and_service_name);

  ~HttpClientConnectionGroup();

  const SharedUvLoop& shared_loop() const { return shared_loop_; }

  AbstractUv* uv() const { return shared_loop()->uv(); }

  const NodeAndServiceName& node_and_service_name() const {
    return node_and_service_name_;
  }

  void AsyncProcess(scoped_ptr<AbstractHttpClientTransaction> transaction);

  void Close(bool result, const Error& error);

 protected:
  scoped_ptr<google::protobuf::Message> CaptureLive() const OVERRIDE;

 private:
  static void StaticOnResolve(uv_getaddrinfo_t* req, int status, addrinfo* res);

  bool OnResolve(int status, addrinfo* res);

  void OnConnectionOpen(HttpClientConnection* connection, bool result,
                        const Error& error);

  void OnConnectionClosing(HttpClientConnection* connection, bool result,
                           const Error& error);

  void OnConnectionClose(HttpClientConnection* connection);

  SharedUvLoop shared_loop_;

  NonNegativeCounter local_pending_resolve_counter_;

  CounterSplitter pending_resolve_counter_splitter_;

  NonNegativeCounter local_pending_connection_counter_;

  CounterSplitter pending_connection_counter_splitter_;

  const NodeAndServiceName node_and_service_name_;

  std::list<AbstractHttpClientTransaction*> wait_transactions_;

  addrinfo resolve_hints_;

  uv_getaddrinfo_t resolve_req_;

  SharedAddrInfoList addr_infos_;

  std::list<HttpClientConnection*> open_connections_;

  std::list<HttpClientConnection*> closing_connections_;

  bool cancel_opening_;

  bool cancel_opening_result_;

  Error cancel_opening_error_;
};

}  // namespace pjcore

#endif  // PJCORE_HTTP_CLIENT_CONNECTION_GROUP_H_
