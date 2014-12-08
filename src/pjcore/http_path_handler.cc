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

#include "pjcore/http_path_handler.h"

#include <map>
#include <string>

#include "pjcore/logging.h"
#include "pjcore/live_http.pb.h"

namespace pjcore {

HttpPathHandler::HttpPathHandler(LiveCapturableList* live_list, const Map& map,
                                 const SharedHttpHandler& default_handler)
    : AbstractHttpHandler("pjcore::HttpPathHandler", live_list),
      map_(map),
      default_handler_(default_handler) {}

void HttpPathHandler::AsyncHandle(scoped_ptr<HttpRequest> request,
                                  const HttpResponseCallback& on_response) {
  PJCORE_CHECK(request);
  PJCORE_CHECK(!on_response.is_null());

  Map::const_iterator it = map_.find(request->parsed_url().path());

  if (it != map_.end()) {
    if (it->second) {
      it->second->AsyncHandle(request.Pass(), on_response);
      return;
    }
  } else {
    if (default_handler_) {
      default_handler_->AsyncHandle(request.Pass(), on_response);
      return;
    }
  }

  Error error;
  error.set_description(std::string("No handler for path ") +
                        request->parsed_url().path());

  on_response.Run(scoped_ptr<HttpResponse>(), error);
}

HttpPathHandler::~HttpPathHandler() { LogDestroy(); }

scoped_ptr<google::protobuf::Message> HttpPathHandler::CaptureLive() const {
  scoped_ptr<LiveHttpPathHandler> live(new LiveHttpPathHandler());
  for (Map::const_iterator it = map_.begin(); it != map_.end(); ++it) {
    LiveHttpPathHandler::PathHandlerEntry* entry = live->add_map();
    if (!it->first.empty()) {
      entry->set_path(it->first);
    }
    if (it->second) {
      entry->set_handler_ptr(reinterpret_cast<int64_t>(it->second.get()));
    }
  }
  if (default_handler_) {
    live->set_default_handler_ptr(
        reinterpret_cast<int64_t>(default_handler_.get()));
  }
  return scoped_ptr<google::protobuf::Message>(live.release());
}

}  // namespace pjcore
