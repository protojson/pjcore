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

#include "pjcore/shared_uv_loop.h"

#include "pjcore/third_party/uv/queue.h"

#include "pjcore/third_party/chromium/callback_helpers.h"
#include "pjcore/third_party/chromium/bind.h"

#include "pjcore/live_util.h"
#include "pjcore/logging.h"
#include "pjcore/uv_util.h"

namespace pjcore {

namespace {

class AsyncClosureContext : public LiveCapturable {
 public:
  AsyncClosureContext(const SharedUvLoop& shared_loop,
                      const Closure& thread_work, const Closure& on_work)
      : LiveCapturable("pjcore::<unnamed>::AsyncClosureContext",
                       shared_loop->live_list()),
        shared_loop_(shared_loop),
        thread_work_(thread_work),
        on_work_(on_work) {
    PJCORE_CHECK(shared_loop_);
    PJCORE_CHECK(!thread_work_.is_null());
    PJCORE_CHECK(!on_work_.is_null());

    memset(&request_, 0, sizeof(request_));
    request_.data = this;
  }

  void AsyncProcess() {
    int queue_work_status = shared_loop_->uv()->queue_work(
        shared_loop_->loop(), &request_, &StaticThreadWork, &StaticOnWork);

    PJCORE_CHECK_EQ(0, queue_work_status);
  }

 protected:
  scoped_ptr<google::protobuf::Message> CaptureLive() const {
    scoped_ptr<LiveUvLoopAsyncClosureContext> live(
        new LiveUvLoopAsyncClosureContext());
    live->set_shared_loop_ptr(reinterpret_cast<uint64_t>(shared_loop_.get()));
    if (!on_work_.is_null()) {
      CaptureLiveCallbackOut(on_work_, live->mutable_on_work());
    }
    live->set_request_ptr(reinterpret_cast<int64_t>(&request_));
    return scoped_ptr<google::protobuf::Message>(live.release());
  }

 private:
  ~AsyncClosureContext() {
    LogDestroy();
    PJCORE_CHECK(thread_work_.is_null());
    PJCORE_CHECK(on_work_.is_null());
  }

  static void StaticThreadWork(uv_work_t* request) {
    PJCORE_CHECK(request);
    AsyncClosureContext* context =
        static_cast<AsyncClosureContext*>(request->data);
    PJCORE_CHECK(context);
    ResetAndReturn(&context->thread_work_).Run();
  }

  static void StaticOnWork(uv_work_t* request, int status) {
    PJCORE_CHECK(request);
    AsyncClosureContext* context =
        static_cast<AsyncClosureContext*>(request->data);
    PJCORE_CHECK(context);
    ResetAndReturn(&context->on_work_).Run();
    delete context;
  }

  SharedUvLoop shared_loop_;

  Closure thread_work_;

  Closure on_work_;

  uv_work_t request_;
};

}  // unnamed namespace

RefCountedUvLoop::RefCountedUvLoop(LiveCapturableList* live_list,
                                   AbstractUv* uv, bool own_loop,
                                   uv_loop_t* loop)
    : LiveCapturable("pjcore::RefCountedUvLoop", live_list),
      uv_(uv),
      own_loop_(own_loop),
      loop_(loop) {
  PJCORE_CHECK(uv_);
  PJCORE_CHECK(loop_);
}

void RefCountedUvLoop::AsyncClosure(const Closure& thread_work,
                                    const Closure& on_work) {
  AsyncClosureContext* context =
      new AsyncClosureContext(this, thread_work, on_work);
  context->AsyncProcess();
}

scoped_refptr<RefCountedFuture> RefCountedUvLoop::CreateFuture() {
  return new RefCountedFuture(this);
}

scoped_ptr<google::protobuf::Message> RefCountedUvLoop::CaptureLive() const {
  scoped_ptr<LiveUvLoop> live(new LiveUvLoop());
  live->set_uv_ptr(reinterpret_cast<uint64_t>(uv_));
  if (own_loop_) {
    live->set_own_loop(own_loop_);
  }
  live->set_loop_ptr(reinterpret_cast<uint64_t>(loop_));
  if (loop_->data) {
    live->set_data_ptr(reinterpret_cast<int64_t>(loop_->data));
  }
  if (loop_->active_handles) {
    live->set_active_handles(loop_->active_handles);
  }
  if (loop_->stop_flag) {
    live->set_stop_flag(loop_->stop_flag);
  }
  if (loop_->handle_queue[0] && loop_->handle_queue[1]) {
    PJCORE_QUEUE* q;
    PJCORE_QUEUE_FOREACH(q, &loop_->handle_queue) {
      uv_handle_t* handle = PJCORE_QUEUE_DATA(q, uv_handle_t, handle_queue);
      PJCORE_CHECK(handle);
      CaptureLiveUvHandleOut(handle, live->add_handle_queue());
    }
  }
  if (loop_->active_reqs[0] && loop_->active_reqs[1]) {
    PJCORE_QUEUE* q;
    PJCORE_QUEUE_FOREACH(q, &loop_->active_reqs) {
      uv_req_t* req = PJCORE_QUEUE_DATA(q, uv_req_t, active_queue);
      CaptureLiveUvReqOut(req, live->add_active_reqs());
    }
  }
  return scoped_ptr<google::protobuf::Message>(live.release());
}

RefCountedUvLoop::~RefCountedUvLoop() {
  LogDestroy();

  if (own_loop_) {
    uv()->loop_delete(loop_);
    own_loop_ = false;
  }

  loop_ = NULL;
}

SharedUvLoop CreateUvLoop(LiveCapturableList* live_list, AbstractUv* uv) {
  return new RefCountedUvLoop(live_list, uv, true, uv->loop_new());
}

SharedUvLoop GetDefaultUvLoop(LiveCapturableList* live_list, AbstractUv* uv) {
  return new RefCountedUvLoop(live_list, uv, false, uv->default_loop());
}

}  // namespace pjcore
