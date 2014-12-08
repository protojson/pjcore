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

#include "pjcore/http_client_connection.h"

#include <algorithm>

#include "pjcore/third_party/chromium/bind.h"
#include "pjcore/third_party/chromium/callback_helpers.h"

#include "pjcore/auto_callback.h"
#include "pjcore/error_util.h"
#include "pjcore/logging.h"
#include "pjcore/http_util.h"
#include "pjcore/live_util.h"

namespace pjcore {

HttpClientConnection::HttpClientConnection(
    LiveCapturableList* live_list, const SharedUvLoop& shared_loop,
    AbstractCounter* pending_connection_counter,
    const SharedAddrInfoList& addr_infos)
    : LiveCapturable("pjcore::HttpClientConnection", live_list),
      shared_loop_(shared_loop),
      pending_connection_counter_unit_(pending_connection_counter),
      addr_infos_(addr_infos),
      open_(false),
      reading_(false),
      closing_(false),
      buffer_size_(0) {
  memset(&connect_handle_, 0, sizeof(connect_handle_));
  connect_handle_.data = this;

  memset(&connect_req_, 0, sizeof(connect_req_));
  connect_req_.data = this;
}

HttpClientConnection::~HttpClientConnection() {
  LogDestroy();

  PJCORE_CHECK(!open_);
  PJCORE_CHECK(!reading_);

  PJCORE_CHECK(wait_queue_.empty());
  PJCORE_CHECK(write_queue_.empty());
  PJCORE_CHECK(read_queue_.empty());

  PJCORE_CHECK(on_closing_.is_null());
  PJCORE_CHECK(on_close_.is_null());
}

bool HttpClientConnection::InitSync(AbstractHttpParserFactory* parser_factory,
                                    Error* error) {
  PJCORE_CHECK(error);
  error->Clear();

  PJCORE_CHECK(!parser_);
  parser_ = parser_factory->CreateParser(live_list(), HTTP_PARSER_RESPONSE);
  PJCORE_CHECK(parser_);
  parser_->set_live_parent(this);

  PJCORE_CHECK(!open_);

  PJCORE_REQUIRE(addr_infos_, "No address list");
  PJCORE_REQUIRE(addr_infos_->list(), "Empty address list");

  PJCORE_UV_REQUIRE(uv()->tcp_init(shared_loop()->loop(), &connect_handle_),
                    "Failed to initialize TCP connection");

  open_ = true;

  return true;
}

void HttpClientConnection::InitAsync(
    std::list<AbstractHttpClientTransaction*>* transactions,
    const Callback<void(bool result, const Error& error)>& on_closing,
    const Closure& on_close) {
  PJCORE_CHECK(transactions);
  PJCORE_CHECK(!on_closing.is_null());
  PJCORE_CHECK(!on_close.is_null());

  PJCORE_CHECK(on_closing_.is_null());
  PJCORE_CHECK(on_close_.is_null());

  PJCORE_CHECK(open_);

  for (std::list<AbstractHttpClientTransaction*>::iterator it =
           transactions->begin();
       it != transactions->end(); ++it) {
    (*it)->set_live_parent(this);
  }

  wait_queue_.splice(wait_queue_.end(), *transactions);

  on_closing_ = on_closing;
  on_close_ = on_close;

  int connect_status =
      uv()->tcp_connect(&connect_req_, &connect_handle_,
                        addr_infos_->list()->ai_addr, &StaticOnConnect);

  if (connect_status < 0) {
    StaticOnConnect(&connect_req_, connect_status);
  }
}

void HttpClientConnection::Close(bool result, const Error& error) {
  if (closing_) {
    return;
  }

  PJCORE_CHECK(open_);

  PJCORE_CHECK(!on_closing_.is_null());
  PJCORE_CHECK(!on_close_.is_null());

  closing_ = true;

  Error default_closing_error;
  default_closing_error.set_description("Connection is closing");

  while (!read_queue_.empty()) {
    scoped_ptr<AbstractHttpClientTransaction> transaction(read_queue_.front());
    read_queue_.pop_front();
    transaction->ResultFailure(transaction.Pass(),
                               result ? default_closing_error : error);
  }

  while (!wait_queue_.empty()) {
    scoped_ptr<AbstractHttpClientTransaction> transaction(wait_queue_.front());
    wait_queue_.pop_front();
    transaction->ResultFailure(transaction.Pass(),
                               result ? default_closing_error : error);
  }

  ResetAndReturn(&on_closing_).Run(result, error);

  if (reading_) {
    int read_stop_status =
        uv()->read_stop(reinterpret_cast<uv_stream_t*>(&connect_handle_));
    PJCORE_CHECK_EQ(0, read_stop_status);
    reading_ = false;
  }

  uv()->close(reinterpret_cast<uv_handle_t*>(&connect_handle_), &StaticOnClose);
}

void HttpClientConnection::AsyncProcess(
    scoped_ptr<AbstractHttpClientTransaction> transaction) {
  PJCORE_CHECK(transaction);

  if (closing_) {
    Error default_closing_error;
    default_closing_error.set_description("Connection is closing");
    transaction->ResultFailure(transaction.Pass(), default_closing_error);
    return;
  }

  if (!reading_) {
    wait_queue_.push_back(transaction.release());
  } else {
    Error error;
    if (!transaction->PrepareWrite(&error)) {
      transaction->ResultFailure(transaction.Pass(), error);
    } else {
      AbstractHttpClientTransaction* local_transaction = transaction.release();
      write_queue_.push_back(local_transaction);
      local_transaction->AsyncWrite(
          reinterpret_cast<uv_stream_t*>(&connect_handle_),
          Bind(&HttpClientConnection::OnTransactionWriteComplete,
               Unretained(this), local_transaction));
    }
  }
}

scoped_ptr<google::protobuf::Message> HttpClientConnection::CaptureLive()
    const {
  scoped_ptr<LiveHttpClientConnection> live(new LiveHttpClientConnection());
  live->set_shared_loop_ptr(reinterpret_cast<uint64_t>(shared_loop_.get()));
  addr_infos_->CaptureLiveOut(live->mutable_addr_infos());
  if (open_) {
    live->set_open(open_);
  }
  CaptureLiveListPtrOut(wait_queue_, live->mutable_wait_queue_ptr());
  CaptureLiveListPtrOut(write_queue_, live->mutable_write_queue_ptr());
  CaptureLiveListPtrOut(read_queue_, live->mutable_read_queue_ptr());
  if (!on_closing_.is_null()) {
    CaptureLiveCallbackOut(on_closing_, live->mutable_on_closing());
  }
  if (!on_close_.is_null()) {
    CaptureLiveCallbackOut(on_close_, live->mutable_on_close());
  }
  live->set_connect_handle_ptr(reinterpret_cast<int64_t>(&connect_handle_));
  live->set_connect_req_ptr(reinterpret_cast<int64_t>(&connect_req_));
  if (reading_) {
    live->set_reading(closing_);
  }
  if (closing_) {
    live->set_closing(closing_);
  }
  live->set_parser_ptr(reinterpret_cast<int64_t>(parser_.get()));
  if (buffer_size_) {
    live->set_buffer_size(buffer_size_);
  }
  if (buffer_.get()) {
    live->set_buffer_ptr(reinterpret_cast<uint64_t>(buffer_.get()));
  }
  return scoped_ptr<google::protobuf::Message>(live.release());
}

void HttpClientConnection::StaticOnConnect(uv_connect_t* req, int status) {
  PJCORE_CHECK(req);

  HttpClientConnection* connection =
      static_cast<HttpClientConnection*>(req->data);
  PJCORE_CHECK(connection);

  PJCORE_VOID_REQUIRE_SILENT(connection->OnConnect(status),
                             "Failed to connect");
}

bool HttpClientConnection::OnConnect(int status) {
  PJCORE_CHECK(!on_closing_.is_null());
  PJCORE_CHECK(!on_close_.is_null());

  if (status < 0) {
    Error error;
    error.set_uv_errno(status);
    error.set_description("Failed to connect");
    Close(false, error);
    PJCORE_FAIL_SILENT(error.description());
  }

  reading_ = true;
  int read_start_status =
      uv()->read_start(reinterpret_cast<uv_stream_t*>(&connect_handle_),
                       &StaticAlloc, &StaticOnRead);

  if (read_start_status < 0) {
    reading_ = false;
    Error error;
    error.set_uv_errno(read_start_status);
    error.set_description("Failed to start reading");
    Close(false, error);
    PJCORE_FAIL_SILENT(error.description());
  }

  while (!wait_queue_.empty()) {
    scoped_ptr<AbstractHttpClientTransaction> transaction(wait_queue_.front());
    wait_queue_.pop_front();

    Error error;
    if (!transaction->PrepareWrite(&error)) {
      transaction->ResultFailure(transaction.Pass(), error);
    } else {
      AbstractHttpClientTransaction* local_transaction = transaction.release();
      write_queue_.push_back(local_transaction);
      local_transaction->AsyncWrite(
          reinterpret_cast<uv_stream_t*>(&connect_handle_),
          Bind(&HttpClientConnection::OnTransactionWriteComplete,
               Unretained(this), local_transaction));
    }
  }

  return true;
}

void HttpClientConnection::StaticAlloc(uv_handle_t* handle,
                                       size_t suggested_size, uv_buf_t* buf) {
  PJCORE_CHECK(handle);

  HttpClientConnection* connection =
      static_cast<HttpClientConnection*>(handle->data);
  PJCORE_CHECK(connection);

  Error error;
  if (!connection->Alloc(suggested_size, buf, &error)) {
    PJCORE_LOG_ERROR("Alloc callback has failed", &error);
  }
}

bool HttpClientConnection::Alloc(size_t suggested_size, uv_buf_t* buf,
                                 Error* error) {
  PJCORE_CHECK(error);
  error->Clear();

  PJCORE_CHECK(buf);
  buf->len = 0;
  buf->base = NULL;

  PJCORE_CHECK(!buffer_.get());

  buffer_.reset(new char[suggested_size]);
  buffer_size_ = suggested_size;

  buf->base = buffer_.get();
  buf->len = buffer_size_;

  return true;
}

void HttpClientConnection::StaticOnRead(uv_stream_t* stream, ssize_t nread,
                                        const uv_buf_t* buf) {
  PJCORE_CHECK(stream);

  HttpClientConnection* connection =
      static_cast<HttpClientConnection*>(stream->data);
  PJCORE_CHECK(connection);

  if (nread == UV_EOF) {
    connection->Close(true, Error());
    return;
  }

  Error error;
  if (!connection->OnRead(nread, buf, &error)) {
    PJCORE_LOG_ERROR("Failed to read request, will close the connection",
                     &error);
    connection->Close(false, error);
  }
}

bool HttpClientConnection::OnRead(ssize_t nread, const uv_buf_t* buf,
                                  Error* error) {
  PJCORE_CHECK(error);
  error->Clear();

  size_t local_buffer_size = buffer_size_;
  buffer_size_ = 0;
  scoped_ptr<char[]> local_buffer(buffer_.release());

  PJCORE_UV_REQUIRE(nread, "Failed to read");
  PJCORE_CHECK_LE(static_cast<size_t>(nread), local_buffer_size);
  PJCORE_REQUIRE(nread != 0, "Read zero bytes");

  StringPiece remaining(buf->base, nread);

  while (!remaining.empty()) {
    size_t offset = 0;
    PJCORE_REQUIRE_SILENT(parser_->Read(remaining, &offset, error),
                          "Failed to read next response");

    PJCORE_CHECK_LE(offset, remaining.size());
    remaining.remove_prefix(offset);
  }

  scoped_ptr<HttpResponse> response;
  while ((response = parser_->NextResponse())) {
    PJCORE_REQUIRE(!read_queue_.empty(), "Unexpected response");

    scoped_ptr<AbstractHttpClientTransaction> transaction(read_queue_.front());
    read_queue_.pop_front();

    transaction->ResultSuccess(transaction.Pass(), response.Pass());
  }

  return true;
}

void HttpClientConnection::OnTransactionWriteComplete(
    AbstractHttpClientTransaction* transaction, bool result,
    const Error& error) {
  PJCORE_CHECK(transaction);

  std::list<AbstractHttpClientTransaction*>::iterator it =
      std::find(write_queue_.begin(), write_queue_.end(), transaction);

  PJCORE_CHECK(it != write_queue_.end());
  write_queue_.erase(it);
  scoped_ptr<AbstractHttpClientTransaction> local_transaction(transaction);

  if (closing_) {
    Error default_closing_error;
    default_closing_error.set_description("Connection is closing");

    local_transaction->ResultFailure(local_transaction.Pass(),
                                     default_closing_error);

    if (!open_ && write_queue_.empty()) {
      ResetAndReturn(&on_close_).Run();
      return;
    }

    return;
  }

  if (!result) {
    local_transaction->ResultFailure(local_transaction.Pass(), error);

    Close(false, error);
    return;
  }

  read_queue_.push_back(local_transaction.release());
}

void HttpClientConnection::StaticOnClose(uv_handle_t* handle) {
  PJCORE_CHECK(handle);

  HttpClientConnection* connection =
      static_cast<HttpClientConnection*>(handle->data);
  PJCORE_CHECK(connection);

  PJCORE_CHECK(connection->open_);
  connection->open_ = false;

  if (connection->write_queue_.empty()) {
    ResetAndReturn(&connection->on_close_).Run();
  }
}

}  // namespace pjcore
