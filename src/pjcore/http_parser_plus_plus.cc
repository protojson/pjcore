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

#include "pjcore/http_parser_plus_plus.h"

#include <http_parser.h>

#include "pjcore/logging.h"
#include "pjcore/http_util.h"
#include "pjcore/json_util.h"
#include "pjcore/name_value_util.h"
#include "pjcore/repeated_field_util.h"
#include "pjcore/url_util.h"

namespace pjcore {

HttpParserPlusPlus::HttpParserPlusPlus(LiveCapturableList* live_list,
                                       HttpParserType type)
    : AbstractHttpParser("pjcore::HttpParserPlusPlus", live_list),
      type_(type),
      error(NULL) {
  PJCORE_CHECK(type_ == HTTP_PARSER_REQUEST || type_ == HTTP_PARSER_RESPONSE);

  memset(&parser_settings_, 0, sizeof(parser_settings_));
  parser_settings_.on_message_begin = &StaticOnMessageBegin;
  parser_settings_.on_url = &StaticOnUrl;
  parser_settings_.on_status = &StaticOnStatus;
  parser_settings_.on_header_field = &StaticOnHeaderField;
  parser_settings_.on_header_value = &StaticOnHeaderValue;
  parser_settings_.on_headers_complete = &StaticOnHeadersComplete;
  parser_settings_.on_body = &StaticOnBody;
  parser_settings_.on_message_complete = &StaticOnMessageComplete;

  memset(&parser_, 0, sizeof(parser_));
  parser_.data = this;

  http_parser_init(
      &parser_, (type_ == HTTP_PARSER_REQUEST) ? HTTP_REQUEST : HTTP_RESPONSE);
}

HttpParserPlusPlus::~HttpParserPlusPlus() { LogDestroy(); }

bool HttpParserPlusPlus::Read(StringPiece data, size_t* offset, Error* error) {
  PJCORE_CHECK(offset);
  *offset = 0;
  PJCORE_CHECK(error);
  error->Clear();

  this->error = error;

  *offset = http_parser_execute(&parser_, &parser_settings_, data.data(),
                                data.size());

  PJCORE_CHECK_LE(*offset, data.size());

  this->error = NULL;

  PJCORE_HTTP_REQUIRE(&parser_, "Failed to parse");

  return true;
}

scoped_ptr<HttpRequest> HttpParserPlusPlus::NextRequest() {
  PJCORE_CHECK_EQ(HTTP_PARSER_REQUEST, type_);
  scoped_ptr<HttpRequest> request;
  if (!request_queue_.empty()) {
    request.reset(request_queue_.front());
    request_queue_.pop_front();
  }
  return request.Pass();
}

scoped_ptr<HttpResponse> HttpParserPlusPlus::NextResponse() {
  PJCORE_CHECK_EQ(HTTP_PARSER_RESPONSE, type_);
  scoped_ptr<HttpResponse> response;
  if (!response_queue_.empty()) {
    response.reset(response_queue_.front());
    response_queue_.pop_front();
  }
  return response.Pass();
}

scoped_ptr<google::protobuf::Message> HttpParserPlusPlus::CaptureLive() const {
  scoped_ptr<LiveHttpParserPlusPlus> live(new LiveHttpParserPlusPlus());
  live->set_type(type_);
  if (pending_request_) {
    CaptureLiveHttpRequestOut(pending_request_.get(),
                              live->mutable_pending_request());
  }
  if (pending_response_) {
    CaptureLiveHttpResponseOut(pending_response_.get(),
                               live->mutable_pending_response());
  }
  CaptureLiveHttpParserOut(&parser_, live->mutable_parser());
  if (error) {
    live->set_error_ptr(reinterpret_cast<int64_t>(error));
  }
  for (std::list<HttpRequest*>::const_iterator request_it =
           request_queue_.begin();
       request_it != request_queue_.end(); ++request_it) {
    CaptureLiveHttpRequestOut(*request_it, live->add_request_queue());
  }
  for (std::list<HttpResponse*>::const_iterator response_it =
           response_queue_.begin();
       response_it != response_queue_.end(); ++response_it) {
    CaptureLiveHttpResponseOut(*response_it, live->add_response_queue());
  }
  return scoped_ptr<google::protobuf::Message>(live.Pass());
}

int HttpParserPlusPlus::StaticOnMessageBegin(http_parser* parser) {
  PJCORE_CHECK(parser);
  HttpParserPlusPlus* context = static_cast<HttpParserPlusPlus*>(parser->data);
  PJCORE_CHECK(context);
  PJCORE_NEGATIVE_REQUIRE_SILENT(context->OnMessageBegin(),
                                 "OnMessageBegin callback has failed");
  return 0;
}

bool HttpParserPlusPlus::OnMessageBegin() {
  if (type_ == HTTP_PARSER_REQUEST) {
    pending_request_.reset(new HttpRequest());
  } else {
    pending_response_.reset(new HttpResponse());
  }
  return true;
}

int HttpParserPlusPlus::StaticOnUrl(http_parser* parser, const char* at,
                                    size_t length) {
  PJCORE_CHECK(parser);
  HttpParserPlusPlus* context = static_cast<HttpParserPlusPlus*>(parser->data);
  PJCORE_CHECK(context);
  PJCORE_NEGATIVE_REQUIRE_SILENT(context->OnUrl(StringPiece(at, length)),
                                 "OnUrl callback has failed");
  return 0;
}

bool HttpParserPlusPlus::OnUrl(StringPiece data) {
  PJCORE_REQUIRE(pending_request_, "Unexpected URL for HTTP_PARSER_RESPONSE");
  data.AppendToString(pending_request_->mutable_url());
  return true;
}

int HttpParserPlusPlus::StaticOnStatus(http_parser* parser, const char* at,
                                       size_t length) {
  PJCORE_CHECK(parser);
  HttpParserPlusPlus* context = static_cast<HttpParserPlusPlus*>(parser->data);
  PJCORE_CHECK(context);
  PJCORE_NEGATIVE_REQUIRE_SILENT(context->OnStatus(StringPiece(at, length)),
                                 "OnStatus callback has failed");
  return 0;
}

bool HttpParserPlusPlus::OnStatus(StringPiece /* data */) {
  PJCORE_REQUIRE(pending_response_,
                 "Unexpected status for HTTP_PARSER_REQUEST");
  return true;
}

int HttpParserPlusPlus::StaticOnHeaderField(http_parser* parser, const char* at,
                                            size_t length) {
  PJCORE_CHECK(parser);
  HttpParserPlusPlus* context = static_cast<HttpParserPlusPlus*>(parser->data);
  PJCORE_CHECK(context);
  PJCORE_NEGATIVE_REQUIRE_SILENT(
      context->OnHeaderField(StringPiece(at, length)),
      "OnHeaderField callback has failed");
  return 0;
}

bool HttpParserPlusPlus::OnHeaderField(StringPiece data) {
  google::protobuf::RepeatedPtrField<HttpHeader>* mutable_headers =
      pending_request_ ? pending_request_->mutable_headers()
                       : pending_response_->mutable_headers();

  if (Empty(*mutable_headers) || Back(*mutable_headers).has_value()) {
    PushBack(mutable_headers);
  }

  data.AppendToString(MutableBack(mutable_headers)->mutable_name());

  return true;
}

int HttpParserPlusPlus::StaticOnHeaderValue(http_parser* parser, const char* at,
                                            size_t length) {
  PJCORE_CHECK(parser);
  HttpParserPlusPlus* context = static_cast<HttpParserPlusPlus*>(parser->data);
  PJCORE_CHECK(context);
  PJCORE_NEGATIVE_REQUIRE_SILENT(
      context->OnHeaderValue(StringPiece(at, length)),
      "OnHeaderValue callback has failed");
  return 0;
}

bool HttpParserPlusPlus::OnHeaderValue(StringPiece data) {
  google::protobuf::RepeatedPtrField<HttpHeader>* mutable_headers =
      pending_request_ ? pending_request_->mutable_headers()
                       : pending_response_->mutable_headers();

  PJCORE_REQUIRE(!Empty(*mutable_headers), "Unexpected header value");

  data.AppendToString(MutableBack(mutable_headers)->mutable_value());

  return true;
}

int HttpParserPlusPlus::StaticOnHeadersComplete(http_parser* parser) {
  PJCORE_CHECK(parser);
  HttpParserPlusPlus* context = static_cast<HttpParserPlusPlus*>(parser->data);
  PJCORE_CHECK(context);
  PJCORE_NEGATIVE_REQUIRE_SILENT(context->OnHeadersComplete(),
                                 "OnHeadersComplete callback has failed");
  return 0;
}

bool HttpParserPlusPlus::OnHeadersComplete() {
  if (pending_response_) {
    pending_response_->set_should_keep_alive(http_should_keep_alive(&parser_));
  } else {
    pending_request_->set_should_keep_alive(http_should_keep_alive(&parser_));
    if (pending_request_->has_url()) {
      PJCORE_REQUIRE_SILENT(
          ParseUrl(pending_request_->url(), parser_.method == HTTP_CONNECT,
                   pending_request_->mutable_parsed_url(), error),
          "Failed to parse URL");
    }
  }
  return true;
}

int HttpParserPlusPlus::StaticOnBody(http_parser* parser, const char* at,
                                     size_t length) {
  PJCORE_CHECK(parser);
  HttpParserPlusPlus* context = static_cast<HttpParserPlusPlus*>(parser->data);
  PJCORE_CHECK(context);
  PJCORE_NEGATIVE_REQUIRE_SILENT(context->OnBody(StringPiece(at, length)),
                                 "OnBody callback has failed");
  return 0;
}

bool HttpParserPlusPlus::OnBody(StringPiece data) {
  std::string* mutable_content = pending_request_
                                     ? pending_request_->mutable_content()
                                     : pending_response_->mutable_content();
  data.AppendToString(mutable_content);
  return true;
}

int HttpParserPlusPlus::StaticOnMessageComplete(http_parser* parser) {
  PJCORE_CHECK(parser);
  HttpParserPlusPlus* context = static_cast<HttpParserPlusPlus*>(parser->data);
  PJCORE_CHECK(context);
  PJCORE_NEGATIVE_REQUIRE_SILENT(context->OnMessageComplete(),
                                 "OnMessageComplete callback has failed");
  return 0;
}

bool HttpParserPlusPlus::OnMessageComplete() {
  if (pending_request_) {
    PJCORE_CHECK(!pending_response_);
    PJCORE_CHECK(pending_request_->has_should_keep_alive());
    pending_request_->set_method(parser_.method);
    pending_request_->mutable_http_version()->set_major(parser_.http_major);
    pending_request_->mutable_http_version()->set_minor(parser_.http_minor);
    request_queue_.push_back(pending_request_.release());
  } else {
    PJCORE_CHECK(pending_response_);
    PJCORE_CHECK(pending_response_->has_should_keep_alive());
    pending_response_->mutable_http_version()->set_major(parser_.http_major);
    pending_response_->mutable_http_version()->set_minor(parser_.http_minor);
    // TODO(pjcore): consider error reporting by adding
    // optional Error error to HttpRequest, HttpResponse
    if (HttpStatusCode_IsValid(parser_.status_code)) {
      pending_response_->set_status_code(
          static_cast<HttpStatusCode>(parser_.status_code));
    }
    response_queue_.push_back(pending_response_.release());
  }

  return true;
}

}  // namespace pjcore
