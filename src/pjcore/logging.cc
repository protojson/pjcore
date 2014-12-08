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

#include "pjcore/logging.h"

#include <stdlib.h>
#include <stdio.h>

#include <string>

#include "pjcore/third_party/chromium/bind.h"

#include "pjcore/error_util.h"

namespace pjcore {

namespace {

void LogToFile(FILE* file, const char* filename, int line,
               StringPiece description, const Error* error) {
  if (!error) {
    fprintf(file, ".%s:%d: %.*s\n", filename, line,
            static_cast<int>(description.size()), description.data());
  } else if (error == &Error::default_instance()) {
    fprintf(file, "!%s:%d: %.*s\n", filename, line,
            static_cast<int>(description.size()), description.data());
  } else {
    std::string error_str = ErrorToString(*error);
    fprintf(file, "!%s:%d: %.*s, %.*s\n", filename, line,
            static_cast<int>(description.size()), description.data(),
            static_cast<int>(error_str.size()), error_str.data());
  }
}

LogCallback g_std_out_log_callback(Bind(&LogToFile, stdout));
LogCallback g_std_err_log_callback(Bind(&LogToFile, stderr));

#ifndef NDEBUG
LogCallback g_global_log_callback(g_std_out_log_callback);
#else
LogCallback g_global_log_callback;
#endif

LogCallback g_global_fatality_callback;

}  // unnamed namespace

const LogCallback& GetStdOutLogCallback() { return g_std_err_log_callback; }
const LogCallback& GetStdErrLogCallback() { return g_std_err_log_callback; }

const LogCallback& GetGlobalLogCallback() { return g_global_log_callback; }

void SetGlobalLogCallback(const LogCallback& log_callback) {
  g_global_log_callback = log_callback;
}

const LogCallback& GetGlobalFatalityCallback() {
  return g_global_fatality_callback;
}

void SetGlobalFatalityCallback(const LogCallback& fatality_callback) {
  g_global_fatality_callback = fatality_callback;
}

GlobalLogOverride::GlobalLogOverride(const LogCallback& new_log_callback) {
  old_log_callback_ = GetGlobalLogCallback();
  SetGlobalLogCallback(new_log_callback);
}

GlobalLogOverride::~GlobalLogOverride() {
  SetGlobalLogCallback(old_log_callback_);
}

void Fatality(const char* filename, int line, StringPiece description) {
  fprintf(stderr, "#%s:%d: %.*s\n", filename, line,
          static_cast<int>(description.size()), description.data());
  fflush(stderr);

  const pjcore::LogCallback& log_callback = GetGlobalLogCallback();
  if (!log_callback.is_null()) {
    log_callback.Run(filename, line, description, &Error::default_instance());
  }

  const pjcore::LogCallback& fatality_callback = GetGlobalFatalityCallback();
  if (!fatality_callback.is_null()) {
    fatality_callback.Run(filename, line, description,
                          &Error::default_instance());
  }

  abort();
}

}  // namespace pjcore
