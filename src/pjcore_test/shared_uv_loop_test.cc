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

#include "pjcore/shared_uv_loop.h"

#include <gtest/gtest.h>

#include "pjcore/third_party/chromium/bind.h"

#include "pjcore/logging.h"

#include "pjcore_test/is_gmock_verbose.h"

namespace pjcore {

namespace {

void Increment(int32_t *value) {
  PJCORE_CHECK(value);
  ++*value;
}

}  // unnamed namespace

TEST(SharedUvLoop, AsyncClosure) {
  LiveCapturableList live_list(IsGmockVerbose());

  SharedUvLoop shared_loop = CreateUvLoop(&live_list);
  ASSERT_TRUE(shared_loop);

  int32_t value = 0;

  SharedFuture on_work_future = shared_loop->CreateFuture();

  shared_loop->AsyncClosure(Bind(&Increment, &value),
                            on_work_future->CreateClosure());

  on_work_future->Wait();

  ASSERT_EQ(1, value);

  shared_loop->AsyncClosure(Bind(&Increment, &value),
                            on_work_future->CreateClosure());

  shared_loop->AsyncClosure(Bind(&Increment, &value),
                            on_work_future->CreateClosure());

  on_work_future->Wait();

  ASSERT_EQ(3, value);
}

}  // namespace pjcore
