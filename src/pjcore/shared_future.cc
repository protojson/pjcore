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

#include "pjcore/shared_future.h"

#include "pjcore/third_party/chromium/bind.h"

#include "pjcore/logging.h"
#include "pjcore/live_uv.pb.h"
#include "pjcore/shared_uv_loop.h"

namespace pjcore {

RefCountedFuture::RefCountedFuture(const SharedUvLoop& shared_loop)
    : LiveCapturable("pjcore::RefCountedFuture", shared_loop->live_list()),
      shared_loop_(shared_loop),
      pending_count_(0) {
  PJCORE_CHECK(shared_loop_);
}

Closure RefCountedFuture::CreateClosure() {
  ++pending_count_;
  return Bind(&RefCountedFuture::OnClosure, this);
}

void RefCountedFuture::Wait() {
  PJCORE_CHECK_GE(pending_count_, 0);
  while (pending_count_ > 0) {
    int run_status = shared_loop_->uv()->run(shared_loop_->loop(), UV_RUN_ONCE);
    (void)run_status;
  }
}

scoped_ptr<google::protobuf::Message> RefCountedFuture::CaptureLive() const {
  scoped_ptr<LiveUvLoopFuture> live(new LiveUvLoopFuture());
  live->set_shared_loop_ptr(reinterpret_cast<uint64_t>(shared_loop_.get()));
  if (pending_count_) {
    live->set_pending_count(pending_count_);
  }
  return scoped_ptr<google::protobuf::Message>(live.release());
}

RefCountedFuture::~RefCountedFuture() { PJCORE_CHECK_EQ(0, pending_count_); }

void RefCountedFuture::OnClosure() {
  PJCORE_CHECK_GT(pending_count_, 0);
  --pending_count_;
}

}  // namespace pjcore
