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

#include "pjcore_test/mock_http_client_transaction.h"

#include "pjcore/json_properties.h"
#include "pjcore/live_util.h"
#include "pjcore/logging.h"
#include "pjcore/third_party/chromium/callback_helpers.h"

namespace pjcore {

using ::testing::AssertionFailure;
using ::testing::AssertionResult;
using ::testing::AssertionSuccess;

MockHttpClientTransaction::MockHttpClientTransaction(
    LiveCapturableList* live_list)
    : AbstractHttpClientTransaction("pjcore::MockHttpClientTransaction",
                                    live_list) {}

MockHttpClientTransaction::~MockHttpClientTransaction() { Die(); }

void MockHttpClientTransaction::ResultFailure(
    scoped_ptr<AbstractHttpClientTransaction> self, const Error& error) {
  PJCORE_CHECK(self);
  PJCORE_CHECK_EQ(this, self.get());

  ResultFailureProxy(error);
}

void MockHttpClientTransaction::ResultSuccess(
    scoped_ptr<AbstractHttpClientTransaction> self,
    scoped_ptr<HttpResponse> response) {
  PJCORE_CHECK(self);
  PJCORE_CHECK_EQ(this, self.get());

  PJCORE_CHECK(response);

  ResultSuccessProxy(response.get());
}

scoped_ptr<google::protobuf::Message> MockHttpClientTransaction::CaptureLive()
    const {
  return scoped_ptr<google::protobuf::Message>();
}

}  // namespace pjcore
