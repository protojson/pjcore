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

#ifndef PJCORE_SHARED_UV_LOOP_H_
#define PJCORE_SHARED_UV_LOOP_H_

#include "pjcore/shared_future.h"

typedef struct uv_loop_s uv_loop_t;

namespace pjcore {

class AbstractUv;
AbstractUv* GetRealUv();

namespace internal {

template <typename Result>
void TemplateUvLoopAsyncCallback(
    RefCountedUvLoop* ref_counted_uv_loop,
    const Callback<Result(Error* error)>& thread_work,
    const Callback<void(Result result, const Error& error)>& on_work);

}  // namespace internal

class RefCountedUvLoop : public LiveCapturable,
                         public RefCounted<RefCountedUvLoop> {
 public:
  RefCountedUvLoop(LiveCapturableList* live_list, AbstractUv* uv, bool own_loop,
                   uv_loop_t* loop);

  AbstractUv* uv() const { return uv_; }

  uv_loop_t* loop() const { return loop_; }

  void AsyncClosure(const Closure& thread_work, const Closure& on_work);

  template <typename Result>
  void AsyncCallback(
      const Callback<Result(Error* error)>& thread_work,
      const Callback<void(Result result, const Error& error)>& on_work) {
    internal::TemplateUvLoopAsyncCallback(this, thread_work, on_work);
  }

  SharedFuture CreateFuture();

 protected:
  scoped_ptr<google::protobuf::Message> CaptureLive() const OVERRIDE;

 private:
  friend class RefCounted<RefCountedUvLoop>;

  ~RefCountedUvLoop();

  AbstractUv* uv_;

  bool own_loop_;

  uv_loop_t* loop_;
};

SharedUvLoop CreateUvLoop(LiveCapturableList* live_list,
                          AbstractUv* uv = GetRealUv());

SharedUvLoop GetDefaultUvLoop(LiveCapturableList* live_list,
                              AbstractUv* uv = GetRealUv());

}  // namespace pjcore

#endif  // PJCORE_SHARED_UV_LOOP_H_
