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

#include "pjcore_test/mock_http_server_connection.h"

#include "pjcore/json_properties.h"
#include "pjcore/live_util.h"
#include "pjcore/logging.h"
#include "pjcore/third_party/chromium/callback_helpers.h"

namespace pjcore {

using ::testing::AssertionFailure;
using ::testing::AssertionResult;
using ::testing::AssertionSuccess;

MockHttpServerConnection::MockHttpServerConnection(
    LiveCapturableList* live_list)
    : AbstractHttpServerConnection("pjcore::MockHttpServerConnection",
                                   live_list) {}

MockHttpServerConnection::~MockHttpServerConnection() { Die(); }

void MockHttpServerConnection::InitAsync(
    const Callback<void(bool result, const Error& error)>& on_closing,
    const Closure& on_close) {
  PJCORE_CHECK(!on_closing.is_null());
  PJCORE_CHECK(!on_close.is_null());

  PJCORE_CHECK(on_closing_.is_null());
  PJCORE_CHECK(on_close_.is_null());

  on_closing_ = on_closing;
  on_close_ = on_close;

  InitAsyncProxy();
}

AssertionResult MockHttpServerConnection::InvokeOnClosing(bool result,
                                                          const Error& error) {
  if (on_closing_.is_null()) {
    return AssertionFailure() << "On closing callback not set";
  }
  ResetAndReturn(&on_closing_).Run(result, error);
  return AssertionSuccess();
}

AssertionResult MockHttpServerConnection::InvokeOnClose() {
  if (on_close_.is_null()) {
    return AssertionFailure() << "On close callback not set";
  }
  ResetAndReturn(&on_close_).Run();
  return AssertionSuccess();
}

scoped_ptr<google::protobuf::Message> MockHttpServerConnection::CaptureLive()
    const {
  scoped_ptr<JsonValue> live(new JsonValue());
  live->set_type(JsonValue::TYPE_OBJECT);

  if (!on_closing_.is_null()) {
    LiveCallback live_on_closing;
    CaptureLiveCallbackOut(on_closing_, &live_on_closing);
    SetJsonProperty(live.get(), "on_closing", live_on_closing);
  }

  if (!on_close_.is_null()) {
    LiveCallback live_on_close;
    CaptureLiveCallbackOut(on_close_, &live_on_close);
    SetJsonProperty(live.get(), "on_close", live_on_close);
  }

  return scoped_ptr<google::protobuf::Message>(live.release());
}

}  // namespace pjcore
