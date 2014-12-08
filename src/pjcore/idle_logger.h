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

#ifndef PJCORE_IDLE_LOGGER_H_
#define PJCORE_IDLE_LOGGER_H_

#include "pjcore/shared_uv_loop.h"

typedef struct uv_idle_s uv_idle_t;

namespace pjcore {

class IdleLogger : public LiveCapturable {
 public:
  IdleLogger(LiveCapturableList* live_list, const SharedUvLoop& shared_loop);

  ~IdleLogger();

  bool get_log_once() const { return log_once_; }

  void set_log_once() { log_once_ = true; }

  void reset_log_once() { log_once_ = false; }

  bool get_log_always() const { return log_always_; }

  void set_log_always() { log_always_ = true; }

  void reset_log_always() { log_always_ = false; }

 protected:
  scoped_ptr<google::protobuf::Message> CaptureLive() const OVERRIDE;

 private:
  static void StaticIdle(uv_idle_t* handle);

  void Idle();

  SharedUvLoop shared_loop_;

  bool log_once_;

  bool log_always_;

  scoped_ptr<uv_idle_t> idle_handle_;
};

}  // namespace pjcore

#endif  // PJCORE_IDLE_LOGGER_H_
