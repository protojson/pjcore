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

#ifndef PJCORE_SHARED_UV_LOOP_INL_H_
#define PJCORE_SHARED_UV_LOOP_INL_H_

#include "pjcore/shared_uv_loop.h"

#include <uv.h>

#include "pjcore/logging.h"
#include "pjcore/live_uv.pb.h"
#include "pjcore/shared_future-inl.h"

namespace pjcore {

namespace internal {

template <typename Result>
class AsyncCallbackContext : public LiveCapturable {
 public:
  typedef ResultErrorPair<Result> Pair;

  AsyncCallbackContext(const SharedUvLoop& shared_loop,
                       const typename Pair::ProduceCallback& thread_work,
                       const typename Pair::ConsumeCallback& on_work)
      : LiveCapturable("pjcore::internal::AsyncCallbackContext",
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
  scoped_ptr<google::protobuf::Message> CaptureLive() const OVERRIDE {
    scoped_ptr<LiveUvLoopAsyncCallbackContext> live(
        new LiveUvLoopAsyncCallbackContext());
    live->set_shared_loop_ptr(reinterpret_cast<uint64_t>(shared_loop_.get()));
    if (!on_work_.is_null()) {
      CaptureLiveCallbackOut(on_work_, live->mutable_on_work());
    }
    live->set_request_ptr(reinterpret_cast<int64_t>(&request_));
    return scoped_ptr<google::protobuf::Message>(live.release());
  }

 private:
  ~AsyncCallbackContext() {
    LogDestroy();
    PJCORE_CHECK(thread_work_.is_null());
    PJCORE_CHECK(on_work_.is_null());
  }

  static void StaticThreadWork(uv_work_t* request) {
    PJCORE_CHECK(request);
    AsyncCallbackContext* context =
        static_cast<AsyncCallbackContext*>(request->data);
    PJCORE_CHECK(context);

    typename Pair::ProduceCallback local_thread_work = context->thread_work_;
    context->thread_work_.Reset();

    context->result_error_pair_.Produce(local_thread_work);
  }

  static void StaticOnWork(uv_work_t* request, int /* status */) {
    PJCORE_CHECK(request);
    AsyncCallbackContext* context =
        static_cast<AsyncCallbackContext*>(request->data);
    PJCORE_CHECK(context);

    typename Pair::ConsumeCallback local_on_work = context->on_work_;
    context->on_work_.Reset();

    context->result_error_pair_.Consume(local_on_work);

    delete context;
  }

  SharedUvLoop shared_loop_;

  typename Pair::ProduceCallback thread_work_;

  typename Pair::ConsumeCallback on_work_;

  uv_work_t request_;

  Pair result_error_pair_;
};

template <typename Result>
void TemplateUvLoopAsyncCallback(
    RefCountedUvLoop* ref_counted_uv_loop,
    const Callback<Result(Error* error)>& thread_work,
    const Callback<void(Result result, const Error& error)>& on_work) {
  PJCORE_CHECK(ref_counted_uv_loop);
  (new internal::AsyncCallbackContext<Result>(ref_counted_uv_loop, thread_work,
                                              on_work))->AsyncProcess();
}

}  // namespace internal

}  // namespace pjcore

#endif  // PJCORE_SHARED_UV_LOOP_INL_H_
