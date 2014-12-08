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

#include "pjcore/abstract_counter.h"

#include <gmock/gmock.h>

#include <algorithm>

#include "pjcore/third_party/chromium/scoped_ptr.h"

#include "pjcore_test/mock_counter.h"

namespace pjcore {

using ::testing::Exactly;
using ::testing::InSequence;
using ::testing::Mock;

TEST(AbstractCounterUnit, CreateDestroy) {
  MockCounter counter;

  {
    InSequence s;

    EXPECT_CALL(counter, Increment()).Times(Exactly(1));
    EXPECT_CALL(counter, Decrement()).Times(Exactly(1));
  }

  { AbstractCounter::Unit unit(&counter); }
}

TEST(AbstractCounterUnit, MoveCreate) {
  MockCounter counter;

  EXPECT_CALL(counter, Increment()).Times(Exactly(1));
  EXPECT_CALL(counter, Decrement()).Times(Exactly(0));

  scoped_ptr<AbstractCounter::Unit> alpha(new AbstractCounter::Unit(&counter));

  ASSERT_TRUE(Mock::VerifyAndClearExpectations(&counter));

  EXPECT_CALL(counter, Increment()).Times(Exactly(0));
  EXPECT_CALL(counter, Decrement()).Times(Exactly(0));

  scoped_ptr<AbstractCounter::Unit> beta(
      new AbstractCounter::Unit(alpha->Pass()));

  ASSERT_TRUE(Mock::VerifyAndClearExpectations(&counter));

  EXPECT_CALL(counter, Increment()).Times(Exactly(0));
  EXPECT_CALL(counter, Decrement()).Times(Exactly(0));

  alpha.reset();

  ASSERT_TRUE(Mock::VerifyAndClearExpectations(&counter));

  EXPECT_CALL(counter, Increment()).Times(Exactly(0));
  EXPECT_CALL(counter, Decrement()).Times(Exactly(1));

  beta.reset();
}

TEST(AbstractCounterUnit, Swap) {
  MockCounter counter;

  EXPECT_CALL(counter, Increment()).Times(Exactly(1));
  EXPECT_CALL(counter, Decrement()).Times(Exactly(0));

  scoped_ptr<AbstractCounter::Unit> alpha(new AbstractCounter::Unit(&counter));

  ASSERT_TRUE(Mock::VerifyAndClearExpectations(&counter));

  EXPECT_CALL(counter, Increment()).Times(Exactly(0));
  EXPECT_CALL(counter, Decrement()).Times(Exactly(0));

  scoped_ptr<AbstractCounter::Unit> beta(new AbstractCounter::Unit());

  {
    using std::swap;
    swap(*beta, *alpha);
  }

  ASSERT_TRUE(Mock::VerifyAndClearExpectations(&counter));

  EXPECT_CALL(counter, Increment()).Times(Exactly(0));
  EXPECT_CALL(counter, Decrement()).Times(Exactly(0));

  alpha.reset();

  ASSERT_TRUE(Mock::VerifyAndClearExpectations(&counter));

  EXPECT_CALL(counter, Increment()).Times(Exactly(0));
  EXPECT_CALL(counter, Decrement()).Times(Exactly(1));

  beta.reset();
}

TEST(AbstractCounterUnit, SwapSelf) {
  MockCounter counter;

  EXPECT_CALL(counter, Increment()).Times(Exactly(1));
  EXPECT_CALL(counter, Decrement()).Times(Exactly(0));

  scoped_ptr<AbstractCounter::Unit> alpha(new AbstractCounter::Unit(&counter));

  ASSERT_TRUE(Mock::VerifyAndClearExpectations(&counter));

  EXPECT_CALL(counter, Increment()).Times(Exactly(0));
  EXPECT_CALL(counter, Decrement()).Times(Exactly(0));

  {
    using std::swap;
    swap(*alpha, *alpha);
  }

  ASSERT_TRUE(Mock::VerifyAndClearExpectations(&counter));

  EXPECT_CALL(counter, Increment()).Times(Exactly(0));
  EXPECT_CALL(counter, Decrement()).Times(Exactly(1));

  alpha.reset();
}

}  // namespace pjcore
