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

#ifndef PJCORE_HTTP_CLIENT_CORE_H_
#define PJCORE_HTTP_CLIENT_CORE_H_

#include <map>

#include "pjcore/http_client_connection_group.h"
#include "pjcore/http_util.h"
#include "pjcore/live_capturable.h"
#include "pjcore/url_util.h"

namespace pjcore {

class HttpClientCore : public LiveCapturable {
 public:
  HttpClientCore(LiveCapturableList* live_list, void* shell);

  ~HttpClientCore();

  bool Init(const HttpClientConfig& config, const SharedUvLoop& shared_loop,
            AbstractHttpClientTransactionFactory* transaction_factory,
            Error* error);

  const SharedUvLoop& shared_loop() const { return shared_loop_; }

  void AsyncProcess(scoped_ptr<HttpRequest> request,
                    const HttpResponseCallback& on_response);

  void OnShellDestroyed(void* shell, const Closure& optional_on_destroy);

 protected:
  scoped_ptr<google::protobuf::Message> CaptureLive() const OVERRIDE;

 private:
  struct NodeAndServiceNameLess {
    bool operator()(const NodeAndServiceName& left,
                    const NodeAndServiceName& right) const;
  };

  typedef std::map<NodeAndServiceName, HttpClientConnectionGroup*,
                   NodeAndServiceNameLess> ConnectionGroupMap;

  class PendingTransactionCounter : public AbstractCounter {
   public:
    explicit PendingTransactionCounter(HttpClientCore* core)
        : core_(core), count_(0) {}

    void Increment() OVERRIDE;

    void Decrement() OVERRIDE;

    size_t count() const { return count_; }

   private:
    HttpClientCore* core_;

    size_t count_;
  };

  void ConsiderDestroy();

  void* shell_;

  NonNegativeCounter pending_resolve_counter_;

  NonNegativeCounter pending_connection_counter_;

  PendingTransactionCounter pending_transaction_counter_;

  HttpClientConfig config_;

  SharedUvLoop shared_loop_;

  AbstractHttpClientTransactionFactory* transaction_factory_;

  ConnectionGroupMap connection_group_map_;

  Closure on_destroy_;
};

}  // namespace pjcore

#endif  // PJCORE_HTTP_CLIENT_CORE_H_
