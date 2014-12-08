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

#include "pjcore/http_client_transaction.h"

#include <string>

#include "pjcore/third_party/chromium/callback_helpers.h"
#include "pjcore/logging.h"
#include "pjcore/http_util.h"
#include "pjcore/json_util.h"
#include "pjcore/json_properties.h"
#include "pjcore/live_http.pb.h"
#include "pjcore/live_util.h"
#include "pjcore/name_value_util.h"
#include "pjcore/number_util.h"
#include "pjcore/url_util.h"
#include "pjcore/uv_util.h"

namespace pjcore {

HttpClientTransaction::HttpClientTransaction(
    LiveCapturableList* live_list, AbstractUv* uv,
    AbstractCounter* pending_transaction_counter,
    scoped_ptr<HttpRequest> request, const HttpResponseCallback& on_response)
    : AbstractHttpClientTransaction("pjcore::HttpClientTransaction", live_list),
      uv_(uv),
      pending_transaction_counter_unit_(pending_transaction_counter),
      request_(request.Pass()),
      on_response_(on_response) {
  PJCORE_CHECK(uv_);
  PJCORE_CHECK(request_);
  PJCORE_CHECK(on_response_);

  memset(&write_req_, 0, sizeof(write_req_));
  write_req_.data = this;
}

HttpClientTransaction::~HttpClientTransaction() { LogDestroy(); }

bool HttpClientTransaction::Init(Error* error) {
  PJCORE_CHECK(error);
  error->Clear();

  PJCORE_REQUIRE(request_->has_url(), "No url property in request");

  SetJsonProperty(error, "url", request_->url());

  PJCORE_REQUIRE(!request_->has_parsed_url(),
                 "Unexpected parsed_url property in request");

  PJCORE_REQUIRE_SILENT(
      ParseUrl(request_->url(), false, request_->mutable_parsed_url(), error),
      "Failed to parse URL");

  std::string error_str;

  PJCORE_REQUIRE(
      pjcore::GetNodeAndServiceName(request_->parsed_url(),
                                    &node_and_service_name_, &error_str),
      std::string("Cannot resolve URL: ") + error_str);

  return true;
}

bool HttpClientTransaction::PrepareWrite(Error* error) {
  PJCORE_CHECK(error);
  error->Clear();

  PJCORE_CHECK(request_header_.empty());
  PJCORE_CHECK(write_bufs_.empty());

  PJCORE_REQUIRE(request_->has_url(), "No URL");
  PJCORE_REQUIRE(request_->has_parsed_url(), "No parsed URL");

  std::string host_header_value = request_->parsed_url().host();
  if (request_->parsed_url().has_port()) {
    host_header_value.push_back(':');
    AppendNumber(request_->parsed_url().port(), &host_header_value);
  }

  InsertName(request_->mutable_headers(), std::string("Host"))
      .first->set_value(host_header_value);

  InsertName(request_->mutable_headers(), std::string("Content-Length"))
      .first->set_value(WriteNumber(request_->content().size()));

  std::string request_line_error;
  PJCORE_REQUIRE(
      CreateHttpRequestLineOut(
          request_->has_method() ? static_cast<http_method>(request_->method())
                                 : HTTP_GET,
          request_->url(), request_->http_version(), &request_header_,
          &request_line_error),
      std::string("Failed to create request line: ") + request_line_error);

  std::string append_headers_error;
  PJCORE_REQUIRE(
      AppendHttpHeaders(request_->headers(), &request_header_,
                        &append_headers_error),
      std::string("Failed to append HTTP headers: ") + append_headers_error);

  request_header_.append("\r\n", 2);

  write_bufs_.push_back(StringPieceToUvBuf(request_header_));
  write_bufs_.push_back(StringPieceToUvBuf(request_->content()));

  return true;
}

void HttpClientTransaction::AsyncWrite(
    uv_stream_t* handle,
    Callback<void(bool result, const Error& error)> on_write_complete) {
  PJCORE_CHECK(!on_write_complete.is_null());

  PJCORE_CHECK(on_write_complete_.is_null());
  on_write_complete_ = on_write_complete;

  int uv_errno = uv()->write(&write_req_, handle, write_bufs_.data(),
                             static_cast<unsigned int>(write_bufs_.size()),
                             &HttpClientTransaction::StaticOnWriteComplete);

  if (uv_errno < 0) {
    StaticOnWriteComplete(&write_req_, uv_errno);
  }
}

void HttpClientTransaction::ResultFailure(
    scoped_ptr<AbstractHttpClientTransaction> self, const Error& error) {
  PJCORE_CHECK(self);
  PJCORE_CHECK_EQ(this, self.get());

  PJCORE_CHECK(on_response_);
  on_response_.Release().Run(scoped_ptr<HttpResponse>(), error);
}

void HttpClientTransaction::ResultSuccess(
    scoped_ptr<AbstractHttpClientTransaction> self,
    scoped_ptr<HttpResponse> response) {
  PJCORE_CHECK(self);
  PJCORE_CHECK_EQ(this, self.get());

  PJCORE_CHECK(on_response_);
  on_response_.Release().Run(response.Pass(), Error());
}

scoped_ptr<google::protobuf::Message> HttpClientTransaction::CaptureLive()
    const {
  scoped_ptr<LiveHttpClientTransaction> live(new LiveHttpClientTransaction());
  live->set_uv_ptr(reinterpret_cast<int64_t>(uv_));
  if (request_) {
    CaptureLiveHttpRequestOut(request_.get(), live->mutable_request());
  }
  if (!NodeAndServiceNameEmpty(node_and_service_name_)) {
    live->mutable_node_and_service_name()->CopyFrom(node_and_service_name_);
  }
  if (on_response_) {
    CaptureLiveCallbackOut(on_response_.Get(), live->mutable_on_response());
  }
  return scoped_ptr<google::protobuf::Message>(live.Pass());
}

void HttpClientTransaction::StaticOnWriteComplete(uv_write_t* req, int status) {
  PJCORE_CHECK(req);

  HttpClientTransaction* transaction =
      static_cast<HttpClientTransaction*>(req->data);
  PJCORE_CHECK(transaction);

  Error error;

  ResetAndReturn(&transaction->on_write_complete_)
      .Run(transaction->OnWriteComplete(status, &error), error);
}

bool HttpClientTransaction::OnWriteComplete(int status, Error* error) {
  PJCORE_CHECK(error);
  error->Clear();

  write_bufs_.clear();
  request_header_.clear();

  PJCORE_UV_REQUIRE(status, "Write failed");
  return true;
}

}  // namespace pjcore
