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

#include "pjcore/http_client_core.h"

#include <algorithm>
#include <utility>

#include "pjcore/third_party/chromium/bind.h"

#include "pjcore/logging.h"
#include "pjcore/live_http.pb.h"
#include "pjcore/live_util.h"
#include "pjcore/uv_util.h"

namespace pjcore {

HttpClientCore::HttpClientCore(LiveCapturableList* live_list, void* shell)
    : LiveCapturable("pjcore::HttpClientCore", live_list),
      shell_(shell),
      pending_resolve_counter_(
          Bind(&HttpClientCore::ConsiderDestroy, Unretained(this))),
      pending_connection_counter_(
          Bind(&HttpClientCore::ConsiderDestroy, Unretained(this))),
      pending_transaction_counter_(this),
      transaction_factory_(NULL) {
  PJCORE_CHECK(shell_);
}

HttpClientCore::~HttpClientCore() {
  LogDestroy();

  PJCORE_CHECK(!shell_);
  PJCORE_CHECK(!pending_transaction_counter_.count());
  PJCORE_CHECK(!pending_resolve_counter_.count());
  PJCORE_CHECK(!pending_connection_counter_.count());

  PJCORE_CHECK(on_destroy_.is_null());

  for (ConnectionGroupMap::iterator it = connection_group_map_.begin();
       it != connection_group_map_.end(); ++it) {
    delete it->second;
    it->second = NULL;
  }
}

bool HttpClientCore::Init(
    const HttpClientConfig& config, const SharedUvLoop& shared_loop,
    AbstractHttpClientTransactionFactory* transaction_factory, Error* error) {
  PJCORE_CHECK(shared_loop);
  PJCORE_CHECK(transaction_factory);
  PJCORE_CHECK(error);
  error->Clear();

  config_ = config;

  PJCORE_CHECK(!shared_loop_);
  shared_loop_ = shared_loop;

  PJCORE_CHECK(!transaction_factory_);
  transaction_factory_ = transaction_factory;

  return true;
}

void HttpClientCore::AsyncProcess(scoped_ptr<HttpRequest> request,
                                  const HttpResponseCallback& on_response) {
  PJCORE_CHECK(request);
  PJCORE_CHECK(!on_response.is_null());

  PJCORE_CHECK(shell_);

  scoped_ptr<AbstractHttpClientTransaction> transaction =
      transaction_factory_->CreateTransaction(live_list(), shared_loop()->uv(),
                                              &pending_transaction_counter_,
                                              request.Pass(), on_response);
  transaction->set_live_parent(this);

  Error error;

  if (!transaction->Init(&error)) {
    PJCORE_LOG_ERROR("Failed to initialize transaction", &error);
    transaction->ResultFailure(transaction.Pass(), error);
    return;
  }

  NodeAndServiceName node_and_service_name =
      transaction->GetNodeAndServiceName();

  PJCORE_CHECK(!NodeAndServiceNameEmpty(node_and_service_name));

  std::pair<ConnectionGroupMap::iterator, bool> insert_result =
      connection_group_map_.insert(
          ConnectionGroupMap::value_type(node_and_service_name, NULL));

  if (insert_result.second) {
    insert_result.first->second = new HttpClientConnectionGroup(
        live_list(), shared_loop(), &pending_resolve_counter_,
        &pending_connection_counter_, node_and_service_name);
    insert_result.first->second->set_live_parent(this);
  }

  insert_result.first->second->AsyncProcess(transaction.Pass());
}

void HttpClientCore::OnShellDestroyed(void* shell,
                                      const Closure& optional_on_destroy) {
  PJCORE_CHECK(shell);
  PJCORE_CHECK(shell_);
  PJCORE_CHECK_EQ(shell_, shell);
  PJCORE_CHECK(on_destroy_.is_null());

  shell_ = NULL;
  on_destroy_ = optional_on_destroy;

  if (!pending_transaction_counter_.count()) {
    for (ConnectionGroupMap::iterator it = connection_group_map_.begin();
         it != connection_group_map_.end(); ++it) {
      it->second->Close(true, Error());
    }
  }

  ConsiderDestroy();
}

scoped_ptr<google::protobuf::Message> HttpClientCore::CaptureLive() const {
  scoped_ptr<LiveHttpClientCore> live(new LiveHttpClientCore());
  if (shell_) {
    live->set_shell_ptr(reinterpret_cast<int64_t>(shell_));
  }
  if (pending_resolve_counter_.count()) {
    live->set_pending_resolve_count(pending_resolve_counter_.count());
  }
  if (pending_connection_counter_.count()) {
    live->set_pending_connection_count(pending_connection_counter_.count());
  }
  if (pending_transaction_counter_.count()) {
    live->set_pending_transaction_count(pending_transaction_counter_.count());
  }
  live->mutable_config()->CopyFrom(config_);
  live->set_shared_loop_ptr(reinterpret_cast<uint64_t>(shared_loop_.get()));
  for (ConnectionGroupMap::const_iterator it = connection_group_map_.begin();
       it != connection_group_map_.end(); ++it) {
    live->add_connection_groups_ptr(reinterpret_cast<int64_t>(it->second));
  }
  if (!on_destroy_.is_null()) {
    CaptureLiveCallbackOut(on_destroy_, live->mutable_on_destroy());
  }
  return scoped_ptr<google::protobuf::Message>(live.release());
}

bool HttpClientCore::NodeAndServiceNameLess::operator()(
    const NodeAndServiceName& left, const NodeAndServiceName& right) const {
  if (left.node_name() < right.node_name()) {
    return true;
  } else if (right.node_name() < left.node_name()) {
    return false;
  } else {
    return left.service_name() < right.service_name();
  }
}

void HttpClientCore::PendingTransactionCounter::Increment() { ++count_; }

void HttpClientCore::PendingTransactionCounter::Decrement() {
  PJCORE_CHECK_GT(count_, 0);

  if (!--count_) {
    if (!core_->shell_) {
      for (ConnectionGroupMap::iterator it =
               core_->connection_group_map_.begin();
           it != core_->connection_group_map_.end(); ++it) {
        it->second->Close(true, Error());
      }
    }

    core_->ConsiderDestroy();
  }
}

void HttpClientCore::ConsiderDestroy() {
  if (shell_ || pending_resolve_counter_.count() ||
      pending_connection_counter_.count() ||
      pending_transaction_counter_.count()) {
    return;
  }

  Closure local_on_destroy;
  using std::swap;
  swap(local_on_destroy, on_destroy_);

  delete this;

  if (!local_on_destroy.is_null()) {
    local_on_destroy.Run();
  }
}

}  // namespace pjcore
