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

#include <algorithm>

#include "pjcore/abstract_counter.h"
#include "pjcore/logging.h"

namespace pjcore {

AbstractCounter::Unit::Unit(AbstractCounter* counter) : counter_(counter) {
  if (counter_) {
    counter_->Increment();
  }
}

AbstractCounter::Unit::Unit(RValue rvalue) : counter_(rvalue.object->counter_) {
  rvalue.object->counter_ = NULL;
}

AbstractCounter::Unit::~Unit() {
  if (counter_) {
    counter_->Decrement();
  }
}

void AbstractCounter::Unit::Swap(Unit& another) {
  std::swap(counter_, another.counter_);
}

void swap(AbstractCounter::Unit& primary, AbstractCounter::Unit& secondary) {
  primary.Swap(secondary);
}

CounterSplitter::CounterSplitter(AbstractCounter* primary,
                                 AbstractCounter* secondary)
    : primary_(primary), secondary_(secondary) {}

void CounterSplitter::Increment() {
  primary_->Increment();
  secondary_->Increment();
}

void CounterSplitter::Decrement() {
  secondary_->Decrement();
  primary_->Decrement();
}

NonNegativeCounter::NonNegativeCounter(const Closure& on_zero)
    : count_(0), on_zero_(on_zero) {}

void NonNegativeCounter::Increment() { ++count_; }

void NonNegativeCounter::Decrement() {
  PJCORE_CHECK_GT(count_, 0);
  if (!--count_ && !on_zero_.is_null()) {
    on_zero_.Run();
  }
}

}  // namespace pjcore
