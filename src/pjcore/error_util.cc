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

#include "pjcore/error_util.h"

#include <stdio.h>

#include <string>

#include "pjcore/abstract_uv.h"
#include "pjcore/errno_description.h"
#include "pjcore/http_util.h"
#include "pjcore/json_util.h"
#include "pjcore/json_writer.h"
#include "pjcore/logging.h"
#include "pjcore/uv_util.h"

namespace pjcore {

bool ErrnosResolved(const Error& error) {
  if (error.has_system_errno() && error.system_errno_description().empty()) {
    return false;
  }

  if (error.has_uv_errno() && error.uv_errno_description().empty()) {
    return false;
  }

  if (error.has_http_errno() && error.http_errno_description().empty()) {
    return false;
  }

  if (error.has_http_status_code() && error.http_reason_phrase().empty()) {
    return false;
  }

  if (error.has_cause() && !ErrnosResolved(error.cause())) {
    return false;
  }

  return true;
}

void ResolveErrnos(Error* mutable_error) {
  PJCORE_CHECK(mutable_error);

  if (!mutable_error->has_system_errno()) {
    mutable_error->clear_system_errno_description();
  } else {
    mutable_error->set_system_errno_description(
        ErrnoDescription(mutable_error->system_errno()));
  }

  if (!mutable_error->has_uv_errno()) {
    mutable_error->clear_uv_errno_description();
  } else {
    mutable_error->set_uv_errno_description(
        uv_strerror(mutable_error->uv_errno()));
  }

  if (!mutable_error->has_http_errno()) {
    mutable_error->clear_http_errno_description();
  } else {
    mutable_error->set_http_errno_description(http_errno_description(
        static_cast<http_errno>(mutable_error->http_errno())));
  }

  if (!mutable_error->has_http_status_code()) {
    mutable_error->clear_http_reason_phrase();
  } else {
    mutable_error->set_http_reason_phrase(
        HttpReasonPhrase(mutable_error->http_status_code()));
  }

  if (mutable_error->has_cause()) {
    ResolveErrnos(mutable_error->mutable_cause());
  }
}

std::string ErrorToString(const Error& error,
                          bool strip_quotes_unescape_tabs_and_slashes) {
  JsonWriterConfig config;
  config.set_escape_unicode(false);
  config.set_space(true);

  std::string str;
  if (ErrnosResolved(error)) {
    str = WriteJson(error, config);
  } else {
    Error resolved(error);
    ResolveErrnos(&resolved);
    str = WriteJson(resolved, config);
  }

  if (strip_quotes_unescape_tabs_and_slashes) {
    str = StripQuotesUnescapeTabsAndSlashes(str);
  }

  return str;
}

void PrintError(const Error& error) {
  std::string error_str = ErrorToString(error);
  fprintf(stdout, "%.*s\n", static_cast<int>(error_str.size()),
          error_str.data());
}

}  // namespace pjcore
