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

// TODO(pjcore): revisit tests

#include "pjcore/auto_callback.h"

#include <gtest/gtest.h>

#include "pjcore/third_party/chromium/bind.h"

#include "pjcore/logging.h"

namespace pjcore {

namespace {

void Increment(int *value) {
  PJCORE_CHECK(value);
  ++*value;
}

Closure CreateIncrementClosure(int *value) { return Bind(&Increment, value); }

}  // unnamed namespace

TEST(AutoClosure, Simple) {
  int value = 0;

  {
    AutoClosure none;
    ASSERT_EQ(0, value);
  }

  ASSERT_EQ(0, value);

  {
    AutoClosure foo(CreateIncrementClosure(&value));
    ASSERT_EQ(0, value);
  }

  ASSERT_EQ(1, value);

  {
    AutoClosure one(CreateIncrementClosure(&value));
    AutoClosure two(one.Pass());
    ASSERT_EQ(1, value);
  }

  ASSERT_EQ(2, value);

  {
    AutoClosure release(CreateIncrementClosure(&value));
    (void)release.Release();
    ASSERT_EQ(2, value);
  }

  ASSERT_EQ(2, value);

  {
    AutoClosure reset(CreateIncrementClosure(&value));
    reset.Reset();
    ASSERT_EQ(3, value);
  }

  ASSERT_EQ(3, value);

  {
    AutoClosure alpha(CreateIncrementClosure(&value));
    ASSERT_EQ(3, value);
    AutoClosure beta;
    ASSERT_EQ(3, value);
    beta = alpha.Pass();
    ASSERT_EQ(3, value);
  }

  ASSERT_EQ(4, value);
}

}  // namespace pjcore
