#include <uv.h>

#include <iostream>

#include "pjcore/third_party/chromium/bind.h"

#include "pjcore/http_client.h"
#include "pjcore/error_util.h"

bool g_client_destroyed = false;

void OnClientDestroy() {
  g_client_destroyed = true;
}

pjcore::scoped_ptr<pjcore::HttpResponse> g_response;
pjcore::Error g_error;

void OnResponse(pjcore::scoped_ptr<pjcore::HttpResponse> response,
                const pjcore::Error& error) {
  g_response = response.Pass();
  g_error = error;
}

int main(int, const char**) {
  pjcore::Error error;
  pjcore::SharedHttpHandler client = pjcore::HttpClient::Create(
      pjcore::HttpClientConfig(), pjcore::Bind(&OnClientDestroy), &error);

  if (!client) {
    std::cerr << "Error: " << pjcore::ErrorToString(g_error) << std::endl;
    return 1;
  } else {
    pjcore::scoped_ptr<pjcore::HttpRequest> request(new pjcore::HttpRequest());
    request->set_url("http://status.aws.amazon.com/rss/ec2-us-west-1.rss");

    client->AsyncHandle(request.Pass(), pjcore::Bind(&OnResponse));
    client = NULL;

    uv_run(uv_default_loop(), UV_RUN_DEFAULT);

    if (!g_response) {
      std::cerr << "Error: " << pjcore::ErrorToString(g_error) << std::endl;
      return 1;
    } else {
      std::cout << g_response->content() << std::endl;
      return 0;
    }
  }
}
