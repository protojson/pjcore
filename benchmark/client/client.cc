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

#include <uv.h>

#include <string>

#include "pjcore/third_party/chromium/bind.h"

#include "pjcore/error_util.h"
#include "pjcore/http_client.h"
#include "pjcore/logging.h"

using namespace pjcore;  // NOLINT(build/namespaces)

class MainContext {
 public:
  MainContext()
      : live_list_(false),
        remaining_requests_(0),
        success_count_(0),
        failure_count_(0) {}

  bool Run(int argc, const char* argv[], Error* error) {
    PJCORE_CHECK(error);
    error->Clear();

    int arg_index = 1;
    PJCORE_REQUIRE(arg_index < argc, "No URL");

    url_ = argv[arg_index];
    ++arg_index;

    remaining_requests_ = 1;
    int32_t remaining_init = 1;

    if (arg_index < argc) {
      remaining_requests_ = atoi(argv[arg_index++]);
      if (remaining_requests_ < 1) {
        remaining_requests_ = 1;
      }

      if (arg_index < argc) {
        remaining_init = atoi(argv[arg_index++]);
        if (remaining_init < 1) {
          remaining_init = 1;
        }
      }
    }

    single_request_ = (remaining_requests_ == 1);

    shared_loop_ = CreateUvLoop(&live_list_);

    HttpClientConfig config;

    client_ = HttpClient::Create(
        &live_list_, config, shared_loop_,
        Bind(&MainContext::OnClientDestroy, Unretained(this)), error);

    PJCORE_REQUIRE_SILENT(client_, "Failed to initialize client");

    while (remaining_init && remaining_requests_) {
      --remaining_init;
      --remaining_requests_;

      scoped_ptr<HttpRequest> request(new HttpRequest());
      request->set_url(url_);

      client_->AsyncHandle(request.Pass(),
                           Bind(&MainContext::OnResponse, Unretained(this)));
    }

    if (!remaining_requests_) {
      client_ = NULL;
    }

    PJCORE_CHECK_EQ(0, uv_run(shared_loop_->loop(), UV_RUN_DEFAULT));

    if (single_request_) {
      if (single_response_) {
        fwrite(single_response_->content().data(), sizeof(char),
               single_response_->content().size(), stdout);
        return true;
      } else {
        *error = single_error_;
        return false;
      }
    } else {
      fprintf(stdout, "Success count: %d, failure count: %d\n", success_count_,
              failure_count_);
      return true;
    }
  }

  bool Run(Error* error);

 private:
  void OnResponse(scoped_ptr<HttpResponse> response, const Error& error) {
    if (response) {
      ++success_count_;
    } else {
      ++failure_count_;
    }

    if (single_request_) {
      single_response_ = response.Pass();
      single_error_ = error;
    }

    if (remaining_requests_) {
      --remaining_requests_;

      scoped_ptr<HttpRequest> request(new HttpRequest());
      request->set_url(url_);

      client_->AsyncHandle(request.Pass(),
                           Bind(&MainContext::OnResponse, Unretained(this)));

      if (!remaining_requests_) {
        client_ = NULL;
      }
    }
  }

  void OnClientDestroy() {}

  LiveCapturableList live_list_;

  SharedUvLoop shared_loop_;

  std::string url_;

  int32_t remaining_requests_;

  bool single_request_;

  SharedHttpHandler client_;

  int32_t success_count_;

  int32_t failure_count_;

  scoped_ptr<HttpResponse> single_response_;

  Error single_error_;
};

int main(int argc, const char* argv[]) {
  MainContext context;

  Error error;
  if (!context.Run(argc, argv, &error)) {
    fprintf(stderr, "Failed to run: %s\n", ErrorToString(error).c_str());
    return 1;
  }

  return 0;
}
