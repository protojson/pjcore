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

#ifndef PJCORE_ABSTRACT_COUNTER_H_
#define PJCORE_ABSTRACT_COUNTER_H_

#include "pjcore/third_party/chromium/callback.h"

namespace pjcore {

class AbstractCounter {
 public:
  virtual void Increment() = 0;

  virtual void Decrement() = 0;

  class Unit {
    MOVE_ONLY_TYPE_FOR_CPP_03(Unit, RValue)

   public:
    explicit Unit(AbstractCounter* counter = NULL);

    Unit(RValue rvalue);  // NOLINT(runtime/explicit)

    ~Unit();

    AbstractCounter* counter() const { return counter_; }

    void Swap(Unit& another);  // NOLINT(build/include_what_you_use)

   private:
    AbstractCounter* counter_;
  };
};

void swap(  // NOLINT(build/include_what_you_use)
    AbstractCounter::Unit& primary, AbstractCounter::Unit& secondary);

class CounterSplitter : public AbstractCounter {
 public:
  CounterSplitter(AbstractCounter* primary, AbstractCounter* secondary);

  void Increment() OVERRIDE;

  void Decrement() OVERRIDE;

 private:
  AbstractCounter* primary_;
  AbstractCounter* secondary_;
};

class NonNegativeCounter : public AbstractCounter {
 public:
  explicit NonNegativeCounter(const Closure& on_zero = Closure());

  uint64_t count() const { return count_; }

  void Increment() OVERRIDE;

  void Decrement() OVERRIDE;

 private:
  uint64_t count_;
  Closure on_zero_;
};

}  // namespace pjcore

#endif  // PJCORE_ABSTRACT_COUNTER_H_
