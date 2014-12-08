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

#include <stdlib.h>
#include <uv.h>

#include "pjcore/third_party/chromium/bind.h"

#include "pjcore/error_util.h"
#include "pjcore/http_path_handler.h"
#include "pjcore/http_server.h"
#include "pjcore/json.h"

using namespace pjcore;  // NOLINT(build/namespaces)

class HelloWorldHttpHandler : public AbstractHttpHandler {
 public:
  HelloWorldHttpHandler()
      : AbstractHttpHandler("HelloWorldHttpHandler", NULL) {}

  void AsyncHandle(scoped_ptr<HttpRequest> request,
                   const HttpResponseCallback& on_response) OVERRIDE {
    scoped_ptr<HttpResponse> response(new HttpResponse());
    response->set_content(WriteJson(MakeJsonObject("hello", "world")));
    HttpHeader* server_header = response->add_headers();
    server_header->set_name("Server");
    server_header->set_value("pjcore");
    HttpHeader* content_type_header = response->add_headers();
    content_type_header->set_name("Content-Type");
    content_type_header->set_value("application/json; charset=UTF-8");
    on_response.Run(response.Pass(), Error());
  }

 protected:
  scoped_ptr<google::protobuf::Message> CaptureLive() const OVERRIDE {
    return scoped_ptr<google::protobuf::Message>();
  }
};

struct ClosureTracker : public RefCounted<ClosureTracker> {
  ClosureTracker() : was_invoked(false) {}

  void Invoke() { was_invoked = true; }

  bool was_invoked;
};

int main(int argc, const char* argv[]) {
  HttpServerConfig config;

  int arg_index = 1;
  if (arg_index < argc) {
    int32_t port = atoi(argv[arg_index]);
    if (port) {
      config.set_port(port);
    }
    ++arg_index;

    if (arg_index < argc) {
      int32_t backlog = atoi(argv[arg_index]);
      if (backlog) {
        config.set_backlog(backlog);
      }
    }
  }

  SharedHttpPathHandler handler(new HttpPathHandler(NULL));

  (*handler->mutable_map())["/json"] = new HelloWorldHttpHandler();

  Error error;
  SharedHttpServer server = HttpServer::Create(config, handler, &error);

  if (!server) {
    PrintError(error);
    return 1;
  }

  scoped_refptr<ClosureTracker> on_destroy_tracker(new ClosureTracker());

  bool result = server->InitAsync(
      Bind(&ClosureTracker::Invoke, on_destroy_tracker.get()), &error);

  if (!result) {
    PrintError(error);
    server = NULL;
  }

  while (!on_destroy_tracker->was_invoked) {
    uv_run(uv_default_loop(), UV_RUN_DEFAULT);
  }

  return result ? 0 : 1;
}
