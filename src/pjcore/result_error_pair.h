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

#ifndef PJCORE_RESULT_ERROR_PAIR_H_
#define PJCORE_RESULT_ERROR_PAIR_H_

#include <algorithm>

#include "pjcore/third_party/chromium/callback.h"

#include "pjcore/error.pb.h"

namespace pjcore {

template <typename Result>
struct ResultErrorPair {
  typedef Callback<Result(Error* error)> ProduceCallback;

  typedef Callback<void(Result result, const Error& error)> ConsumeCallback;
  ResultErrorPair() : result_(), error_() {}

  void Swap(ResultErrorPair* another) {
    using std::swap;
    swap(result_, another->result_);
    swap(error_, another->error_);
  }

  void Reset() {
    ResultErrorPair empty;
    Swap(&empty);
  }

  void Produce(const ProduceCallback& produce_callback) {
    result_ = produce_callback.Run(&error_);
  }

  void Consume(const ConsumeCallback& consume_callback) {
    consume_callback.Run(internal::CallbackForward(result_),
                         internal::CallbackForward(error_));
  }

  Result result_;

  Error error_;
};

typedef ResultErrorPair<bool> BoolErrorPair;

template <typename Result>
void swap(ResultErrorPair<Result>& left, ResultErrorPair<Result>& right) {
  left.Swap(&right);
}

}  // namespace pjcore

#endif  // PJCORE_RESULT_ERROR_PAIR_H_
