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

#ifndef PJCORE_TEST_MOCK_HTTP_SERVER_CORE_H_
#define PJCORE_TEST_MOCK_HTTP_SERVER_CORE_H_

#include <gmock/gmock.h>

#include "pjcore/abstract_http_server_core.h"

namespace pjcore {

class MockHttpServerCore : public AbstractHttpServerCore {
 public:
  explicit MockHttpServerCore(LiveCapturableList* live_list);

  ~MockHttpServerCore();

  MOCK_METHOD0(Die, void());

  MOCK_METHOD6(InitSync,
               bool(const HttpServerConfig& config,
                    const SharedUvLoop& shared_loop,
                    const SharedHttpHandler& handler,
                    AbstractHttpServerConnectionFactory* connection_factory,
                    AbstractHttpParserFactory* parser_factory, Error* error));

  MOCK_METHOD1(InitAsync, bool(Error* error));

  MOCK_METHOD2(OnShellDestroyed,
               void(void* shell, const Closure& optional_on_destroy));

  void Destroy();

 protected:
  scoped_ptr<google::protobuf::Message> CaptureLive() const OVERRIDE;
};

}  // namespace pjcore

#endif  // PJCORE_TEST_MOCK_HTTP_SERVER_CORE_H_
