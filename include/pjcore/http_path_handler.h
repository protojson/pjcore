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

#ifndef PJCORE_HTTP_PATH_HANDLER_H_
#define PJCORE_HTTP_PATH_HANDLER_H_

#include <map>
#include <string>

#include "pjcore/abstract_http_handler.h"

namespace pjcore {

class HttpPathHandler : public AbstractHttpHandler {
 public:
  typedef std::map<std::string, SharedHttpHandler> Map;

  HttpPathHandler(
      LiveCapturableList* live_list, const Map& map = Map(),
      const SharedHttpHandler& default_handler = SharedHttpHandler());

  const Map& map() const { return map_; }

  Map* mutable_map() { return &map_; }

  const SharedHttpHandler& default_handler() const { return default_handler_; }

  void set_default_handler(const SharedHttpHandler& default_handler) {
    default_handler_ = default_handler;
  }

  void AsyncHandle(scoped_ptr<HttpRequest> request,
                   const HttpResponseCallback& on_response) OVERRIDE;

 protected:
  ~HttpPathHandler();

  scoped_ptr<google::protobuf::Message> CaptureLive() const OVERRIDE;

 private:
  friend class RefCounted<HttpPathHandler>;

  Map map_;

  SharedHttpHandler default_handler_;
};

typedef scoped_refptr<HttpPathHandler> SharedHttpPathHandler;

}  // namespace pjcore

#endif  // PJCORE_HTTP_PATH_HANDLER_H_
