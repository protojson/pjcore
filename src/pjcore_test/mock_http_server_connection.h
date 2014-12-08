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

#ifndef PJCORE_TEST_MOCK_HTTP_SERVER_CONNECTION_H_
#define PJCORE_TEST_MOCK_HTTP_SERVER_CONNECTION_H_

#include <gmock/gmock.h>

#include "pjcore/abstract_http_server_connection.h"

namespace pjcore {

class MockHttpServerConnection : public AbstractHttpServerConnection {
 public:
  explicit MockHttpServerConnection(LiveCapturableList* live_list);

  ~MockHttpServerConnection();

  MOCK_METHOD0(Die, void());

  MOCK_METHOD2(InitSync,
               bool(AbstractHttpParserFactory* parser_factory, Error* error));

  MOCK_METHOD2(Close, void(bool result, const Error& error));

  void InitAsync(
      const Callback<void(bool result, const Error& error)>& on_closing,
      const Closure& on_close) OVERRIDE;

  MOCK_METHOD0(InitAsyncProxy, void());

  testing::AssertionResult InvokeOnClosing(bool result, const Error& error);

  testing::AssertionResult InvokeOnClose();

 protected:
  scoped_ptr<google::protobuf::Message> CaptureLive() const OVERRIDE;

 private:
  Callback<void(bool result, const Error& error)> on_closing_;

  Closure on_close_;
};

}  // namespace pjcore

#endif  // PJCORE_TEST_MOCK_HTTP_SERVER_CONNECTION_H_
