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

#include "pjcore/live_capturable.h"

#include <gmock/gmock.h>

#include "pjcore/json_util.h"

namespace pjcore {

using ::testing::Exactly;

namespace {

class MockLiveCapturable : public LiveCapturable {
 public:
  MockLiveCapturable(const char *live_class, LiveCapturableList *list)
      : LiveCapturable(live_class, list) {}

  MOCK_CONST_METHOD0(CaptureLiveProxy, google::protobuf::Message *());

 protected:
  scoped_ptr<google::protobuf::Message> CaptureLive() const OVERRIDE {
    return scoped_ptr<google::protobuf::Message>(CaptureLiveProxy());
  }
};

}  // unnamed namespace

TEST(LiveCapturable, CaptureLive) {
  LiveCapturableList list(false);
  EXPECT_FALSE(list.log_create_destroy());
  EXPECT_EQ(0, list.next_id());
  EXPECT_EQ(0, list.size());

  EXPECT_TRUE(AreJsonValuesEqual(list.CaptureAllLiveJson(), MakeJsonArray()));

  scoped_ptr<MockLiveCapturable> alpha(new MockLiveCapturable("Alpha", &list));
  EXPECT_EQ("Alpha", alpha->live_class());
  EXPECT_EQ(&list, alpha->live_list());
  EXPECT_EQ(0, alpha->live_id());

  EXPECT_EQ(1, list.next_id());
  EXPECT_EQ(1, list.size());

  EXPECT_CALL(*alpha, CaptureLiveProxy()).Times(Exactly(1));

  EXPECT_TRUE(
      AreJsonValuesEqual(list.CaptureAllLiveJson(),
                         MakeJsonArray(MakeJsonObject(
                             "live_id", 0, "live_class", "Alpha", "live_ptr",
                             LiveCapturableList::HexifyPtr(alpha.get())))));

  scoped_ptr<MockLiveCapturable> beta(new MockLiveCapturable("Beta", &list));
  EXPECT_EQ("Beta", beta->live_class());
  EXPECT_EQ(&list, beta->live_list());
  EXPECT_EQ(1, beta->live_id());

  EXPECT_EQ(2, list.next_id());
  EXPECT_EQ(2, list.size());

  EXPECT_CALL(*alpha, CaptureLiveProxy()).Times(Exactly(1));
  EXPECT_CALL(*beta, CaptureLiveProxy()).Times(Exactly(1));

  EXPECT_TRUE(AreJsonValuesEqual(
      list.CaptureAllLiveJson(),
      MakeJsonArray(
          MakeJsonObject("live_id", 0, "live_class", "Alpha", "live_ptr",
                         LiveCapturableList::HexifyPtr(alpha.get())),
          MakeJsonObject("live_id", 1, "live_class", "Beta", "live_ptr",
                         LiveCapturableList::HexifyPtr(beta.get())))));

  alpha.reset();

  EXPECT_EQ(2, list.next_id());
  EXPECT_EQ(1, list.size());

  scoped_ptr<MockLiveCapturable> gamma(new MockLiveCapturable("Gamma", &list));
  EXPECT_EQ("Gamma", gamma->live_class());
  EXPECT_EQ(&list, gamma->live_list());
  EXPECT_EQ(2, gamma->live_id());

  EXPECT_EQ(3, list.next_id());
  EXPECT_EQ(2, list.size());

  EXPECT_CALL(*beta, CaptureLiveProxy()).Times(Exactly(1));
  EXPECT_CALL(*gamma, CaptureLiveProxy()).Times(Exactly(1));

  EXPECT_TRUE(AreJsonValuesEqual(
      list.CaptureAllLiveJson(),
      MakeJsonArray(
          MakeJsonObject("live_id", 1, "live_class", "Beta", "live_ptr",
                         LiveCapturableList::HexifyPtr(beta.get())),
          MakeJsonObject("live_id", 2, "live_class", "Gamma", "live_ptr",
                         LiveCapturableList::HexifyPtr(gamma.get())))));

  beta.reset();

  EXPECT_EQ(3, list.next_id());
  EXPECT_EQ(1, list.size());

  EXPECT_CALL(*gamma, CaptureLiveProxy()).Times(Exactly(1));

  EXPECT_TRUE(
      AreJsonValuesEqual(list.CaptureAllLiveJson(),
                         MakeJsonArray(MakeJsonObject(
                             "live_id", 2, "live_class", "Gamma", "live_ptr",
                             LiveCapturableList::HexifyPtr(gamma.get())))));

  gamma.reset();

  EXPECT_EQ(3, list.next_id());
  EXPECT_EQ(0, list.size());

  EXPECT_TRUE(AreJsonValuesEqual(list.CaptureAllLiveJson(), MakeJsonArray()));
}

}  // namespace pjcore
