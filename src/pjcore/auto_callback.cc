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

#include "pjcore/auto_callback.h"

#include <algorithm>

#include "pjcore/logging.h"

namespace pjcore {

AutoClosure::AutoClosure() : closure_() {}

AutoClosure::AutoClosure(const Closure& closure) : closure_(closure) {}

AutoClosure::AutoClosure(RValue rvalue) : closure_(rvalue.object->closure_) {
  rvalue.object->closure_.Reset();
}

AutoClosure::~AutoClosure() {
  if (!closure_.is_null()) {
    Closure local_closure;
    using std::swap;
    swap(local_closure, closure_);
    local_closure.Run();
  }
}

AutoClosure& AutoClosure::operator=(RValue rvalue) {
  AutoClosure temp(rvalue);
  Swap(&temp);
  return *this;
}

void AutoClosure::Reset(const Closure& closure) {
  AutoClosure temp(closure);
  Swap(&temp);
}

const Closure& AutoClosure::Get() const { return closure_; }

AutoClosure::operator AutoClosure::Testable() const {
  return closure_.is_null() ? NULL : &AutoClosure::closure_;
}

bool AutoClosure::operator==(const Closure& closure) const {
  return &closure_ == &closure;
}

bool AutoClosure::operator!=(const Closure& closure) const {
  return &closure_ != &closure;
}

void AutoClosure::Swap(AutoClosure* another) {
  PJCORE_CHECK(another);
  using std::swap;
  swap(closure_, another->closure_);
}

Closure AutoClosure::Release() {
  Closure temp = closure_;
  closure_.Reset();
  return temp;
}

}  // namespace pjcore
