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

#include <algorithm>

#include "pjcore/http_client.h"
#include "pjcore/http_client_core.h"
#include "pjcore/live_http.pb.h"
#include "pjcore/live_util.h"
#include "pjcore/logging.h"

namespace pjcore {

SharedHttpHandler HttpClient::Create(const HttpClientConfig& config,
                                     const Closure& on_destroy, Error* error) {
  return Create(NULL, config, GetDefaultUvLoop(NULL), on_destroy, error);
}

SharedHttpHandler HttpClient::Create(LiveCapturableList* live_list,
                                     const HttpClientConfig& config,
                                     const SharedUvLoop& shared_loop,
                                     const Closure& on_destroy, Error* error) {
  return Create(live_list, config, shared_loop, on_destroy,
                GetDefaultHttpClientTransactionFactory(), error);
}

SharedHttpHandler HttpClient::Create(
    LiveCapturableList* live_list, const HttpClientConfig& config,
    const SharedUvLoop& shared_loop, const Closure& on_destroy,
    AbstractHttpClientTransactionFactory* transaction_factory, Error* error) {
  PJCORE_CHECK(shared_loop);
  PJCORE_CHECK(!on_destroy.is_null());
  PJCORE_CHECK(error);
  error->Clear();

  SharedHttpHandler client(new HttpClient(live_list));

  PJCORE_NULL_REQUIRE_SILENT(
      static_cast<HttpClient*>(client.get())
          ->core_->Init(config, shared_loop, transaction_factory, error),
      "Failed to initialize HTTP client");

  static_cast<HttpClient*>(client.get())->on_destroy_ = on_destroy;

  return client;
}

void HttpClient::AsyncHandle(scoped_ptr<HttpRequest> request,
                             const HttpResponseCallback& on_response) {
  PJCORE_CHECK(request);
  PJCORE_CHECK(!on_response.is_null());

  core_->AsyncProcess(request.Pass(), on_response);
}

scoped_ptr<google::protobuf::Message> HttpClient::CaptureLive() const {
  scoped_ptr<LiveHttpClient> live(new LiveHttpClient());
  live->set_core_ptr(reinterpret_cast<uint64_t>(core_));
  if (!on_destroy_.is_null()) {
    CaptureLiveCallbackOut(on_destroy_, live->mutable_on_destroy());
  }
  return scoped_ptr<google::protobuf::Message>(live.release());
}

HttpClient::HttpClient(LiveCapturableList* live_list)
    : AbstractHttpHandler("pjcore::HttpClient", live_list),
      core_(new HttpClientCore(live_list, this)) {
  core_->set_live_parent(this);
}

HttpClient::~HttpClient() {
  LogDestroy();

  HttpClientCore* local_core = NULL;
  Closure local_on_destroy;

  using std::swap;
  swap(local_core, core_);
  swap(local_on_destroy, on_destroy_);

  local_core->OnShellDestroyed(this, local_on_destroy);
}

}  // namespace pjcore
