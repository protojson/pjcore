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

#include "pjcore/http_server_transaction.h"

#include <utility>

#include "pjcore/third_party/chromium/callback_helpers.h"
#include "pjcore/logging.h"
#include "pjcore/http_util.h"
#include "pjcore/json_util.h"
#include "pjcore/live_http.pb.h"
#include "pjcore/live_util.h"
#include "pjcore/name_value_util.h"
#include "pjcore/number_util.h"
#include "pjcore/repeated_field_util.h"
#include "pjcore/uv_util.h"

namespace pjcore {

HttpServerTransaction::HttpServerTransaction(
    LiveCapturableList* live_list, AbstractUv* uv,
    AbstractCounter* pending_transaction_counter,
    scoped_ptr<HttpRequest> request)
    : LiveCapturable("pjcore::HttpServerTransaction", live_list),
      uv_(uv),
      pending_transaction_counter_unit_(pending_transaction_counter),
      request_(request.Pass()) {
  PJCORE_CHECK(uv_);
  PJCORE_CHECK(request_);
  memset(&write_req_, 0, sizeof(write_req_));
  write_req_.data = this;
}

HttpServerTransaction::~HttpServerTransaction() { LogDestroy(); }

bool HttpServerTransaction::PrepareWrite(Error* error) {
  PJCORE_CHECK(error);
  error->Clear();

  PJCORE_CHECK(response_header_.empty());
  PJCORE_CHECK(write_bufs_.empty());

  InsertName(response_->mutable_headers(), std::string("Content-Length"))
      .first->set_value(WriteNumber(response_->content().size()));

  std::pair<HttpHeader*, bool> insert_date =
      InsertName(response_->mutable_headers(), std::string("Date"));

  if (insert_date.second) {
    insert_date.first->set_value(GetCurrentHttpDate());
  }

  response_header_ = CreateHttpStatusLine(response_->http_version(),
                                          response_->has_status_code()
                                              ? response_->status_code()
                                              : HTTP_STATUS_CODE_OK);

  std::string append_headers_error;
  PJCORE_REQUIRE(
      AppendHttpHeaders(response_->headers(), &response_header_,
                        &append_headers_error),
      std::string("Failed to append HTTP headers: ") + append_headers_error);

  response_header_.append("\r\n", 2);

  write_bufs_.push_back(StringPieceToUvBuf(response_header_));
  write_bufs_.push_back(StringPieceToUvBuf(response_->content()));

  return true;
}

void HttpServerTransaction::AsyncWrite(
    uv_stream_t* handle,
    Callback<void(bool result, const Error& error)> on_write_complete) {
  PJCORE_CHECK(!on_write_complete.is_null());

  PJCORE_CHECK(on_write_complete_.is_null());
  on_write_complete_ = on_write_complete;

  int uv_errno = uv()->write(&write_req_, handle, write_bufs_.data(),
                             static_cast<unsigned int>(write_bufs_.size()),
                             &HttpServerTransaction::StaticOnWriteComplete);

  if (uv_errno < 0) {
    StaticOnWriteComplete(&write_req_, uv_errno);
  }
}

scoped_ptr<google::protobuf::Message> HttpServerTransaction::CaptureLive()
    const {
  scoped_ptr<LiveHttpServerTransaction> live(new LiveHttpServerTransaction());
  live->set_uv_ptr(reinterpret_cast<int64_t>(uv_));
  if (request_) {
    CaptureLiveHttpRequestOut(request_.get(), live->mutable_request());
  }
  if (response_) {
    CaptureLiveHttpResponseOut(response_.get(), live->mutable_response());
  }
  if (!on_write_complete_.is_null()) {
    CaptureLiveCallbackOut(on_write_complete_,
                           live->mutable_on_write_complete());
  }
  if (!response_header_.empty()) {
    live->set_response_header(response_header_);
  }
  if (!write_bufs_.empty()) {
    live->set_write_bufs_ptr(reinterpret_cast<int64_t>(write_bufs_.data()));
    for (std::vector<uv_buf_t>::const_iterator buf_it = write_bufs_.begin();
         buf_it != write_bufs_.end(); ++buf_it) {
      CaptureLiveUvBufOut(*buf_it, live->add_write_bufs());
    }
  }
  live->set_write_req_ptr(reinterpret_cast<int64_t>(&write_req_));
  return scoped_ptr<google::protobuf::Message>(live.Pass());
}

void HttpServerTransaction::StaticOnWriteComplete(uv_write_t* req, int status) {
  PJCORE_CHECK(req);

  HttpServerTransaction* transaction =
      static_cast<HttpServerTransaction*>(req->data);
  PJCORE_CHECK(transaction);

  Error error;

  ResetAndReturn(&transaction->on_write_complete_)
      .Run(transaction->OnWriteComplete(status, &error), error);
}

bool HttpServerTransaction::OnWriteComplete(int status, Error* error) {
  PJCORE_CHECK(error);
  error->Clear();

  write_bufs_.clear();
  response_header_.clear();

  PJCORE_UV_REQUIRE(status, "Write failed");
  return true;
}

}  // namespace pjcore
