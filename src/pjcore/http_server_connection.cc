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

#include "pjcore/http_server_connection.h"

#include "pjcore/third_party/chromium/bind.h"

#include "pjcore/error_util.h"
#include "pjcore/third_party/chromium/callback_helpers.h"
#include "pjcore/logging.h"
#include "pjcore/http_util.h"
#include "pjcore/live_util.h"

namespace pjcore {

HttpServerConnection::HttpServerConnection(
    LiveCapturableList* live_list, const SharedUvLoop& shared_loop,
    AbstractCounter* pending_connection_counter,
    AbstractCounter* pending_transaction_counter, uv_tcp_t* listen_handle,
    const SharedHttpHandler& handler)
    : AbstractHttpServerConnection("pjcore::HttpServerConnection", live_list),
      shared_loop_(shared_loop),
      pending_connection_counter_unit_(pending_connection_counter),
      pending_transaction_counter_splitter_(
          pending_transaction_counter, &local_pending_transaction_counter_),
      listen_handle_(listen_handle),
      handler_(handler),
      open_(false),
      reading_(false),
      closing_(false),
      buffer_size_(0) {
  PJCORE_CHECK(shared_loop_);
  PJCORE_CHECK(handler_);

  memset(&accept_handle_, 0, sizeof(accept_handle_));
  accept_handle_.data = this;
}

HttpServerConnection::~HttpServerConnection() {
  LogDestroy();

  PJCORE_CHECK(!open_);
  PJCORE_CHECK(!reading_);

  PJCORE_CHECK(handle_queue_.empty());
  PJCORE_CHECK(write_queue_.empty());

  PJCORE_CHECK(on_closing_.is_null());
  PJCORE_CHECK(on_close_.is_null());
}

bool HttpServerConnection::InitSync(AbstractHttpParserFactory* parser_factory,
                                    Error* error) {
  PJCORE_CHECK(error);
  error->Clear();

  PJCORE_CHECK(!parser_);
  parser_ = parser_factory->CreateParser(live_list(), HTTP_PARSER_REQUEST);
  PJCORE_CHECK(parser_);
  parser_->set_live_parent(this);

  PJCORE_CHECK(!open_);
  PJCORE_CHECK(!reading_);

  PJCORE_UV_REQUIRE(uv()->tcp_init(shared_loop()->loop(), &accept_handle_),
                    "Failed to initialize TCP connection");

  open_ = true;

  return true;
}

void HttpServerConnection::InitAsync(
    const Callback<void(bool result, const Error& error)>& on_closing,
    const Closure& on_close) {
  PJCORE_CHECK(!on_closing.is_null());
  PJCORE_CHECK(!on_close.is_null());

  PJCORE_CHECK(on_closing_.is_null());
  PJCORE_CHECK(on_close_.is_null());

  PJCORE_CHECK(open_);

  on_closing_ = on_closing;
  on_close_ = on_close;

  int accept_status =
      uv()->accept(reinterpret_cast<uv_stream_t*>(listen_handle_),
                   reinterpret_cast<uv_stream_t*>(&accept_handle_));

  if (accept_status < 0) {
    Error error;
    error.set_uv_errno(accept_status);
    error.set_description("Failed to accept");
    Close(false, error);
    PJCORE_VOID_FAIL_SILENT(error.description());
  }

  reading_ = true;
  int read_start_status =
      uv()->read_start(reinterpret_cast<uv_stream_t*>(&accept_handle_),
                       &StaticAlloc, &StaticOnRead);

  if (read_start_status < 0) {
    reading_ = false;
    Error error;
    error.set_uv_errno(read_start_status);
    error.set_description("Failed to start reading");
    Close(false, error);
    PJCORE_VOID_FAIL_SILENT(error.description());
  }
}

scoped_ptr<google::protobuf::Message> HttpServerConnection::CaptureLive()
    const {
  scoped_ptr<LiveHttpServerConnection> live(new LiveHttpServerConnection());
  live->set_shared_loop_ptr(reinterpret_cast<uint64_t>(shared_loop_.get()));
  if (local_pending_transaction_counter_.count()) {
    live->set_pending_transaction_count(
        local_pending_transaction_counter_.count());
  }
  if (listen_handle_) {
    live->set_listen_handle_ptr(reinterpret_cast<uint64_t>(listen_handle_));
  }
  if (handler_) {
    live->set_handler_ptr(reinterpret_cast<int64_t>(handler_.get()));
  }
  if (open_) {
    live->set_open(open_);
  }
  live->set_accept_handle_ptr(reinterpret_cast<uint64_t>(&accept_handle_));
  CaptureLiveListPtrOut(handle_queue_, live->mutable_handle_queue_ptr());
  CaptureLiveListPtrOut(write_queue_, live->mutable_write_queue_ptr());
  if (!on_closing_.is_null()) {
    CaptureLiveCallbackOut(on_closing_, live->mutable_on_closing());
  }
  if (!on_close_.is_null()) {
    CaptureLiveCallbackOut(on_close_, live->mutable_on_close());
  }
  if (reading_) {
    live->set_reading(reading_);
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

void HttpServerConnection::StaticAlloc(uv_handle_t* handle,
                                       size_t suggested_size, uv_buf_t* buf) {
  PJCORE_CHECK(handle);

  HttpServerConnection* connection =
      static_cast<HttpServerConnection*>(handle->data);
  PJCORE_CHECK(connection);

  Error error;
  if (!connection->Alloc(suggested_size, buf, &error)) {
    PJCORE_LOG_ERROR("Alloc callback has failed", &error);
  }
}

bool HttpServerConnection::Alloc(size_t suggested_size, uv_buf_t* buf,
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

void HttpServerConnection::StaticOnRead(uv_stream_t* stream, ssize_t nread,
                                        const uv_buf_t* buf) {
  PJCORE_CHECK(stream);

  HttpServerConnection* connection =
      static_cast<HttpServerConnection*>(stream->data);
  PJCORE_CHECK(connection);

  if (nread == UV_EOF) {
    connection->Close(true, Error());
    return;
  }

  Error error;
  if (!connection->OnRead(nread, buf, &error)) {
    connection->Close(false, error);
    return;
  }
}

bool HttpServerConnection::OnRead(ssize_t nread, const uv_buf_t* buf,
                                  Error* error) {
  PJCORE_CHECK(error);
  error->Clear();

  PJCORE_CHECK(reading_);

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
                          "Failed to read next request");

    PJCORE_CHECK_LE(offset, remaining.size());
    remaining.remove_prefix(offset);
  }

  scoped_ptr<HttpRequest> request;
  while ((request = parser_->NextRequest())) {
    scoped_ptr<HttpServerTransaction> transaction(new HttpServerTransaction(
        live_list(), uv(), &pending_transaction_counter_splitter_,
        request.Pass()));
    transaction->set_live_parent(this);

    if (!closing_) {
      handle_queue_.push_back(transaction.release());
      handler_->AsyncHandle(
          handle_queue_.back()->request().Pass(),
          Bind(&HttpServerConnection::OnTransactionHandleComplete,
               Unretained(this), Unretained(handle_queue_.back())));
    }
  }

  return true;
}

void HttpServerConnection::OnTransactionHandleComplete(
    HttpServerTransaction* transaction, scoped_ptr<HttpResponse> response,
    const Error& error) {
  PJCORE_CHECK(transaction);

  std::list<HttpServerTransaction*>::iterator transaction_it =
      std::find(handle_queue_.begin(), handle_queue_.end(), transaction);

  PJCORE_CHECK(transaction_it != handle_queue_.end());

  if (!response) {
    response.reset(new HttpResponse());
    *response->mutable_http_version() = DefaultHttpVersion();
    response->set_status_code(HTTP_STATUS_CODE_INTERNAL_SERVER_ERROR);
    response->set_content(ErrorToString(error, false));
  }

  PJCORE_CHECK(!transaction->response());
  transaction->response().swap(response);

  while (!handle_queue_.empty() && handle_queue_.front()->response()) {
    scoped_ptr<HttpServerTransaction> write_transaction(handle_queue_.front());
    handle_queue_.pop_front();

    if (closing_) {
      if (!open_ && handle_queue_.empty() && write_queue_.empty()) {
        ResetAndReturn(&on_close_).Run();
        return;
      }
    } else {
      Error error;
      if (!write_transaction->PrepareWrite(&error)) {
        Close(false, error);
        return;
      }

      write_queue_.push_back(write_transaction.release());

      write_queue_.back()->AsyncWrite(
          reinterpret_cast<uv_stream_t*>(&accept_handle_),
          Bind(&HttpServerConnection::OnTransactionWriteComplete,
               Unretained(this), write_queue_.back()));
    }
  }
}

void HttpServerConnection::OnTransactionWriteComplete(
    HttpServerTransaction* transaction, bool result, const Error& error) {
  std::list<HttpServerTransaction*>::iterator transaction_it =
      std::find(write_queue_.begin(), write_queue_.end(), transaction);

  PJCORE_CHECK(transaction_it != write_queue_.end());

  write_queue_.erase(transaction_it);
  scoped_ptr<HttpServerTransaction> local_transaction(transaction);

  if (closing_) {
    if (!open_ && handle_queue_.empty() && write_queue_.empty()) {
      ResetAndReturn(&on_close_).Run();
      return;
    }
  } else {
    if (!result) {
      Close(result, error);
    }
  }
}

void HttpServerConnection::Close(bool result, const Error& error) {
  if (closing_) {
    return;
  }

  PJCORE_CHECK(open_);

  PJCORE_CHECK(!on_closing_.is_null());
  PJCORE_CHECK(!on_close_.is_null());

  closing_ = true;

  ResetAndReturn(&on_closing_).Run(result, error);

  if (reading_) {
    int read_stop_status =
        uv()->read_stop(reinterpret_cast<uv_stream_t*>(&accept_handle_));
    PJCORE_CHECK_EQ(0, read_stop_status);
    reading_ = false;
  }

  uv()->close(reinterpret_cast<uv_handle_t*>(&accept_handle_), &StaticOnClose);
}

void HttpServerConnection::StaticOnClose(uv_handle_t* handle) {
  PJCORE_CHECK(handle);

  HttpServerConnection* connection =
      static_cast<HttpServerConnection*>(handle->data);
  PJCORE_CHECK(connection);

  PJCORE_CHECK(connection->open_);
  connection->open_ = false;

  if (connection->handle_queue_.empty() && connection->write_queue_.empty()) {
    ResetAndReturn(&connection->on_close_).Run();
  }
}

}  // namespace pjcore
