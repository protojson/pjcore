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

#ifndef PJCORE_SHARED_FUTURE_H_
#define PJCORE_SHARED_FUTURE_H_

#include "pjcore/third_party/chromium/callback_forward.h"
#include "pjcore/third_party/chromium/ref_counted.h"

#include "pjcore/live_capturable.h"

namespace pjcore {

class Error;

template <typename Result>
struct ResultErrorPair;

class RefCountedUvLoop;
typedef scoped_refptr<RefCountedUvLoop> SharedUvLoop;

class RefCountedFuture;
typedef scoped_refptr<RefCountedFuture> SharedFuture;

namespace internal {

template <typename Result>
Callback<void(Result result, const Error& error)> TemplateFutureCreateCallback(
    RefCountedFuture* ref_counted_future,
    ResultErrorPair<Result>* mutable_store);

}  // namespace internal

class RefCountedFuture : public LiveCapturable,
                         public RefCounted<RefCountedFuture> {
 public:
  explicit RefCountedFuture(const SharedUvLoop& shared_loop);

  void Wait();

  Closure CreateClosure();

  template <typename Result>
  Callback<void(Result result, const Error& error)> CreateCallback(
      ResultErrorPair<Result>* mutable_store) {
    return internal::TemplateFutureCreateCallback(this, mutable_store);
  }

 protected:
  scoped_ptr<google::protobuf::Message> CaptureLive() const OVERRIDE;

 private:
  friend class RefCounted<RefCountedFuture>;
  friend class RefCountedUvLoop;

  ~RefCountedFuture();

  void OnClosure();

  SharedUvLoop shared_loop_;

  int32_t pending_count_;
};

}  // namespace pjcore

#endif  // PJCORE_SHARED_FUTURE_H_
