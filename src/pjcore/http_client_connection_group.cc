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

#include "pjcore/http_client_connection_group.h"

#include "pjcore/third_party/chromium/bind.h"

#include "pjcore/logging.h"
#include "pjcore/http_util.h"
#include "pjcore/live_util.h"
#include "pjcore/shared_addr_info_list.h"
#include "pjcore/uv_util.h"

namespace pjcore {

HttpClientConnectionGroup::HttpClientConnectionGroup(
    LiveCapturableList* live_list, const SharedUvLoop& shared_loop,
    AbstractCounter* pending_resolve_counter,
    AbstractCounter* pending_connection_counter,
    const NodeAndServiceName& node_and_service_name)
    : LiveCapturable("pjcore::HttpClientConnectionGroup", live_list),
      shared_loop_(shared_loop),
      pending_resolve_counter_splitter_(pending_resolve_counter,
                                        &local_pending_resolve_counter_),
      pending_connection_counter_splitter_(pending_connection_counter,
                                           &local_pending_connection_counter_),
      node_and_service_name_(node_and_service_name),
      addr_infos_(NULL) {
  PJCORE_CHECK(!NodeAndServiceNameEmpty(node_and_service_name_));

  memset(&resolve_hints_, 0, sizeof(resolve_hints_));
  resolve_hints_.ai_family = PF_INET;
  resolve_hints_.ai_socktype = SOCK_STREAM;
  resolve_hints_.ai_protocol = IPPROTO_TCP;
  resolve_hints_.ai_flags = 0;

  memset(&resolve_req_, 0, sizeof(resolve_req_));
  resolve_req_.data = this;
}

HttpClientConnectionGroup::~HttpClientConnectionGroup() {
  LogDestroy();

  PJCORE_CHECK(open_connections_.empty());

  for (std::list<AbstractHttpClientTransaction*>::const_iterator it =
           wait_transactions_.begin();
       it != wait_transactions_.begin(); ++it) {
    delete *it;
  }
  wait_transactions_.clear();
}

void HttpClientConnectionGroup::AsyncProcess(
    scoped_ptr<AbstractHttpClientTransaction> transaction) {
  PJCORE_CHECK(transaction);

  if (!open_connections_.empty()) {
    open_connections_.front()->AsyncProcess(transaction.Pass());
    return;
  }

  if (!wait_transactions_.empty()) {
    wait_transactions_.push_back(transaction.release());
    return;
  }

  wait_transactions_.push_back(transaction.release());

  pending_resolve_counter_splitter_.Increment();

  int status =
      uv()->getaddrinfo(shared_loop()->loop(), &resolve_req_, &StaticOnResolve,
                        node_and_service_name_.node_name().empty()
                            ? NULL
                            : node_and_service_name_.node_name().c_str(),
                        node_and_service_name_.service_name().empty()
                            ? NULL
                            : node_and_service_name_.service_name().c_str(),
                        &resolve_hints_);

  if (status < 0) {
    StaticOnResolve(&resolve_req_, status, NULL);
  }
}

void HttpClientConnectionGroup::Close(bool result, const Error& error) {
  std::list<HttpClientConnection*> local_connections = open_connections_;
  for (std::list<HttpClientConnection*>::iterator it =
           local_connections.begin();
       it != local_connections.end(); ++it) {
    (*it)->Close(result, error);
  }
}

scoped_ptr<google::protobuf::Message> HttpClientConnectionGroup::CaptureLive()
    const {
  scoped_ptr<LiveHttpClientConnectionGroup> live(
      new LiveHttpClientConnectionGroup());
  live->set_shared_loop_ptr(reinterpret_cast<uint64_t>(shared_loop_.get()));
  if (local_pending_resolve_counter_.count()) {
    live->set_pending_resolve_count(local_pending_resolve_counter_.count());
  }
  if (local_pending_connection_counter_.count()) {
    live->set_pending_connection_count(
        local_pending_connection_counter_.count());
  }
  live->mutable_node_and_service_name()->CopyFrom(node_and_service_name_);
  CaptureLiveAddrInfoOut(&resolve_hints_, live->mutable_resolve_hints());
  live->set_resolve_req_ptr(reinterpret_cast<int64_t>(&resolve_req_));
  if (addr_infos_) {
    addr_infos_->CaptureLiveOut(live->mutable_addr_infos());
  }
  CaptureLiveListPtrOut(open_connections_,
                        live->mutable_open_connections_ptr());
  CaptureLiveListPtrOut(closing_connections_,
                        live->mutable_closing_connections_ptr());
  return scoped_ptr<google::protobuf::Message>(live.release());
}

void HttpClientConnectionGroup::StaticOnResolve(uv_getaddrinfo_t* req,
                                                int status, addrinfo* res) {
  PJCORE_CHECK(req);

  HttpClientConnectionGroup* connection_group =
      static_cast<HttpClientConnectionGroup*>(req->data);
  PJCORE_CHECK(connection_group);

  PJCORE_VOID_REQUIRE_SILENT(connection_group->OnResolve(status, res),
                             "Failed to resolve address");
}

bool HttpClientConnectionGroup::OnResolve(int status, addrinfo* res) {
  if (status < 0) {
    Error error;
    error.set_uv_errno(static_cast<int>(status));
    error.set_description("Failed to resolve address");
    PJCORE_LOG_ERROR(error.description(), &error);

    while (!wait_transactions_.empty()) {
      scoped_ptr<AbstractHttpClientTransaction> transaction(
          wait_transactions_.front());
      wait_transactions_.pop_front();
      transaction->ResultFailure(transaction.Pass(), error);
    }

    pending_resolve_counter_splitter_.Decrement();
    return false;
  }

  addr_infos_ = new AddrInfoList(uv(), res);

  scoped_ptr<HttpClientConnection> opening_connection(new HttpClientConnection(
      live_list(), shared_loop(), &pending_connection_counter_splitter_,
      addr_infos_));
  opening_connection->set_live_parent(this);

  pending_resolve_counter_splitter_.Decrement();

  Error connection_error;
  if (!opening_connection->InitSync(GetDefaultHttpParserFactory(),
                                    &connection_error)) {
    while (!wait_transactions_.empty()) {
      scoped_ptr<AbstractHttpClientTransaction> transaction(
          wait_transactions_.front());
      wait_transactions_.pop_front();
      transaction->ResultFailure(transaction.Pass(), connection_error);
    }

    Error* error = &connection_error;
    PJCORE_FAIL_CAUSE("Failed to initialize connection");
  }

  open_connections_.push_back(opening_connection.release());

  open_connections_.back()->InitAsync(
      &wait_transactions_, Bind(&HttpClientConnectionGroup::OnConnectionClosing,
                                Unretained(this), open_connections_.back()),
      Bind(&HttpClientConnectionGroup::OnConnectionClose, Unretained(this),
           open_connections_.back()));

  return true;
}

void HttpClientConnectionGroup::OnConnectionClosing(
    HttpClientConnection* connection, bool /* result */,
    const Error& /* error */) {
  PJCORE_CHECK(connection);

  std::list<HttpClientConnection*>::iterator it =
      std::find(open_connections_.begin(), open_connections_.end(), connection);

  PJCORE_CHECK(it != open_connections_.end());

  open_connections_.erase(it);

  closing_connections_.push_back(connection);
}

void HttpClientConnectionGroup::OnConnectionClose(
    HttpClientConnection* connection) {
  PJCORE_CHECK(connection);

  std::list<HttpClientConnection*>::iterator it = std::find(
      closing_connections_.begin(), closing_connections_.end(), connection);

  PJCORE_CHECK(it != closing_connections_.end());

  closing_connections_.erase(it);

  delete connection;
}

}  // namespace pjcore
