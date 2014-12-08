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

#ifndef PJCORE_SHARED_FUTURE_INL_H_
#define PJCORE_SHARED_FUTURE_INL_H_

#include "pjcore/shared_future.h"

#include <algorithm>

#include "pjcore/third_party/chromium/bind.h"
#include "pjcore/third_party/chromium/callback.h"

#include "pjcore/logging.h"
#include "pjcore/result_error_pair.h"

namespace pjcore {

namespace internal {

template <typename Result>
class FutureResultErrorContext
    : public RefCounted<FutureResultErrorContext<Result> > {
 public:
  FutureResultErrorContext(ResultErrorPair<Result>* mutable_store,
                           const Closure& closure)
      : mutable_store_(mutable_store), closure_(closure) {
    PJCORE_CHECK(mutable_store_);
    PJCORE_CHECK(!closure_.is_null());
  }

  void OnCallback(Result result, const Error& error) {
    using std::swap;
    swap(mutable_store_->result_, result);
    mutable_store_->error_ = error;
    closure_.Run();
  }

  ResultErrorPair<Result>* mutable_store_;

  Closure closure_;
};

template <typename Result>
Callback<void(Result result, const Error& error)> TemplateFutureCreateCallback(
    RefCountedFuture* ref_counted_future,
    ResultErrorPair<Result>* mutable_store) {
  scoped_refptr<FutureResultErrorContext<Result> > context =
      new FutureResultErrorContext<Result>(mutable_store,
                                           ref_counted_future->CreateClosure());

  return Bind(&FutureResultErrorContext<Result>::OnCallback, context);
}

}  // namespace internal

}  // namespace pjcore

#endif  // PJCORE_SHARED_FUTURE_INL_H_
