#include <uv.h>

#include <iostream>

#include "pjcore/third_party/chromium/bind.h"

#include "pjcore/error_util.h"
#include "pjcore/http_server.h"

class HelloWorldHttpHandler : public pjcore::AbstractHttpHandler {
 public:
  HelloWorldHttpHandler()
      : AbstractHttpHandler("HelloWorldHttpHandler", NULL) {}

  void AsyncHandle(pjcore::scoped_ptr<pjcore::HttpRequest> request,
                   const pjcore::HttpResponseCallback& on_response) {
    pjcore::scoped_ptr<pjcore::HttpResponse> response(
        new pjcore::HttpResponse());
    response->set_content("Hello, world!\n");
    on_response.Run(response.Pass(), pjcore::Error());
  }

 protected:
  pjcore::scoped_ptr<google::protobuf::Message> CaptureLive() const {
    return pjcore::scoped_ptr<google::protobuf::Message>();
  }
};

bool g_server_destroyed = false;

void OnServerDestroy() {
  g_server_destroyed = true;
}

int main(int, const char**) {
  pjcore::HttpServerConfig config;
  config.set_port(8080);

  pjcore::SharedHttpHandler handler(new HelloWorldHttpHandler());

  pjcore::Error error;
  pjcore::SharedHttpServer server = pjcore::HttpServer::Create(
      config, handler, &error);

  if (!server) {
    std::cerr << "Error: " << pjcore::ErrorToString(error) << std::endl;
    return 1;
  } else if (!server->InitAsync(pjcore::Bind(&OnServerDestroy), &error)) {
    std::cerr << "Error: " << pjcore::ErrorToString(error) << std::endl;
    server = NULL;
    while (!g_server_destroyed) {
      uv_run(uv_default_loop(), UV_RUN_DEFAULT);
    }
    return 1;
  } else {
    for (;;) {
      uv_run(uv_default_loop(), UV_RUN_DEFAULT);
    }
  }
}

