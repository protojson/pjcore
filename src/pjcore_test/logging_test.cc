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

#include "pjcore/logging.h"

#include <gmock/gmock.h>

#include "pjcore/third_party/chromium/bind.h"

namespace pjcore {

using ::testing::Exactly;
using ::testing::NotNull;
using ::testing::_;

namespace {

class MockLog {
 public:
  MockLog() : callback_(Bind(&MockLog::Log, Unretained(this))) {}

  MOCK_CONST_METHOD4(Log, void(const char* filename, int line,
                               StringPiece description, const Error* error));

  LogCallback GetCallback() { return callback_; }

 private:
  LogCallback callback_;
};

}  // unnamed namespace

TEST(Logging, GlobalLogOverride) {
  MockLog mock_log;

  LogCallback old_log_callback = GetGlobalLogCallback();
  EXPECT_FALSE(mock_log.GetCallback().Equals(old_log_callback));

  {
    GlobalLogOverride global_log_override(mock_log.GetCallback());

    EXPECT_TRUE(GetGlobalLogCallback().Equals(mock_log.GetCallback()));
    int alpha_line = __LINE__ + 3;
    EXPECT_CALL(mock_log, Log(__FILE__, alpha_line, StringPiece("alpha"), NULL))
        .Times(Exactly(1));
    PJCORE_LOG("alpha");

    int beta_line = __LINE__ + 3;
    EXPECT_CALL(mock_log, Log(__FILE__, beta_line, StringPiece("beta"),
                              &Error::default_instance())).Times(Exactly(1));
    PJCORE_LOG_EMPTY_ERROR("beta");

    Error error;
    int gamma_line = __LINE__ + 3;
    EXPECT_CALL(mock_log, Log(__FILE__, gamma_line, StringPiece("gamma"),
                              &error)).Times(Exactly(1));
    PJCORE_LOG_ERROR("gamma", &error);
  }

  EXPECT_TRUE(GetGlobalLogCallback().Equals(old_log_callback));
}

TEST(Logging, Fatality) {
  GlobalLogOverride global_log_override;
  ASSERT_DEATH(Fatality("alpha", 123, "beta"), ".*#alpha:123: beta");
}

TEST(Logging, CheckSuccess) {
  MockLog mock_log;
  GlobalLogOverride global_log_override(mock_log.GetCallback());

  EXPECT_CALL(mock_log, Log(_, _, _, _)).Times(Exactly(0));
  PJCORE_CHECK(true);
}

TEST(Logging, CheckFailure) {
  GlobalLogOverride global_log_overide;
  ASSERT_DEATH(PJCORE_CHECK(false), "#.*logging_test\\.cc:.*: false");
}

}  // namespace pjcore
