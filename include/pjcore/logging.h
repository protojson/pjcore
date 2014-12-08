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

#ifndef PJCORE_LOGGING_H_
#define PJCORE_LOGGING_H_

#include "pjcore/third_party/chromium/callback.h"
#include "pjcore/third_party/chromium/string_piece.h"

#include "pjcore/error.pb.h"

namespace pjcore {

int CurrentErrno();

typedef Callback<void(const char* filename, int line, StringPiece description,
                      const Error* error)> LogCallback;

const LogCallback& GetStdOutLogCallback();
const LogCallback& GetStdErrLogCallback();

const LogCallback& GetGlobalLogCallback();
void SetGlobalLogCallback(const LogCallback& log_callback);

const LogCallback& GetGlobalLogCallback();
void SetGlobalFatalityCallback(const LogCallback& fatality_callback);

class GlobalLogOverride {
 public:
  explicit GlobalLogOverride(
      const LogCallback& new_log_callback = LogCallback());

  ~GlobalLogOverride();

 private:
  LogCallback old_log_callback_;

  DISALLOW_EVIL_CONSTRUCTORS(GlobalLogOverride);
};

void Fatality(const char* filename, int line, StringPiece description);

#define PJCORE_FATALITY(_DESCRIPTION) \
  pjcore::Fatality(__FILE__, __LINE__, _DESCRIPTION)

#define PJCORE_CHECK(_CONDITION)                         \
  do {                                                   \
    if (_CONDITION) {                                    \
    } else {                                             \
      pjcore::Fatality(__FILE__, __LINE__, #_CONDITION); \
    }                                                    \
  } while (false)

#define PJCORE_LOG_ERROR(_DESCRIPTION, _ERROR)                         \
  do {                                                                 \
    const pjcore::LogCallback& _log_callback = GetGlobalLogCallback(); \
    if (!_log_callback.is_null()) {                                    \
      _log_callback.Run(__FILE__, __LINE__, (_DESCRIPTION), (_ERROR)); \
    }                                                                  \
  } while (false);

#define PJCORE_LOG(_DESCRIPTION) PJCORE_LOG_ERROR(_DESCRIPTION, NULL)

#define PJCORE_LOG_EMPTY_ERROR(_DESCRIPTION) \
  PJCORE_LOG_ERROR(_DESCRIPTION, &pjcore::Error::default_instance())

#define PJCORE_CHECK_EQ(_LEFT, _RIGHT) PJCORE_CHECK((_LEFT) == (_RIGHT))
#define PJCORE_CHECK_NE(_LEFT, _RIGHT) PJCORE_CHECK((_LEFT) != (_RIGHT))
#define PJCORE_CHECK_LE(_LEFT, _RIGHT) PJCORE_CHECK((_LEFT) <= (_RIGHT))
#define PJCORE_CHECK_LT(_LEFT, _RIGHT) PJCORE_CHECK((_LEFT) < (_RIGHT))
#define PJCORE_CHECK_GE(_LEFT, _RIGHT) PJCORE_CHECK((_LEFT) >= (_RIGHT))
#define PJCORE_CHECK_GT(_LEFT, _RIGHT) PJCORE_CHECK((_LEFT) > (_RIGHT))

#define PJCORE_CUSTOM_FAIL(_DESCRIPTION, _RETURN) \
  do {                                            \
    error->set_description(_DESCRIPTION);         \
    error->set_source_file(__FILE__);             \
    error->set_source_line(__LINE__);             \
    PJCORE_LOG_ERROR((_DESCRIPTION), error);      \
    return _RETURN;                               \
  } while (false)

#define PJCORE_FAIL(_DESCRIPTION) PJCORE_CUSTOM_FAIL(_DESCRIPTION, false)

#define PJCORE_NULL_FAIL(_DESCRIPTION) PJCORE_CUSTOM_FAIL(_DESCRIPTION, NULL)

#define PJCORE_SCOPED_FAIL(_TYPE, _DESCRIPTION) \
  PJCORE_CUSTOM_FAIL(_DESCRIPTION, scoped_ptr<_TYPE>())

#define PJCORE_VOID_FAIL(_DESCRIPTION) PJCORE_CUSTOM_FAIL(_DESCRIPTION, )

#define PJCORE_CUSTOM_REQUIRE(_CONDITION, _DESCRIPTION, _RETURN) \
  do {                                                           \
    if (_CONDITION) {                                            \
    } else {                                                     \
      PJCORE_CUSTOM_FAIL(_DESCRIPTION, _RETURN);                 \
    }                                                            \
  } while (false)

#define PJCORE_REQUIRE(_CONDITION, _DESCRIPTION) \
  PJCORE_CUSTOM_REQUIRE(_CONDITION, _DESCRIPTION, false)

#define PJCORE_NULL_REQUIRE(_CONDITION, _DESCRIPTION) \
  PJCORE_CUSTOM_REQUIRE(_CONDITION, _DESCRIPTION, NULL)

#define PJCORE_SCOPED_REQUIRE(_TYPE, _CONDITION, _DESCRIPTION) \
  PJCORE_CUSTOM_REQUIRE(_CONDITION, _DESCRIPTION, scoped_ptr<_TYPE>())

#define PJCORE_VOID_REQUIRE(_CONDITION, _DESCRIPTION) \
  PJCORE_CUSTOM_REQUIRE(_CONDITION, _DESCRIPTION, )

#define PJCORE_ERRNO_CUSTOM_REQUIRE(_CONDITION, _DESCRIPTION, _RETURN) \
  do {                                                                 \
    if (_CONDITION) {                                                  \
    } else {                                                           \
      error->set_system_errno(pjcore::CurrentErrno());                 \
      PJCORE_CUSTOM_FAIL(_DESCRIPTION, _RETURN);                       \
    }                                                                  \
  } while (false)

#define PJCORE_ERRNO_REQUIRE(_CONDITION, _DESCRIPTION) \
  PJCORE_ERRNO_CUSTOM_REQUIRE(_CONDITION, _DESCRIPTION, false)

#define PJCORE_ERRNO_NULL_REQUIRE(_CONDITION, _DESCRIPTION) \
  PJCORE_ERRNO_CUSTOM_REQUIRE(_CONDITION, _DESCRIPTION, NULL)

#define PJCORE_ERRNO_SCOPED_REQUIRE(_TYPE, _CONDITION, _DESCRIPTION) \
  PJCORE_ERRNO_CUSTOM_REQUIRE(_CONDITION, _DESCRIPTION, scoped_ptr<_TYPE>())

#define PJCORE_ERRNO_VOID_REQUIRE(_CONDITION, _DESCRIPTION) \
  PJCORE_ERRNO_CUSTOM_REQUIRE(_CONDITION, _DESCRIPTION, )

#define PJCORE_UV_CUSTOM_REQUIRE(_UV_EXPRESSION, _DESCRIPTION, _RETURN) \
  do {                                                                  \
    long _uv_errno = (_UV_EXPRESSION); /* NOLINT(runtime/int) */        \
    if (_uv_errno < 0) {                                                \
      error->set_uv_errno(static_cast<int>(_uv_errno));                 \
      PJCORE_CUSTOM_FAIL(_DESCRIPTION, _RETURN);                        \
    }                                                                   \
  } while (false);

#define PJCORE_UV_REQUIRE(_CONDITION, _DESCRIPTION) \
  PJCORE_UV_CUSTOM_REQUIRE(_CONDITION, _DESCRIPTION, false)

#define PJCORE_UV_NULL_REQUIRE(_CONDITION, _DESCRIPTION) \
  PJCORE_UV_CUSTOM_REQUIRE(_CONDITION, _DESCRIPTION, NULL)

#define PJCORE_UV_SCOPED_REQUIRE(_TYPE, _CONDITION, _DESCRIPTION) \
  PJCORE_UV_CUSTOM_REQUIRE(_CONDITION, _DESCRIPTION, scoped_ptr<_TYPE>())

#define PJCORE_UV_VOID_REQUIRE(_CONDITION, _DESCRIPTION) \
  PJCORE_UV_CUSTOM_REQUIRE(_CONDITION, _DESCRIPTION, )

#define PJCORE_CUSTOM_FAIL_SILENT(_DESCRIPTION, _RETURN) \
  do {                                                   \
    PJCORE_LOG_EMPTY_ERROR(_DESCRIPTION);                \
    return _RETURN;                                      \
  } while (false)

#define PJCORE_FAIL_SILENT(_DESCRIPTION) \
  PJCORE_CUSTOM_FAIL_SILENT(_DESCRIPTION, false)

#define PJCORE_NULL_FAIL_SILENT(_DESCRIPTION) \
  PJCORE_CUSTOM_FAIL_SILENT(_DESCRIPTION, NULL)

#define PJCORE_SCOPED_FAIL_SILENT(_TYPE, _DESCRIPTION) \
  PJCORE_CUSTOM_FAIL_SILENT(_DESCRIPTION, scoped_ptr<_TYPE>())

#define PJCORE_VOID_FAIL_SILENT(_DESCRIPTION) \
  PJCORE_CUSTOM_FAIL_SILENT(_DESCRIPTION, )

#define PJCORE_NEGATIVE_FAIL_SILENT(_DESCRIPTION) \
  PJCORE_CUSTOM_FAIL_SILENT(_DESCRIPTION, -1)

#define PJCORE_CUSTOM_REQUIRE_SILENT(_CONDITION, _DESCRIPTION, _RETURN) \
  do {                                                                  \
    if (_CONDITION) {                                                   \
    } else {                                                            \
      PJCORE_CUSTOM_FAIL_SILENT(_DESCRIPTION, _RETURN);                 \
    }                                                                   \
  } while (false)

#define PJCORE_REQUIRE_SILENT(_CONDITION, _DESCRIPTION) \
  PJCORE_CUSTOM_REQUIRE_SILENT(_CONDITION, _DESCRIPTION, false)

#define PJCORE_NULL_REQUIRE_SILENT(_CONDITION, _DESCRIPTION) \
  PJCORE_CUSTOM_REQUIRE_SILENT(_CONDITION, _DESCRIPTION, NULL)

#define PJCORE_SCOPED_REQUIRE_SILENT(_TYPE, _CONDITION, _DESCRIPTION) \
  PJCORE_CUSTOM_REQUIRE_SILENT(_CONDITION, _DESCRIPTION, scoped_ptr<_TYPE>())

#define PJCORE_VOID_REQUIRE_SILENT(_CONDITION, _DESCRIPTION) \
  PJCORE_CUSTOM_REQUIRE_SILENT(_CONDITION, _DESCRIPTION, )

#define PJCORE_NEGATIVE_REQUIRE_SILENT(_CONDITION, _DESCRIPTION) \
  PJCORE_CUSTOM_REQUIRE_SILENT(_CONDITION, _DESCRIPTION, -1)

#define PJCORE_CUSTOM_FAIL_CAUSE(_DESCRIPTION, _RETURN) \
  do {                                                  \
    pjcore::Error _cause_error;                         \
    _cause_error.Swap(error);                           \
    error->mutable_cause()->Swap(&_cause_error);        \
    PJCORE_CUSTOM_FAIL(_DESCRIPTION, _RETURN);          \
  } while (false)

#define PJCORE_FAIL_CAUSE(_DESCRIPTION) \
  PJCORE_CUSTOM_FAIL_CAUSE(_DESCRIPTION, false)

#define PJCORE_NULL_FAIL_CAUSE(_DESCRIPTION) \
  PJCORE_CUSTOM_FAIL_CAUSE(_DESCRIPTION, NULL)

#define PJCORE_SCOPED_FAIL_CAUSE(_TYPE, _DESCRIPTION) \
  PJCORE_CUSTOM_FAIL_CAUSE(_DESCRIPTION, scoped_ptr<_TYPE>())

#define PJCORE_VOID_FAIL_CAUSE(_DESCRIPTION) \
  PJCORE_CUSTOM_FAIL_CAUSE(_DESCRIPTION, )

#define PJCORE_NEGATIVE_FAIL_CAUSE(_DESCRIPTION) \
  PJCORE_CUSTOM_FAIL_CAUSE(_DESCRIPTION, -1)

#define PJCORE_CUSTOM_REQUIRE_CAUSE(_CONDITION, _DESCRIPTION, _RETURN) \
  do {                                                                 \
    if (_CONDITION) {                                                  \
    } else {                                                           \
      PJCORE_CUSTOM_FAIL_CAUSE(_DESCRIPTION, _RETURN);                 \
    }                                                                  \
  } while (false)

#define PJCORE_REQUIRE_CAUSE(_CONDITION, _DESCRIPTION) \
  PJCORE_CUSTOM_REQUIRE_CAUSE(_CONDITION, _DESCRIPTION, false)

#define PJCORE_NULL_REQUIRE_CAUSE(_CONDITION, _DESCRIPTION) \
  PJCORE_CUSTOM_REQUIRE_CAUSE(_CONDITION, _DESCRIPTION, NULL)

#define PJCORE_SCOPED_REQUIRE_CAUSE(_TYPE, _CONDITION, _DESCRIPTION) \
  PJCORE_CUSTOM_REQUIRE_CAUSE(_CONDITION, _DESCRIPTION, scoped_ptr<_TYPE>())

#define PJCORE_VOID_REQUIRE_CAUSE(_CONDITION, _DESCRIPTION) \
  PJCORE_CUSTOM_REQUIRE_CAUSE(_CONDITION, _DESCRIPTION, )

#define PJCORE_NEGATIVE_REQUIRE_CAUSE(_CONDITION, _DESCRIPTION) \
  PJCORE_CUSTOM_REQUIRE_CAUSE(_CONDITION, _DESCRIPTION, -1)

#define PJCORE_CUSTOM_FAIL_STRING(_DESCRIPTION, _RETURN) \
  do {                                                   \
    *error = _DESCRIPTION;                               \
    PJCORE_LOG_EMPTY_ERROR(*error);                      \
    return _RETURN;                                      \
  } while (false)

#define PJCORE_FAIL_STRING(_DESCRIPTION) \
  PJCORE_CUSTOM_FAIL_STRING(_DESCRIPTION, false)

#define PJCORE_CUSTOM_REQUIRE_STRING(_CONDITION, _DESCRIPTION, _RETURN) \
  do {                                                                  \
    if (_CONDITION) {                                                   \
    } else {                                                            \
      PJCORE_CUSTOM_FAIL_STRING(_DESCRIPTION, _RETURN);                 \
    }                                                                   \
  } while (false)

#define PJCORE_REQUIRE_STRING(_CONDITION, _DESCRIPTION) \
  PJCORE_CUSTOM_REQUIRE_STRING(_CONDITION, _DESCRIPTION, false)

#define PJCORE_NULL_REQUIRE_STRING(_CONDITION, _DESCRIPTION) \
  PJCORE_CUSTOM_REQUIRE_STRING(_CONDITION, _DESCRIPTION, NULL)

#define PJCORE_SCOPED_REQUIRE_STRING(_TYPE, _CONDITION, _DESCRIPTION) \
  PJCORE_CUSTOM_REQUIRE_STRING(_CONDITION, _DESCRIPTION, scoped_ptr<_TYPE>())

#define PJCORE_VOID_REQUIRE_STRING(_CONDITION, _DESCRIPTION) \
  PJCORE_CUSTOM_REQUIRE_STRING(_CONDITION, _DESCRIPTION, )

#define PJCORE_HTTP_CUSTOM_REQUIRE(_HTTP_PARSER, _DESCRIPTION, _RETURN) \
  do {                                                                  \
    http_errno _http_errno = HTTP_PARSER_ERRNO(_HTTP_PARSER);           \
    if (_http_errno != HPE_OK) {                                        \
      error->set_http_errno(_http_errno);                               \
      PJCORE_CUSTOM_FAIL(_DESCRIPTION, _RETURN);                        \
    }                                                                   \
  } while (false);

#define PJCORE_HTTP_REQUIRE(_CONDITION, _DESCRIPTION) \
  PJCORE_HTTP_CUSTOM_REQUIRE(_CONDITION, _DESCRIPTION, false)

#define PJCORE_HTTP_NULL_REQUIRE(_CONDITION, _DESCRIPTION) \
  PJCORE_HTTP_CUSTOM_REQUIRE(_CONDITION, _DESCRIPTION, NULL)

#define PJCORE_HTTP_SCOPED_REQUIRE(_TYPE, _CONDITION, _DESCRIPTION) \
  PJCORE_HTTP_CUSTOM_REQUIRE(_CONDITION, _DESCRIPTION, scoped_ptr<_TYPE>())

#define PJCORE_HTTP_VOID_REQUIRE(_CONDITION, _DESCRIPTION) \
  PJCORE_HTTP_CUSTOM_REQUIRE(_CONDITION, _DESCRIPTION, )

}  // namespace pjcore

#endif  // PJCORE_LOGGING_H_
