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

#include "pjcore/abstract_uv.h"

#include "pjcore/idle_logger.h"

#include "pjcore/uv_util.h"
#include "pjcore/shared_uv_loop-inl.h"

namespace pjcore {

IdleLogger::IdleLogger(LiveCapturableList* live_list,
                       const SharedUvLoop& shared_loop)
    : LiveCapturable("pjcore::IdleLogger", live_list),
      shared_loop_(shared_loop),
      log_once_(false),
      log_always_(false) {
  PJCORE_CHECK(shared_loop_);

  idle_handle_.reset(new uv_idle_t());
  memset(idle_handle_.get(), 0, sizeof(*idle_handle_.get()));
  idle_handle_->data = this;

  int idle_init_status =
      shared_loop_->uv()->idle_init(shared_loop->loop(), idle_handle_.get());
  PJCORE_CHECK_EQ(0, idle_init_status);  // idle_init_status

  int idle_start_status = shared_loop_->uv()->idle_start(
      idle_handle_.get(), &IdleLogger::StaticIdle);

  PJCORE_CHECK_EQ(0, idle_start_status);  // idle_start_status
}

scoped_ptr<google::protobuf::Message> IdleLogger::CaptureLive() const {
  scoped_ptr<LiveIdleLogger> live(new LiveIdleLogger());
  live->set_shared_loop_ptr(reinterpret_cast<uint64_t>(shared_loop_.get()));
  if (log_once_) {
    live->set_log_once(log_once_);
  }
  if (log_always_) {
    live->set_log_always(log_always_);
  }
  if (idle_handle_) {
    live->set_idle_handle_ptr(reinterpret_cast<int64_t>(idle_handle_.get()));
  }
  return scoped_ptr<google::protobuf::Message>(live.release());
}

void IdleLogger::StaticIdle(uv_idle_t* handle) {
  PJCORE_CHECK(handle);

  IdleLogger* idle_logger = static_cast<IdleLogger*>(handle->data);
  PJCORE_CHECK(idle_logger);

  idle_logger->Idle();
}

void IdleLogger::Idle() {
  if (get_log_once()) {
    PJCORE_LOG_ALL_LIVE_JSON;
    reset_log_once();
  } else if (get_log_always()) {
    PJCORE_LOG_ALL_LIVE_JSON;
  }
}

IdleLogger::~IdleLogger() {
  LogDestroy();

  int idle_stop_status = shared_loop_->uv()->idle_stop(idle_handle_.get());
  PJCORE_CHECK_EQ(0, idle_stop_status);  // idle_stop_status

  CloseAndDeleteUvHandle(shared_loop_->uv(), idle_handle_.Pass());
}

}  // namespace pjcore
