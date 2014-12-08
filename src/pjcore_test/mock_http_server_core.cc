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

#include "pjcore_test/mock_http_server_core.h"

namespace pjcore {

MockHttpServerCore::MockHttpServerCore(LiveCapturableList* live_list)
    : AbstractHttpServerCore("pjcore::MockHttpServerCore", live_list) {}

MockHttpServerCore::~MockHttpServerCore() { Die(); }

scoped_ptr<google::protobuf::Message> MockHttpServerCore::CaptureLive() const {
  return scoped_ptr<google::protobuf::Message>();
}

}  // namespace pjcore
