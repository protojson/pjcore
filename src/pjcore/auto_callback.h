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

#ifndef PJCORE_AUTO_CALLBACK_H_
#define PJCORE_AUTO_CALLBACK_H_

#include <algorithm>

#include "pjcore/third_party/chromium/callback.h"
#include "pjcore/third_party/chromium/move.h"

#include "pjcore/logging.h"
#include "pjcore/result_error_pair.h"

namespace pjcore {

class AutoClosure {
  MOVE_ONLY_TYPE_FOR_CPP_03(AutoClosure, RValue)

 public:
  AutoClosure();

  explicit AutoClosure(const Closure& closure);

  AutoClosure(RValue rvalue);  // NOLINT(runtime/explicit)

  ~AutoClosure();

  AutoClosure& operator=(RValue rvalue);

  void Reset(const Closure& closure = Closure());

  const Closure& Get() const;

 private:
  typedef Closure AutoClosure::*Testable;

 public:
  operator Testable() const;

  bool operator==(const Closure& closure) const;

  bool operator!=(const Closure& closure) const;

  void Swap(AutoClosure* another);

  Closure Release() WARN_UNUSED_RESULT;

 private:
  Closure closure_;
};

template <typename Result>
class AutoResultErrorCallback {
  MOVE_ONLY_TYPE_FOR_CPP_03(AutoResultErrorCallback, RValue)

 public:
  typedef ResultErrorPair<Result> Pair;

  AutoResultErrorCallback() : callback_(), result_error_pair_() {}

  explicit AutoResultErrorCallback(
      const typename Pair::ConsumeCallback& callback)
      : callback_(callback) {}

  AutoResultErrorCallback(RValue rvalue) {  // NOLINT(runtime/explicit)
    using std::swap;
    swap(callback_, rvalue.object->callback_);
    swap(result_error_pair_, rvalue.object->result_error_pair_);
  }

  ~AutoResultErrorCallback() {
    if (!callback_.is_null()) {
      typename Pair::ConsumeCallback local_callback = callback_;
      callback_.Reset();
      result_error_pair_.Consume(local_callback);
    }
  }

  AutoResultErrorCallback& operator=(RValue rvalue) {
    AutoResultErrorCallback tmp(rvalue);
    Swap(&tmp);
    return *this;
  }

  void Reset(const typename Pair::ConsumeCallback& callback =
                 typename Pair::ConsumeCallback()) {
    AutoResultErrorCallback temp(callback);
    Swap(&temp);
  }

  const typename Pair::ConsumeCallback& Get() const { return callback_; }

  const Pair& result_error_pair() const { return result_error_pair_; }

  Pair* mutable_result_error_pair() { return &result_error_pair_; }

  const Result& result() const { return result_error_pair_.result_; }

  Result* mutable_result() { return &result_error_pair_.result_; }

  const Error& error() const { return result_error_pair_.error_; }

  Error* mutable_error() { return &result_error_pair_.error_; }

 private:
  typedef typename Pair::ConsumeCallback AutoResultErrorCallback::*Testable;

 public:
  operator Testable() const {
    return callback_.is_null() ? NULL : &AutoResultErrorCallback::callback_;
  }

  void Swap(AutoResultErrorCallback* another) {
    PJCORE_CHECK(another);
    using std::swap;
    swap(callback_, another->callback_);
    swap(result_error_pair_, another->result_error_pair_);
  }

  typename Pair::ConsumeCallback Release() {
    typename Pair::ConsumeCallback temp = callback_;
    callback_.Reset();
    result_error_pair_.Reset();
    return temp;
  }

 private:
  typename Pair::ConsumeCallback callback_;

  Pair result_error_pair_;
};

typedef AutoResultErrorCallback<bool> AutoBoolErrorCallback;

}  // namespace pjcore

#endif  // PJCORE_AUTO_CALLBACK_H_
